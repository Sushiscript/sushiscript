#ifndef SUSHI_AST_STATEMENT_H_
#define SUSHI_AST_STATEMENT_H_

#include "expression.h"
#include <memory>
#include <vector>

namespace sushi {

class Statement {
  public:
  private:
};

class Program {
  public:
  private:
    std::vector<std::unique_ptr<Statement>> statements_;
};

class IfStmt : public Statement {
  public:
  private:
    std::unique_ptr<Expression> condition_;
    std::unique_ptr<Program> true_body_;
    // false_body_ can be nullptr
    std::unique_ptr<Program> false_body_;
};

class ReturnStmt : public Statement {
  public:
  private:
    std::unique_ptr<Expression> value_;
};

class SwitchCase {
  public:
  private:
    std::unique_ptr<Expression> condition_;
    std::unique_ptr<Program> body_;
};

class SwitchStmt : public Statement {
  public:
  private:
    std::vector<SwitchCase> cases_;
    // default can be nullptr
    std::unique_ptr<Program> default_;
};

class LoopCondition {
  public:
  private:
};

class LoopControl : public Statement {
  public:
  private:
    int level_ = 1;
};

class ForStmt : public Statement {
  public:
  private:
    std::unique_ptr<LoopCondition> condition_;
    std::unique_ptr<Program> body_;
};

} // namespace sushi

#endif // SUSHI_AST_STATEMENT_H_
