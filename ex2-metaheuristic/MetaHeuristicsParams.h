#pragma once

struct MetaHeuristicsParams {
    // Probability that a new offspring is mutated.
    // Acceptable range: [0, 1]
    double mutation_probability;

    // Probability that any two selected parents generate an offsprings, or 1-probability that
    // any two selected parents are simply copied to become offsprings.
    // Acceptable range: [0, 1]
    double crossover_rate;

    // Size of the population pool. It must be an even number.
    size_t mu;

    // Size of the offsprings before pruning. It must be an even number.
    // Acceptable range: [mu+1, +infinity)
    size_t lambda;

    // Maximum number of generations without improvement.
    size_t max_n_generations_without_improvement;

    // Maximum total number of generations.
    size_t max_n_generations;

    // Size of the tournament.
    // Acceptable range: [2, mu]
    size_t tournament_k;
};