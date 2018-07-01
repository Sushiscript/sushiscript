#include "./util.h"
#include "algorithm"

using namespace sushi::pipeline;

TEST(PipelineParser, Void1) {
    int argc = 1;
    const char *argv[] = {"./sushi"};
    auto config = Pipeline::Parser(argc, argv);
    EXPECT_EQ(config.command_type, Config::CommandType::Void);
}

TEST(PipelineParser, Version1) {
    int argc = 2;
    const char *argv[] = {"./sushi", "-v"};
    auto config = Pipeline::Parser(argc, argv);
    EXPECT_EQ(config.command_type, Config::CommandType::Version);
}

TEST(PipelineParser, Version2) {
    int argc = 4;
    const char *argv[] = {"./sushi", "--version", "ashdfika", "--.sh"};
    auto config = Pipeline::Parser(argc, argv);
    EXPECT_EQ(config.command_type, Config::CommandType::Version);
}

TEST(PipelineParser, Version3) {
    int argc = 4;
    const char *argv[] = {"./sushi", "--Version", "ashdfika", "--.sh"};
    auto config = Pipeline::Parser(argc, argv);
    EXPECT_EQ(config.command_type, Config::CommandType::UnKnown);
}

TEST(PipelineParser, Help1) {
    int argc = 4;
    const char *argv[] = {"./sushi", "--help", "ashdfika", "--.sh"};
    auto config = Pipeline::Parser(argc, argv);
    EXPECT_EQ(config.command_type, Config::CommandType::Help);
}

TEST(PipelineParser, Help2) {
    int argc = 4;
    const char *argv[] = {"./sushi", "-h", "ashdfika", "--.sh"};
    auto config = Pipeline::Parser(argc, argv);
    EXPECT_EQ(config.command_type, Config::CommandType::Help);
}

TEST(PipelineParser, Run1) {
    int argc = 2;
    const char *argv[] = {"./sushi", "--"};
    auto config = Pipeline::Parser(argc, argv);
    EXPECT_EQ(config.command_type, Config::CommandType::Run);
    EXPECT_TRUE(
        std::find(config.input_path.begin(), config.input_path.end(), "--") !=
        config.input_path.end());
    EXPECT_TRUE(config.input_path.size() == 1);
}

TEST(PipelineParser, Run2) {
    int argc = 2;
    const char *argv[] = {"./sushi", "..."};
    auto config = Pipeline::Parser(argc, argv);
    EXPECT_EQ(config.command_type, Config::CommandType::Run);
    EXPECT_TRUE(
        std::find(config.input_path.begin(), config.input_path.end(), "...") !=
        config.input_path.end());
    EXPECT_TRUE(config.input_path.size() == 1);
}

TEST(PipelineParser, Run3) {
    int argc = 3;
    const char *argv[] = {"./sushi", "...", "..."};
    auto config = Pipeline::Parser(argc, argv);
    EXPECT_EQ(config.command_type, Config::CommandType::UnKnown);
}

TEST(PipelineParser, Build1) {
    int argc = 4;
    const char *argv[] = {"./sushi", "--", "-o", "--.sh"};
    auto config = Pipeline::Parser(argc, argv);
    EXPECT_EQ(config.command_type, Config::CommandType::Build);
    EXPECT_TRUE(
        std::find(config.input_path.begin(), config.input_path.end(), "--") !=
        config.input_path.end());
    EXPECT_TRUE(config.input_path.size() == 1);
    EXPECT_TRUE(config.output_path == "--.sh");
}

TEST(PipelineParser, Build2) {
    int argc = 4;
    const char *argv[] = {"./sushi", "--", "-O", "--.sh"};
    auto config = Pipeline::Parser(argc, argv);
    EXPECT_EQ(config.command_type, Config::CommandType::UnKnown);
}
