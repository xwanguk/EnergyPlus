#include "genetic_algorithm.hh"
#include <algorithm>
#include <assert.h>
#include <numeric>
#include <cstdlib>
#include <chrono>
#include <random>
#include <vector>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <cmath>
#include <ctime>
#include <cstring>
#include <array>

double
globalSearch::exampleObjectiveFunction(std::vector<double> xs)
{
	double x0 = xs.at(0);
	double x1 = xs.at(1);
	double x2 = xs.at(2);
	return (x0 * x0) - (x0 * x1) + x2;
}

globalSearch::GeneticAlgorithm::GeneticAlgorithm(
	double (*objFn)(std::vector<double>),
	std::vector<double> defaultGenes,
	std::vector<bool> continuousFlags,
	std::vector<bool> variableFlags,
	std::vector<double> upperBounds,
	std::vector<double> lowerBounds,
	int populationSize,
	int eliteSize,
	int maximumGenerations,
	double probabilityOfCrossover,
	double probabilityOfMutation
) :
	objFn_(objFn),
	defaultGenes_(defaultGenes),
	continuousFlags_(continuousFlags),
	variableFlags_(variableFlags),
	upperBounds_(upperBounds),
	lowerBounds_(lowerBounds),
	populationSize_(populationSize),
	eliteSize_(eliteSize),
	maximumGenerations_(maximumGenerations),
	probabilityOfCrossover_(probabilityOfCrossover),
	probabilityOfMutation_(probabilityOfMutation),
	rng_()
{
	numberOfGenes_ = defaultGenes_.size();
	assert(numberOfGenes_ == continuousFlags_.size());
	assert(numberOfGenes_ == variableFlags_.size());
	assert(numberOfGenes_ == upperBounds_.size());
	assert(numberOfGenes_ == lowerBounds_.size());
	assert(eliteSize_ < populationSize_);
}

globalSearch::GeneticAlgorithm::GeneticAlgorithm() :
	globalSearch::GeneticAlgorithm::GeneticAlgorithm{
		&globalSearch::exampleObjectiveFunction,
		{2.5, 2.5, 0.0},
		{true, true, true},
		{true, true, true},
		{5.0, 5.0, 2.0},
		{0.0, 0.0, -2.0},
		50,
		1,
		1000,
		0.8,
		0.15}
{}

double
globalSearch::GeneticAlgorithm::run()
{
	startReport();
	std::vector<globalSearch::Individual> population = generateInitialPopulation();
	std::vector<globalSearch::Individual> nextPopulation;
	globalSearch::Individual fittest;
	evaluatePopulation(population);
	fittest = fittestMemberOf(population);
	for (int generation=0; generation < maximumGenerations_; generation++) {
		nextPopulation = selectSurvivors(population);
		nextPopulation = crossover(nextPopulation);
		mutatePopulation(nextPopulation);
		reportProgress(generation, population, fittest);
		//evaluate ( );
		//elitist ( );
	}
	return 0.0;
}

double
globalSearch::GeneticAlgorithm::randomRange(
	double lower,
	double upper
)
{
	std::uniform_real_distribution<double> distribution(lower, upper);
	return distribution(rng_);
}

double
globalSearch::GeneticAlgorithm::nthRandomGene(int n) {
	double offset = 0.49999;
	bool continuous = continuousFlags_.at(n);
 	bool variable = variableFlags_.at(n);
	double newValue, lowerBound, upperBound;
	if (variable) {
		lowerBound = lowerBounds_.at(n);
		upperBound = upperBounds_.at(n);
		if (continuous) {
			newValue = randomRange(lowerBound, upperBound);
		} else {
			newValue = std::round(
					randomRange(lowerBound - offset, upperBound + offset));
		}
	} else {
		newValue = defaultGenes_.at(n);
	}
	return newValue;
}

std::vector<globalSearch::Individual>
globalSearch::GeneticAlgorithm::generateInitialPopulation()
{
	std::vector<globalSearch::Individual> inds;
	for (int i=0; i < populationSize_; i++) {
		inds.push_back(initIndividual());
	}
	return inds;
}

globalSearch::Individual
globalSearch::GeneticAlgorithm::initIndividual()
{
	std::vector<double> vars;
	for (int i = 0; i < numberOfGenes_; ++i) {
		vars.push_back(nthRandomGene(i));
	}
	globalSearch::Individual indy;
	indy.genes = vars;
	indy.fitness = 0.0;
	indy.relativeFitness = 0.0;
	indy.cumulativeFitness = 0.0;
	return indy;
}

void
globalSearch::GeneticAlgorithm::evaluatePopulation(std::vector<globalSearch::Individual>& pop)
{
	for (auto pIt = pop.begin(); pIt != pop.end(); ++pIt) {
		(*pIt).fitness = (*objFn_)((*pIt).genes);
	}
	return;
}

globalSearch::Individual
globalSearch::GeneticAlgorithm::fittestMemberOf(const std::vector<globalSearch::Individual>& pop)
{
	double bestFitnessThusFar;
	globalSearch::Individual fittest;
	bool first = true;
	for (const auto member : pop) {
		if (first) {
			fittest = member;
			bestFitnessThusFar = fittest.fitness;
			first = false;
			continue;
		}
		if (member.fitness > bestFitnessThusFar) {
			fittest = member;
			bestFitnessThusFar = fittest.fitness;
		}
	}
	return fittest;
}

std::vector<globalSearch::Individual>
globalSearch::GeneticAlgorithm::selectSurvivors(std::vector<globalSearch::Individual>& pop)
{
	std::vector<globalSearch::Individual> newPop;
	int i, j;
	double p;
	double cumulativeFitness = 0.0;
	double totalFitnessOfPopulation = std::accumulate(
			pop.begin(),
			pop.end(),
			0.0,
			[](double sum, globalSearch::Individual ind) { return sum + ind.fitness; });
	for (auto memIt = pop.begin(); memIt != pop.end(); ++memIt) {
		(*memIt).relativeFitness = (*memIt).fitness / totalFitnessOfPopulation;
	}
	for (auto memIt = pop.begin(); memIt != pop.end(); ++memIt) {
		cumulativeFitness += (*memIt).relativeFitness;
		(*memIt).cumulativeFitness = cumulativeFitness;
	}
	// Roulette Wheel Selection for the New Population
	for (i = 0; i < populationSize_; ++i) {
		p = randomRange(0.0, 1.0);
		if (p < pop.at(0).cumulativeFitness) {
			newPop.push_back(pop.at(0));
		} else {
			for (j = 0; j < (populationSize_ - 1); ++j) {
				if ((p >= pop.at(j).cumulativeFitness) &&
						(p < pop.at(j+1).cumulativeFitness)) {
					newPop.push_back(pop.at(j+1));
				}
			}
		}
	}
	return newPop;
}

std::vector<globalSearch::Individual>
globalSearch::GeneticAlgorithm::crossover(const std::vector<globalSearch::Individual>& pop)
{
	globalSearch::Individual mother;
	globalSearch::Individual father;
	std::array<globalSearch::Individual,2> children;
	bool has_mother = false;
	double p;
	std::vector<globalSearch::Individual> nextPop;
	for (const auto member : pop) {
		p = randomRange(0.0, 1.0);
		if (p < probabilityOfCrossover_) {
			if (has_mother) {
				father = member;
				children = makeChildren(mother, father);
				nextPop.push_back(children.at(0));
				nextPop.push_back(children.at(1));
				has_mother = false;
			} else {
				mother = member;
				has_mother = true;
			}
		} else {
			nextPop.push_back(member);
		}
	}
	return nextPop;
}

// Note that it is irrelevant if the genes are variable or not
// as genes that are not variable (i.e., constant) will have
// the same value for mother and father. Similarly, continuous
// or discrete is also irrelevant for making children
std::array<globalSearch::Individual,2>
globalSearch::GeneticAlgorithm::makeChildren(
		const globalSearch::Individual& mom,
		const globalSearch::Individual& dad
)
{
	std::array<globalSearch::Individual,2> children;
	double v1, v2, p;
	auto first = globalSearch::Individual();
	auto second = globalSearch::Individual();
	for (int gIdx = 0; gIdx < numberOfGenes_; gIdx++) {
		v1 = mom.genes.at(gIdx);
		v2 = dad.genes.at(gIdx);
		p = randomRange(0.0, 1.0);
		if (p < 0.5) {
			first.genes.push_back(v1);
			second.genes.push_back(v2);
		} else {
			first.genes.push_back(v2);
			second.genes.push_back(v1);
		}
	}
	children.at(0) = first;
	children.at(1) = second;
	return children;
}

void
globalSearch::GeneticAlgorithm::mutatePopulation(std::vector<globalSearch::Individual>& pop)
{
	double p, newValue;
	bool varFlag;
	for (auto member : pop) {
		for (int geneIdx = 0; geneIdx < numberOfGenes_; ++geneIdx) {
			p = randomRange(0.0, 1.0);
			if (p < probabilityOfMutation_) {
				member.genes.at(geneIdx) = nthRandomGene(geneIdx);
			}
		}
	}
}

void
globalSearch::GeneticAlgorithm::startReport()
{
	std::cout << "#globalSearch/GeneticAlgorithm\n";
	std::cout << "{:population-size " << populationSize_ << "\n";
	std::cout << " :maximum-generations " << maximumGenerations_ << "\n";
	std::cout << " :elite-size " << eliteSize_ << "\n";
	std::cout << " :probability-of-crossover " << std::setprecision(3) << probabilityOfCrossover_ << "\n";
	std::cout << " :probability-of-mutation " << std::setprecision(3) << probabilityOfMutation_ << "\n";
	std::cout << " :number-of-genes " << numberOfGenes_ << "\n";
	std::cout << " :genes\n";
	for (auto i=0; i<numberOfGenes_; ++i) {
		if (i == 0) {
			std::cout << " [{idx " << i << ", ";
		} else {
			std::cout << "  {idx " << i << ", ";
		}
		std::cout << "default " << defaultGenes_.at(i) << ", ";
		std::cout << "continuous? " << continuousFlags_.at(i) << ", ";
		std::cout << "variable? " << variableFlags_.at(i) << ", ";
		std::cout << "bounds [" << lowerBounds_.at(i) << ", " << upperBounds_.at(i) << "]}";
		if (i == numberOfGenes_ - 1) {
			std::cout << "]}\n";
		} else {
			std::cout << "\n"; 
		}
	}
	std::cout << "\n";
	// line 1
	std::cout << std::setw(14) << "Generation No.";
	std::cout << "  ";
	std::cout << std::setw(14) << "Best Fitness";
	std::cout << "  ";
	std::cout << std::setw(14) << "Pop Fitness";
	std::cout << "  ";
	std::cout << std::setw(14) << "Avg Fitness\n";
	// line 2
	std::cout << std::setfill('-') << std::setw(14) << "";
	std::cout << "  ";
	std::cout << std::setfill('-') << std::setw(14) << "";
	std::cout << "  ";
	std::cout << std::setfill('-') << std::setw(14) << "";
	std::cout << "  ";
	std::cout << std::setfill('-') << std::setw(14) << "" << "\n";
	std::cout << std::setfill(' ');
}

void
globalSearch::GeneticAlgorithm::reportProgress(
		int generationNumber,
		const std::vector<globalSearch::Individual>& pop,
		const globalSearch::Individual& fittest
)
{
	double totalFitness = std::accumulate(
			pop.begin(),
			pop.end(),
			0.0,
			[](double sum, globalSearch::Individual ind) { return sum + ind.fitness; });
	double avgFitness = totalFitness / ((double)populationSize_);
	double bestFitness = fittest.fitness;
	std::cout << std::setw(14) << std::setprecision(6) << generationNumber << "  ";
	std::cout << std::setw(14) << std::setprecision(6) << bestFitness << "  ";
	std::cout << std::setw(14) << std::setprecision(6) << totalFitness << "  ";
	std::cout << std::setw(14) << std::setprecision(6) << avgFitness << "\n";
}
