#ifndef SUSHI_PIPELINE_PIPELINE_H_
#define SUSHI_PIPELINE_PIPELINE_H_

#include <string>

namespace sushi {
namespace pipeline {

class Pipeline {
  private:
    // config
    Config config;
    // std::string bash_str = "echo \"helloworld\"";

  public:
    Pipeline(int argc, const char *argv[]);
    void Main();
    void DisplayMessage();
    void ExecSingleFile(std::string file_path);
    void BuildSingleFile(std::string file_path, std::string output_path);
    static std::string TransToSushi(std::string);
    static Config Parser(int argc, const char *argv[]);
};

} // namespace pipeline
} // namespace sushi

#endif // SUSHI_PIPELINE_PIPELINE_H_
