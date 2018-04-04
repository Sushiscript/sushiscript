#ifndef SUSHI_AST_STATEMENT_H_
#define SUSHI_AST_STATEMENT_H_

#include "./expression.h"
#include "./type.h"
#include "sushi/util/visitor.h"
#include <cstdint>
#include <memory>
#include <vector>

namespace sushi {

class VariableDef;
class FunctionDef;
class IfStmt;
class ReturnStmt;
class SwitchStmt;
class ForStmt;
class LoopControlStmt;

using StatementVisitor = sushi::util::Visitor<
    VariableDef, FunctionDef, IfStmt, ReturnStmt, SwitchStmt, ForStmt,
    LoopControlStmt>;

class Statement {
  public:
    SUSHI_VISITABLE(StatementVisitor)
    virtual ~Statement() {}

  private:
};

class Program {
  public:
  private:
    std::vector<std::unique_ptr<Statement>> statements_;
};

class VariableDef : public Statement {
  public:
    SUSHI_ACCEPT_VISITOR(Statement)

  private:
    bool export_;
    std::string name_;
    // type can be nullptr
    std::unique_ptr<Type> type_;
    std::unique_ptr<Expression> value_;
};

struct Parameter {
    std::string name_;
    std::unique_ptr<Type> type;
};

class FunctionDef : public Statement {
  public:
    SUSHI_ACCEPT_VISITOR(Statement)

  private:
    std::string name_;
    std::vector<Parameter> params_;
    std::unique_ptr<Program> body_;
    bool export_;
};

class IfStmt : public Statement {
  public:
    SUSHI_ACCEPT_VISITOR(Statement)

  private:
    std::unique_ptr<Expression> condition_;
    std::unique_ptr<Program> true_body_;
    // false_body_ can be nullptr
    std::unique_ptr<Program> false_body_;
};

class ReturnStmt : public Statement {
  public:
    SUSHI_ACCEPT_VISITOR(Statement)

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
    SUSHI_ACCEPT_VISITOR(Statement)

  private:
    std::vector<SwitchCase> cases_;
    // default can be nullptr
    std::unique_ptr<Program> default_;
};

class LoopCondition {
  public:
    bool IsRange() const {
        return not ident_name_.empty();
    }

  private:
    std::string ident_name_;
    std::unique_ptr<Expression> condition_;
};

class ForStmt : public Statement {
  public:
    SUSHI_ACCEPT_VISITOR(Statement)

  private:
    std::unique_ptr<LoopCondition> condition_;
    std::unique_ptr<Program> body_;
};

class LoopControlStmt : public Statement {
  public:
    SUSHI_ACCEPT_VISITOR(Statement)

    enum class Type : uint8_t { kBreak = 1, kContinue = 2 };

  private:
    int level_ = 1;
    LoopControlStmt::Type control_type_;
};

} // namespace sushi

#endif // SUSHI_AST_STATEMENT_H_
