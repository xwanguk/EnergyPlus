#include <iostream>
#include "genetic_algorithm.hh"

int main(int argc, char *argv[])
{
	auto ga = globalSearch::GeneticAlgorithm();
	auto out = ga.run();
	return 0;
}
