/**
* Programme qui contient les classes ListeFilms et Liste ainsi que les structures utilisés.
* \file  structures_solutionnaire_td2_3.hpp
* \author Wassim Keddache et Chu Xiang Su
* \date   2021-03-07
* Créé le 15 février 2015
*/
#pragma once

#include <string>
#include "gsl/span"
#include <memory>
#include <functional>

using gsl::span;
using namespace std;
struct Film; struct Acteur; // Permet d'utiliser les types alors qu'ils seront défini après.


class ListeFilms 
{
public:

	ListeFilms() = default;
	void ajouterFilm(Film* film);
	void enleverFilm(const Film* film);
	shared_ptr<Acteur> trouverActeur(const std::string& nomActeur) const; 
	span<Film*> enSpan() const;
	int size() const { return nElements; }
	void detruire(bool possedeLesFilms = false);
	Film* trouverFilm(const function<bool(Film*)>& fonction);
	
	Film*& operator[](int i);
	
	friend ostream& operator<< (ostream& o, const Film& film);

private:
	void changeDimension(int nouvelleCapacite);

	int capacite = 0, nElements = 0;
	Film** elements = nullptr; // Pointeur vers un tableau de Film*, chaque Film* pointant vers un Film.
};

template <typename T>
class Liste
{
public:
	Liste() = default;
	Liste(int valeur)
	{
		nElements_ = valeur;
		elements_ = make_unique<shared_ptr<T>[]>(nElements_);
		for (int i = 0; i < nElements_; i++)
		{
			elements_[i] = make_shared<T>();
		}
		capacite_ = valeur;
	}

	
	Liste(const Liste<T>& liste)
	{
		elements_ = make_unique<shared_ptr<T>[]>(liste.nElements_);
		nElements_ = liste.nElements_;
		capacite_ = liste.capacite_;
		for (int i = 0; i < liste.nElements_; i++) //for :
		{
			elements_[i] = liste.elements_[i];
		}
	}
	span<shared_ptr<T>> spanListe() const { return span(elements_.get(), nElements_); }
	
	void transmission(Liste& liste)
	{
		elements_ = move(liste.elements_);
		nElements_ = liste.nElements_;
		capacite_ = liste.capacite_;
	}
	
	void operator=(const Liste<T>& liste) 
	{
		elements_ = make_unique<shared_ptr<T>[]>(liste.nElements_);
		nElements_ = liste.nElements_;
		capacite_ = liste.capacite_;
		for (int i = 0; i < liste.nElements_; i++) 
		{
			elements_[i] = liste.elements_[0];
		}

	}

	friend ostream& operator<< (ostream& o, const Liste<T>& liste) 
	{
		for (auto element : liste.spanListe())
		{
			o << *element;
		}

		return o;
	}

	shared_ptr<T>& operator[](int i)
	{
		return elements_[i];
	}

private:
	int capacite_ = 0, nElements_ = 0;
	unique_ptr<shared_ptr<T>[]> elements_;
};

using ListeActeurs = Liste<Acteur>;
struct Film
{
	std::string titre, realisateur; // Titre et nom du réalisateur (on suppose qu'il n'y a qu'un réalisateur).
	int anneeSortie, recette; // Année de sortie et recette globale du film en millions de dollars
	ListeActeurs acteurs;
};

struct Acteur
{
	string nom="N/A"; int anneeNaissance=0; char sexe='N';
	
};






