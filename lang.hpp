#ifndef HEADER_CODING_TASK_BRULL_LANG
#define HEADER_CODING_TASK_BRULL_LANG

#include <memory>
#include <variant>
#include <iostream>

namespace lang
{
  /*
   * Part one: The AST itself
   */
  struct ASTNode
  {
    virtual ~ASTNode () {}
  };

  using AST = std::unique_ptr<ASTNode>;

  struct IntLiteral : public ASTNode
  {
    int value;

    IntLiteral (int const value) : value (value) {}
  };

  struct BoolLiteral : public ASTNode
  {
    bool value;

    BoolLiteral (bool const value) : value (value) {}
  };

  struct Operator : public ASTNode
  {
    enum Type
    {
      ADDITION,
      SUBTRACTION,
      MULTIPLICATION,
      DIVISION,
      NOT,
      AND,
      OR, // unary
      EQUAL
    };

    Type type;
    AST arg_1;
    AST arg_2; // Is nullptr for unary operator.

    Operator (Type const type, AST arg_1, AST arg_2)
      : type (type)
      , arg_1 (std::move (arg_1))
      , arg_2 (std::move (arg_2))
    { }
  };

  /*
   * Part two: DSL for ASTs
   */
  namespace literals
  {
    AST operator "" _i (unsigned long long i)
    {
      return std::make_unique<IntLiteral> (i);
    }

    AST operator "" _b (unsigned long long b)
    {
      if (!(b == 0 || b == 1))
      {
        throw std::runtime_error (
            "only 0_b and 1_b allowed as bool literals");
      }

      return std::make_unique<BoolLiteral> (b);
    }
  }

  #define LANG_BINARY_EXPRESSION(op, type) \
    AST operator op (AST lhs, AST rhs) \
    { \
      return std::make_unique<Operator> ( \
          Operator::type, std::move (lhs), std::move (rhs)); \
    }

  LANG_BINARY_EXPRESSION (+, ADDITION);
  LANG_BINARY_EXPRESSION (-, SUBTRACTION);
  LANG_BINARY_EXPRESSION (*, MULTIPLICATION);
  LANG_BINARY_EXPRESSION (/, DIVISION);

  LANG_BINARY_EXPRESSION (&&, AND);
  LANG_BINARY_EXPRESSION (||, OR);
  LANG_BINARY_EXPRESSION (==, EQUAL);

  AST operator ! (AST arg)
  {
    return std::make_unique<Operator> (
        Operator::NOT, std::move (arg), AST ());
  }

  /*
   * Part three: Checking & evaluating ASTs.
   */
  struct InvalidExpression : public std::monostate {};

  using Result = std::variant<InvalidExpression, int, bool>;

  inline std::ostream&
  operator << (std::ostream& os, Result const& result)
  {
    switch (result.index ())
    {
      case 0: { os << "invalid"; } break;
      case 1: { os << "(int) " << std::get<int> (result); } break;
      case 2: { os << "(bool) " << std::get<bool> (result); } break;
    }
    return os;
  }

  Result evaluate (AST const& ast)
  {
    if (auto const ptr = dynamic_cast<IntLiteral*> (ast.get ()))
    {
      return Result (std::in_place_type<int>, ptr->value);
    }
    else if (auto const ptr = dynamic_cast<BoolLiteral*> (ast.get ()))
    {
      return Result (std::in_place_type<bool>, ptr->value);
    }
    else if (auto const ptr = dynamic_cast<Operator*> (ast.get ()))
    {
      if (ptr->type == Operator::NOT)
      {
        Result const value = evaluate (ptr->arg_1);

        if (value.index () == 0)
        {
          return { InvalidExpression {} };
        }
        else if (value.index () == 1)
        {
          return { InvalidExpression {} };
        }
        else if (value.index () == 2)
        {
          bool const b_value = std::get<2> (value);
          return Result (!b_value);
        }
      }
      else
      {
        Result const lhs = evaluate (ptr->arg_1);
        Result const rhs = evaluate (ptr->arg_2);

        if (lhs.index () == 0 || rhs.index () == 0)
        {
          return { InvalidExpression {} };
        }

        if (lhs.index () != rhs.index ())
        {
          return { InvalidExpression {} };
        }

        if (lhs.index () == 1)
        {
          int const i_lhs = std::get<1> (lhs);
          int const i_rhs = std::get<1> (rhs);
          switch (ptr->type)
          {
            case Operator::ADDITION:        { return Result (i_lhs + i_rhs); } break;
            case Operator::SUBTRACTION:     { return Result (i_lhs - i_rhs); } break;
            case Operator::MULTIPLICATION:  { return Result (i_lhs * i_rhs); } break;
            case Operator::DIVISION:        { return Result (i_lhs / i_rhs); } break;

            case Operator::NOT: break;
            case Operator::AND:             { return InvalidExpression {}; } break;
            case Operator::OR:              { return InvalidExpression {}; } break;
            case Operator::EQUAL:           { return Result (i_lhs == i_rhs); } break;
          }
        }
        else if (lhs.index () == 2)
        {
          int const b_lhs = std::get<2> (lhs);
          int const b_rhs = std::get<2> (rhs);
          switch (ptr->type)
          {
            case Operator::ADDITION:        { return InvalidExpression {}; } break;
            case Operator::SUBTRACTION:     { return InvalidExpression {}; } break;
            case Operator::MULTIPLICATION:  { return InvalidExpression {}; } break;
            case Operator::DIVISION:        { return InvalidExpression {}; } break;

            case Operator::NOT: break;
            case Operator::AND:             { return Result (b_lhs && b_rhs); } break;
            case Operator::OR:              { return Result (b_lhs || b_rhs); } break;
            case Operator::EQUAL:           { return Result (b_lhs == b_rhs); } break;
          }
        }
      }
    }

    throw std::runtime_error ("Unrecognized ASTNode");
  }
}

#endif
