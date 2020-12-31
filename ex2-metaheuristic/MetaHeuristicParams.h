#pragma once

struct MetaHeuristicsParams {
    // Probability that a new offspring is mutated.
    // Acceptable range: [0, 1)
    double mutation_probability;

    // Size of the population pool
    size_t mu;

    // Size of the offsprings before pruning
    size_t lambda;

    // Maximum number of generations without improvement
    size_t max_n_generations_without_improvement;

    // Maximum total number of generations
    size_t max_n_generations;
};