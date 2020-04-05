#include "lang.hpp"

void
test_all (
    std::string_view const expression,
    lang::AST const& ast_result,
    lang::AST const& ast_expected)
{
  size_t const fill_size = 35;
  size_t const n = expression.size ();
  std::string const fill (fill_size - std::min (n, fill_size), ' ');

  lang::Result const result = evaluate (ast_result);
  std::cout << "test: " << expression << fill << " --> " << result << '\n';

  lang::Result const expected = evaluate (ast_expected);
  if (result != expected)
  {
    throw std::runtime_error ("TEST FAILED!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
  }
}

int main ()
{
  #define TEST_EXPRESSION(expr, expected) { test_all (#expr, expr, expected); }

  using namespace lang::literals;

  lang::AST const invalid = (0_i * 0_b);

  TEST_EXPRESSION (  3_i,                                3_i );
  TEST_EXPRESSION (  10_i + 4_i,                        14_i );
  TEST_EXPRESSION (  10_i - 4_i,                         6_i );
  TEST_EXPRESSION (  10_i * 4_i,                        40_i );
  TEST_EXPRESSION (  10_i / 4_i,                         2_i );
  TEST_EXPRESSION (  (5_i + 2_i - 3_i + 6_i) * 5_i,     50_i );

  TEST_EXPRESSION (  1_b == 1_b,                        1_b );
  TEST_EXPRESSION (  1_b == 0_b,                        0_b );
  TEST_EXPRESSION (  1_b && 1_b,                        1_b );
  TEST_EXPRESSION (  1_b && 0_b,                        0_b );
  TEST_EXPRESSION (  0_b && 1_b,                        0_b );
  TEST_EXPRESSION (  0_b && 0_b,                        0_b );
  TEST_EXPRESSION (  1_b || 1_b,                        1_b );
  TEST_EXPRESSION (  1_b || 0_b,                        1_b );
  TEST_EXPRESSION (  0_b || 1_b,                        1_b );
  TEST_EXPRESSION (  0_b || 0_b,                        0_b );
  TEST_EXPRESSION (  !0_b,                              1_b );
  TEST_EXPRESSION (  !1_b,                              0_b );
  TEST_EXPRESSION (  (5_i + 2_i) * 2_i == 14_i,         1_b );
  TEST_EXPRESSION (  (5_i + 2_i) * 2_i == 15_i,         0_b );
  TEST_EXPRESSION (  !((5_i + 2_i) * 2_i == 15_i),      1_b );

  TEST_EXPRESSION (  1_i + 1_b,                         invalid );
  TEST_EXPRESSION (  1_b - 1_i,                         invalid );
  TEST_EXPRESSION (  1_b * 1_i,                         invalid );
  TEST_EXPRESSION (  1_i / 1_b,                         invalid );
  TEST_EXPRESSION (  1_i && 1_i,                        invalid );
  TEST_EXPRESSION (  1_i || 1_i,                        invalid );
  TEST_EXPRESSION (  1_i == 1_b,                        invalid );
  TEST_EXPRESSION (  !1_i,                              invalid );
  TEST_EXPRESSION (  (5_i + 2_i) * 2_i == (1_b && 0_b), invalid );

  std::cout << "\nAll tests passed!\n";
}
