#ifndef SUSHI_PIPELINE_CONFIG_H_
#define SUSHI_PIPELINE_CONFIG_H_

#include <string>
#include <vector>

namespace sushi{
namespace pipeline {

const std::string kBashHeader = "#/bin/bash\n\n";
const std::string kSushiVersion = "SushiScript Version 0.0.1\n";
const std::string kNoInputError = 
    "sushi: error: not input files\n";
const std::string kHelpMenu = 
    "usage: \n"
    "   sushi <file(s)>\n"
    "       (run file(s)\n"
    "   sushi [options]\n"
    "       (system options)\n"
    "   sushi [options] <file(s)> -o output_path\n"
    "       (build files)\n"
    "\n"
    "options: \n"
    "   -v | --version  display sushiscript version\n"
    "   -h | --help     display sushiscript help menu\n"
    "options for '-o': \n"
    "   empty now\n"
    "";


struct Config {
    enum CommandType {UnKnown, Void, Run, Build, Version, Help};

    CommandType command_type = CommandType::UnKnown;
    // bool set_quiet = false;
    // bool set_parrel = false;
    std::string output_path = "";
    std::vector<std::string> input_path = std::vector<std::string> {};
};

} // namespace pipeline
} // namespace sushi

#endif // SUSHI_PIPELINE_CONFIG_H_
