#pragma once

#include <iostream>  // std::cout
#include <string>    // std::string

#include <argparse/include/argparse/argparse.hpp>

namespace cli {
    struct CLIArgs : public argparse::Args {
        std::string& filename = kwarg("f,filename", "Name of the TSP instance file");

        unsigned int& timeout_ms = kwarg("t,timeout-ms", "Timeout (in milliseconds)")
            .set_default(1000u);

        double& mutation_probability = kwarg("m,mutation-probability", "GA mutation probability")
            .set_default(0.018317703302043006);

        double& crossover_rate = kwarg("c,crossover-rate", "GA crossover rate")
            .set_default(0.9254767404002548);

        unsigned int& mu = kwarg("mu", "GA population pool size")
            .set_default(40u);

        unsigned int& lambda = kwarg("lambda", "GA offspring pool size")
            .set_default(60u);

        unsigned int& max_gen_no_improvement =
            kwarg("N,max-gen-no-improvement", "Maximum number of generations without solution improvement")
            .set_default(177u);

        unsigned int& max_gen = kwarg("M,max-gen", "Maximum number of generations")
            .set_default(408u);

        unsigned int& k = kwarg("k", "Size of the GA tournament")
            .set_default(13u);

        // Show introductory message to the users
        void welcome() override {
            std::cout << "Metaheuristic solver\n";
        }
    };

    CLIArgs parse(int argc, char** argv) {
        return argparse::parse<CLIArgs>(argc, argv); 
    }
}