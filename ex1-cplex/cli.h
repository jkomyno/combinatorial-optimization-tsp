#pragma once

#include <iostream>  // std::cout
#include <string>    // std::string

#include <argparse/include/argparse/argparse.hpp>

namespace cli {
    struct CLIArgs : public argparse::Args {
        std::string& filename = kwarg("f,filename", "Name of the TSP instance file");

        unsigned int& timeout_ms =
            kwarg("t,timeout-ms", "Timeout (in milliseconds)").set_default(1000u);

        // Show introductory message to the users
        void welcome() override {
            std::cout << "CPLEX MILP solver\n";
        }
    };

    CLIArgs parse(int argc, char** argv) {
        return argparse::parse<CLIArgs>(argc, argv);
    }
}  // namespace cli