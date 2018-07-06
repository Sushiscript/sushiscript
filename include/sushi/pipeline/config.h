#ifndef SUSHI_PIPELINE_CONFIG_H_
#define SUSHI_PIPELINE_CONFIG_H_

#include <map>
#include <string>
#include <vector>

namespace sushi {
namespace pipeline {

const std::string kBashHeader = "#!/bin/bash\n\n";
const std::string kSushiVersion = "SushiScript Version 0.1.0\n";
const std::string kNoInputError = "sushi: error: not input files\n";
const std::string kUnKownError = "sushi: error: unkown options/arguements\n\n"
                                 "'sushi -h' for help\n";
const std::string kHelpMessage =
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
    "   -h | --help     display sushiscript help message\n";

struct Config {
    enum CommandType { UnKnown, Void, Run, Build, Version, Help };

    CommandType command_type = CommandType::UnKnown;
    // bool set_quiet = false;
    // bool set_parrel = false;
    std::string output_path = "";
    std::vector<std::string> input_path = std::vector<std::string>{};
};

const std::map<Config::CommandType, std::string> kMessageDict =
    std::map<Config::CommandType, std::string>{
        {Config::CommandType::Help, kHelpMessage},
        {Config::CommandType::Version, kSushiVersion},
        {Config::CommandType::Void, kNoInputError},
        {Config::CommandType::UnKnown, kUnKownError},
    };

} // namespace pipeline
} // namespace sushi

#endif // SUSHI_PIPELINE_CONFIG_H_
