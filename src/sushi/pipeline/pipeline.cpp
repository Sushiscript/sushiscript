#include "sushi/pipeline.h"
#include "sushi/code-generation/code-generation.h"
#include "sushi/lexer.h"
#include "sushi/parser/parser.h"
#include "sushi/scope.h"
#include "sushi/type-system.h"
#include "sushi/type-system/type-checking.h"

#include <cstring>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <sushi/ast.h>
#include <vector>

namespace sushi {
namespace pipeline {

#define PRINT_ERROR(POSITION, ERROR_TYPE, OTHERS)                              \
    std::cout << "error:" << (POSITION) << ":" << (ERROR_TYPE) << " " << OTHERS\
              << std::endl;

Pipeline::Pipeline(int argc, const char *argv[]) {
    config = Parser(argc, argv);
};

void Pipeline::Main() {
    switch (config.command_type) {
    case Config::CommandType::Run:
        ExecSingleFile(config.input_path.front());
        break;
    case Config::CommandType::Build:
        BuildSingleFile(config.input_path.front(), config.output_path);
        break;
    default: DisplayMessage(); break;
    }
}

void Pipeline::DisplayMessage() {
    std::cout << kMessageDick.at(config.command_type);
}

void Pipeline::ExecSingleFile(std::string file_path) {
    std::ifstream in(file_path);
    std::stringstream buffer;
    buffer << in.rdbuf();
    in.close();

    auto bash_str = TransToSushi(buffer.str());
    int result = system(bash_str.c_str());
}

void Pipeline::BuildSingleFile(std::string file_path, std::string output_path) {
    std::ifstream in(file_path);
    std::stringstream buffer;
    buffer << in.rdbuf();
    in.close();

    auto bash_str = TransToSushi(buffer.str());

    std::ofstream out(output_path);
    out << bash_str;
    out.close();
}

std::string Pipeline::TransToSushi(std::string s) {
    std::istringstream iss(s);
    // lexer
    lexer::Lexer lexer(iss, {"", 1, 1});
    // parser
    parser::Parser p(std::move(lexer));
    auto result = p.Parse();
    for (auto &error : result.errors) {
        PRINT_ERROR("parser", error.ToString(), error.position);
    }
    if (not result.errors.empty()) exit(-1);
    // scope check
    scope::Environment enviroment;
    auto scope_errors = scope::ScopeCheck(result.program, enviroment);
    for (auto &error : scope_errors) {
        PRINT_ERROR("scope", error.ToString(), error.error_detail);
    }
    if (!scope_errors.empty()) exit(-1);

    // type check
    auto errors = type::Check(result.program, enviroment);
    for (auto &error : errors) {
        PRINT_ERROR("type-check", error.ToString(), "type-check error");
    }
    if (not errors.empty()) exit(-1);

    // code gen
    code_generation::CodeGenerator generator;
    auto codes =  generator.GenCode(result.program, enviroment);
    return (kBashHeader + codes);
}

Config Pipeline::Parser(int argc, const char *argv[]) {
    Config config;
    if (argc < 2) {
        config.command_type = Config::Void;
        return config;
    } else {
        // version
        if (!strcmp(argv[1], "-v") or !strcmp(argv[1], "--version")) {
            config.command_type = Config::Version;
            return config;
        }
        // help
        if (!strcmp(argv[1], "-h") or !strcmp(argv[1], "--help")) {
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
        if (argc == 4 and !strcmp(argv[2], "-o")) {
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
