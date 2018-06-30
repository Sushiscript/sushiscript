#ifndef SUSHI_PIPELINE_PIPELINE_H_
#define SUSHI_PIPELINE_PIPELINE_H_

namespace sushi{
namespace pipeline {

class Pipeline {
  private:
    std::string bash_str = "echo \"helloworld\"";

  public:
    Pipeline(int argc, char* argv[]) {};
    void Main();
    void DisplayHelpMenu();
    void DesplayVertion();
    void ExecSigleFile(std::string file_path);
    static Config Parser(int argc, char* argv[]);
};

} // namespace pipeline
} // namespace sushi

#endif // SUSHI_PIPELINE_PIPELINE_H_
