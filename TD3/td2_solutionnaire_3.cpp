

#pragma region "Includes"//{
#define _CRT_SECURE_NO_WARNINGS // On permet d'utiliser les fonctions de copies de chaînes qui sont considérées non sécuritaires.

#include "structures_solutionnaire_td2_3.hpp"      // Structures de données pour la collection de films en mémoire.

#include "bibliotheque_cours.hpp"
#include "verification_allocation.hpp" // Nos fonctions pour le rapport de fuites de mémoire.

#include <iostream>
#include <fstream>
#include <string>
#include <limits>
#include <algorithm>
#include "cppitertools/itertools.hpp"
#include "gsl/span"
#include "debogage_memoire.hpp"        // Ajout des numéros de ligne des "new" dans le rapport de fuites.  Doit être après les include du système, qui peuvent utiliser des "placement new" (non supporté par notre ajout de numéros de lignes).
#include <memory>
#include <sstream>
using namespace std;
using namespace iter;
using namespace gsl;

#pragma endregion//}

typedef uint8_t UInt8;
typedef uint16_t UInt16;

#pragma region "Fonctions de base pour lire le fichier binaire"//{

UInt8 lireUint8(istream& fichier)
{
	UInt8 valeur = 0;
	fichier.read((char*)&valeur, sizeof(valeur));
	return valeur;
}
UInt16 lireUint16(istream& fichier)
{
	UInt16 valeur = 0;
	fichier.read((char*)&valeur, sizeof(valeur));
	return valeur;
}
string lireString(istream& fichier)
{
	string texte;
	texte.resize(lireUint16(fichier));
	fichier.read((char*)&texte[0], streamsize(sizeof(texte[0])) * texte.length());
	return texte;
}

#pragma endregion//}




ostream& operator<< (ostream& o, const Acteur& acteur) {

	o << "  " << acteur.nom << ", " << acteur.anneeNaissance << " " << acteur.sexe << "\n";
	return o;
}

ostream& operator<< (ostream& o, const Film& film) {
	o << "Titre: " << film.titre << "\n"
		<< "  Réalisateur: " << film.realisateur << "  Année :" << film.anneeSortie << "\n"
		<< "  Recette: " << film.recette << "M$" << "\n"
		<< "Acteurs:";
	for (auto acteur : film.acteurs.spanListe())
	{
		o << *acteur;
	}
	o << endl;
	return o;
}


Film*& ListeFilms::operator[](int i)
{
	return elements[i];
}

// Fonctions pour ajouter un Film à une ListeFilms.
//[
void ListeFilms::changeDimension(int nouvelleCapacite)
{
	Film** nouvelleListe = new Film*[nouvelleCapacite];
	
	if (elements != nullptr) {  // Noter que ce test n'est pas nécessaire puique nElements sera zéro si elements est nul, donc la boucle ne tentera pas de faire de copie, et on a le droit de faire delete sur un pointeur nul (ça ne fait rien).
		nElements = min(nouvelleCapacite, nElements);
		for (int i : range(nElements))
			nouvelleListe[i] = elements[i];
		delete[] elements;
	}
	
	elements = nouvelleListe;
	capacite = nouvelleCapacite;
}

void ListeFilms::ajouterFilm(Film* film)
{
	if (nElements == capacite)
		changeDimension(max(1, capacite * 2));
	elements[nElements++] = film;
}
//]

// Fonction pour enlever un Film d'une ListeFilms (enlever le pointeur) sans effacer le film; la fonction prenant en paramètre un pointeur vers le film à enlever.  L'ordre des films dans la liste n'a pas à être conservé.
//[
// On a juste fait une version const qui retourne un span non const.  C'est valide puisque c'est la struct qui est const et non ce qu'elle pointe.  Ça ne va peut-être pas bien dans l'idée qu'on ne devrait pas pouvoir modifier une liste const, mais il y aurais alors plusieurs fonctions à écrire en version const et non-const pour que ça fonctionne bien, et ce n'est pas le but du TD (il n'a pas encore vraiment de manière propre en C++ de définir les deux d'un coup).
span<Film*> ListeFilms::enSpan() const { return span(elements, nElements); }
void ListeFilms::enleverFilm(const Film* film)
{
	for (Film*& element : enSpan()) {  // Doit être une référence au pointeur pour pouvoir le modifier.
		if (element == film) {
			if (nElements > 1)
				element = elements[nElements - 1];
			nElements--;
			return;
		}
	}
}
//]

//NOTE: Doit retourner un Acteur modifiable, sinon on ne peut pas l'utiliser pour modifier l'acteur tel que demandé dans le main, et on ne veut pas faire écrire deux versions aux étudiants dans le TD2.
shared_ptr<Acteur> ListeFilms::trouverActeur(const string& nomActeur) const
{
	for (const Film* film : enSpan()) {
		for (shared_ptr<Acteur> acteur : film->acteurs.spanListe()) {
			if (acteur->nom == nomActeur)
				return acteur;
		}
	}
	return nullptr;
}
//]

// Les fonctions pour lire le fichier et créer/allouer une ListeFilms.

shared_ptr<Acteur> lireActeur(istream& fichier, ListeFilms& listeFilms)
{
	Acteur acteur = {};
	acteur.nom            = lireString(fichier);
	acteur.anneeNaissance = lireUint16 (fichier);
	acteur.sexe           = lireUint8  (fichier);

	shared_ptr<Acteur> acteurExistant = listeFilms.trouverActeur(acteur.nom);

	if (acteurExistant != nullptr)
		return acteurExistant;
	else {
		cout << "Création Acteur " << acteur.nom << endl;
		return make_shared<Acteur>(acteur);
	}
}

Film* lireFilm(istream& fichier, ListeFilms& listeFilms)
{
	Film* film = new Film;
	film->titre       = lireString(fichier);
	film->realisateur = lireString(fichier);
	film->anneeSortie = lireUint16 (fichier);
	film->recette     = lireUint16 (fichier);
	int nElement = lireUint8 (fichier);
	ListeActeurs listeActeurs(nElement);
	film->acteurs.transmission(listeActeurs) ;
		

	; //NOTE: On aurait normalement fait le "new" au début de la fonction pour directement mettre les informations au bon endroit; on le fait ici pour que le code ci-dessus puisse être directement donné aux étudiants dans le TD2 sans qu'ils aient le "new" déjà écrit.  On aurait alors aussi un nom "film" pour le pointeur, pour suivre le guide de codage; on a mis un suffixe "p", contre le guide de codage, pour le différencier de "film".
	cout << "Création Film " << film->titre << endl;
	
	for (shared_ptr<Acteur>& acteur : film->acteurs.spanListe()) {
		acteur = lireActeur(fichier, listeFilms);
		//acteur->joueDans.ajouterFilm(film);
	}
	return film;
}

ListeFilms creerListe(string nomFichier)
{
	ifstream fichier(nomFichier, ios::binary);
	fichier.exceptions(ios::failbit);
	
	int nElements = lireUint16(fichier);

	ListeFilms listeFilms;
	for ([[maybe_unused]] int i : range(nElements)) { //NOTE: On ne peut pas faire un span simple avec ListeFilms::enSpan car la liste est vide et on ajoute des éléments à mesure.
		listeFilms.ajouterFilm(lireFilm(fichier, listeFilms));
	}
	
	return listeFilms;
}


void detruireFilm(Film* film)
{

	cout << "Destruction Film " << film->titre << endl;
	
	delete film;
}
//]

// Fonction pour détruire une ListeFilms et tous les films qu'elle contient.
//[
//NOTE: La bonne manière serait que la liste sache si elle possède, plutôt qu'on le dise au moment de la destruction, et que ceci soit le destructeur.  Mais ça aurait complexifié le TD2 de demander une solution de ce genre, d'où le fait qu'on a dit de le mettre dans une méthode.
void ListeFilms::detruire(bool possedeLesFilms)
{
	if (possedeLesFilms)
		for (Film* film : enSpan())
			detruireFilm(film);
	delete[] elements;
}
//]


void afficherListeFilms(const ListeFilms& listeFilms)
{
	static const string ligneDeSeparation = //[
		"\033[32m────────────────────────────────────────\033[0m\n";
	cout << ligneDeSeparation;
	for (const Film* film : listeFilms.enSpan()) {
		cout << *film;
		cout << ligneDeSeparation;
	}
}





Film* ListeFilms::trouverFilm(const function<bool(Film*)>& fonction)
{

	for (auto film : enSpan()){
		if (fonction(film))
		{
			return film;
		}
	}
	return nullptr;
}


int main()
{
	#ifdef VERIFICATION_ALLOCATION_INCLUS
	bibliotheque_cours::VerifierFuitesAllocations verifierFuitesAllocations;
	#endif
	bibliotheque_cours::activerCouleursAnsi();  // Permet sous Windows les "ANSI escape code" pour changer de couleurs https://en.wikipedia.org/wiki/ANSI_escape_code ; les consoles Linux/Mac les supportent normalement par défaut.

	static const string ligneDeSeparation = "\n\033[35m════════════════════════════════════════\033[0m\n";

	ListeFilms listeFilms = creerListe("films.bin");
	
	cout << ligneDeSeparation << "Le premier film de la liste est:" << endl;
	// Le premier film de la liste.  Devrait être Alien.
	cout<< *listeFilms.enSpan()[0];

	cout << ligneDeSeparation << "Les films sont:" << endl;
	// Affiche la liste des films.  Il devrait y en avoir 7.
	afficherListeFilms(listeFilms);

	listeFilms.trouverActeur("Benedict Cumberbatch")->anneeNaissance = 1976;


	//Skylien

	Film skylien = *listeFilms[0];
	skylien.titre = "Skylien";
	skylien.acteurs.spanListe()[0] = listeFilms[1]->acteurs.spanListe()[0];
	skylien.acteurs.spanListe()[0].get()->nom = "Daniel Wroughton Craig";
	cout << skylien;


	ostringstream tamponStringStream;
	tamponStringStream << skylien;
	string filmEnString = tamponStringStream.str();
	cout << filmEnString;

	Liste<string> listeTextes(2);
	*(listeTextes[0]) = "bep bep";
	*(listeTextes[1]) = "ouais ouais";
	Liste<string> listeTextes2 = listeTextes;
	listeTextes[0] = make_shared<string>("OUAIS OUAIS");
	*(listeTextes[1]) = "BEP BEP ";

	cout << *listeTextes[0] << "\n"<< *listeTextes[1] << "\n" << *listeTextes2[0] << "\n" << *listeTextes2[1] <<endl;

	cout << listeFilms.trouverFilm([](Film* film) {return film->recette == 955; })->titre<<endl;



	// Détruit et enlève le premier film de la liste (Alien).
	detruireFilm(listeFilms.enSpan()[0]);
	listeFilms.enleverFilm(listeFilms.enSpan()[0]);

	cout << ligneDeSeparation << "Les films sont maintenant:" << endl;
	afficherListeFilms(listeFilms);

	// Pour une couverture avec 0% de lignes non exécutées:
	listeFilms.enleverFilm(nullptr); // Enlever un film qui n'est pas dans la liste (clairement que nullptr n'y est pas).
	//afficherFilmographieActeur(listeFilms, "N'existe pas"); // Afficher les films d'un acteur qui n'existe pas.

	// Détruire tout avant de terminer le programme.
	listeFilms.detruire(true);
}
