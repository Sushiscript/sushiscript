#ifndef SUSHI_AST_STATEMENT_H_
#define SUSHI_AST_STATEMENT_H_

#include "expression.h"
#include <memory>
#include <vector>

namespace sushi {

class StatementVisitor;

class Statement {
  public:
    virtual void AcceptVisitor(StatementVisitor &visitor) = 0;

  private:
};

class VariableDef;
class FunctionDef;
class IfStmt;
class ReturnStmt;
class SwitchStmt;
class ForStmt;
class LoopControl;

class StatementVisitor {
  public:
    virtual void Visit(VariableDef *) = 0;
    virtual void Visit(FunctionDef *) = 0;
    virtual void Visit(IfStmt *) = 0;
    virtual void Visit(ReturnStmt *) = 0;
    virtual void Visit(SwitchStmt *) = 0;
    virtual void Visit(ForStmt *) = 0;
    virtual void Visit(LoopControl *) = 0;
};

class Program {
  public:
  private:
    std::vector<std::unique_ptr<Statement>> statements_;
};

class VariableDef : public Statement {
  public:
    virtual void AcceptVisitor(StatementVisitor &visitor) {
        visitor.Visit(this);
    }

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
    virtual void AcceptVisitor(StatementVisitor &visitor) {
        visitor.Visit(this);
    }

  private:
    std::string name_;
    std::vector<Parameter> params_;
    std::unique_ptr<Program> body_;
    bool export_;
};

class IfStmt : public Statement {
  public:
    virtual void AcceptVisitor(StatementVisitor &visitor) {
        visitor.Visit(this);
    }

  private:
    std::unique_ptr<Expression> condition_;
    std::unique_ptr<Program> true_body_;
    // false_body_ can be nullptr
    std::unique_ptr<Program> false_body_;
};

class ReturnStmt : public Statement {
  public:
    virtual void AcceptVisitor(StatementVisitor &visitor) {
        visitor.Visit(this);
    }

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
    virtual void AcceptVisitor(StatementVisitor &visitor) {
        visitor.Visit(this);
    }

  private:
    std::vector<SwitchCase> cases_;
    // default can be nullptr
    std::unique_ptr<Program> default_;
};

class LoopCondition {
  public:
  private:
};

class ForStmt : public Statement {
  public:
    virtual void AcceptVisitor(StatementVisitor &visitor) {
        visitor.Visit(this);
    }

  private:
    std::unique_ptr<LoopCondition> condition_;
    std::unique_ptr<Program> body_;
};

class LoopControl : public Statement {
  public:
    virtual void AcceptVisitor(StatementVisitor &visitor) {
        visitor.Visit(this);
    }

  private:
    int level_ = 1;
};

} // namespace sushi

#endif // SUSHI_AST_STATEMENT_H_
