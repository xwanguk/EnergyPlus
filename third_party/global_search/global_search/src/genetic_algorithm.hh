#ifndef GLOBAL_SEARCH__GENETIC_ALGORITHM_H_
#define GLOBAL_SEARCH__GENETIC_ALGORITHM_H_
#include <random>
#include <vector>
#include <array>

namespace globalSearch {

struct Individual
{
	std::vector<double> genes;
	double fitness;
	double relativeFitness;
	double cumulativeFitness;
};

double exampleObjectiveFunction(std::vector<double>);

class GeneticAlgorithm
{
	public: // Creation
		GeneticAlgorithm(
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
		);
		GeneticAlgorithm();

	public: // Methods
		double run();
		double randomRange(double lower, double upper);
		double nthRandomGene(int n);

	private: // Methods
		std::vector<Individual> generateInitialPopulation();
		Individual initIndividual();
		void evaluatePopulation(std::vector<Individual>& population);
		Individual fittestMemberOf(const std::vector<Individual>& population);
		std::vector<Individual> selectSurvivors(std::vector<Individual>& population);
		std::vector<Individual> crossover(const std::vector<Individual>& population);
		std::array<Individual,2> makeChildren(const Individual& mother, const Individual& father);
		void mutatePopulation(std::vector<Individual>& population);
		void startReport();
		void reportProgress(int generationNumber, const std::vector<Individual>& population, const Individual& fittest);

	private: // Data
		double (*objFn_)(std::vector<double>);
		int numberOfGenes_;
		std::vector<double> defaultGenes_;
		std::vector<bool> continuousFlags_;
		std::vector<bool> variableFlags_;
		std::vector<double> upperBounds_;
		std::vector<double> lowerBounds_;
		int populationSize_;
		int eliteSize_;
		int maximumGenerations_;
		double probabilityOfCrossover_;
		double probabilityOfMutation_;
		std::mt19937 rng_;
};
} // global_search
#endif // GLOBAL_SEARCH__GENETIC_ALGORITHM_H_

