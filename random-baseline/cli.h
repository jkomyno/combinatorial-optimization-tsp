#pragma once

#include <argparse/include/argparse/argparse.hpp>
#include <iostream>  // std::cout
#include <string>    // std::string

namespace cli {
    struct CLIArgs : public argparse::Args {
        std::string& filename = kwarg("f,filename", "Name of the TSP instance file");

        unsigned int& timeout_ms =
            kwarg("t,timeout-ms", "Timeout (in milliseconds)").set_default(1000u);

        bool& show_path = flag("s,show-path", "Show the path of the solution").set_default(false);

        // Show introductory message to the users
        void welcome() override {
            std::cout << "Random baseline solver\n";
        }
    };

    CLIArgs parse(int argc, char** argv) {
        return argparse::parse<CLIArgs>(argc, argv);
    }
}  // namespace cli