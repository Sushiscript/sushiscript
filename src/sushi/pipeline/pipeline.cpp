#include "sushi/pipeline.h"
#include <string>
#include <vector>
#include <iostream>

namespace sushi{
namespace pipeline {

void Pipeline::Main() {
    bash_str = kBashHeader + bash_str;
    int result = system(bash_str.c_str());
    exit(result);
}

void Pipeline::DisplayHelpMenu() {
    std::cout << kHelpMenu;
}
void Pipeline::DesplayVertion() {
    std::cout << kSushiVersion;
}
void Pipeline::ExecSigleFile(std::string file_path) {
    // pass
}

Config Pipeline::Parser(int argc, char* argv[]) {
    Config config;
    if (argc < 2) {
        config.command_type = Config::Void;
        return config;
    } else {
        // version
        if (strcmp(argv[1], "-v") or 
            strcmp(argv[1], "--version")) {
            config.command_type = Config::Version;
            return config;
        }
        // help
        if (strcmp(argv[1], "-h") or 
            strcmp(argv[1], "--help")) {
            config.command_type = Config::Help;
            return config;
        }
        // run or build
        config.input_path.push_back(std::string(argv[1]));
        // run
        if (argc == 2) {
            config.command_type = Config::Run;
            return config;
        }
        // build
        if (argc == 4 and
            strcmp(argv[2], "-o")) {
            config.output_path = argv[3];
            config.command_type = Config::Build;
            return config;
        }
        config.command_type = Config::UnKnown;
        return config;
    }
}


} // namespace pipeline
} // namespace sushi
