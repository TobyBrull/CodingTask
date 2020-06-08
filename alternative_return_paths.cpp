// References:
// [1] http://lucteo.ro/2018/04/21/exception-exploration-2/
// [2] Phil Nash's CppCon 2019 talk,
//      "The Dawn of a New Error" ( https://www.youtube.com/watch?v=ZUH8p1EQswA )

// Semantics:
// * A function can declare an arbitrary number of alternative return-paths
//   via the "->[<identifier>]" notation. For each returns-path declaration,
//   a return-type may also be specified; if it is omitted it defaults to
//   "void".
// * At function call-site, each of the alternative return-paths must be
//   handled in exactly one of the following two ways:
//   - The caller can specify a return-path handler, also via the
//     "->[<identifier>]" notation.
//   - The function containing the call-site can itself contain the same
//     alternative return-path (with a return-type that can be constructed
//     from the type returned by the called function on the return-path). In
//     this case, the current function returns, and the alternative return
//     value is passed through.
// * _All_ function calls allow the following return-path handlers:
//   - just the keyword "throw"; this could mean that the value returned via
//     this return-path is thrown as an exception.
//   - A parameter declaration plus a function body. The function body must
//     either return, alternative-return, join, or throw an exception.
//     ~ An (alternative-)return statement here indicates that the _calling_
//       function returns (either via the default return path or the specified
//       alternative return-path.
//     ~ A join statement means that the return-path is joined back into the
//       default return-path. It might take the form
//          { ...; join <expression>; }
//       where "<expression>" is convertible to the default return-type.
// * Function calls to functions that return void or whose return-value
//   is ignored, could also have function bodies on return-path handlers
//   that don't do any of the above (i.e., neither return, join, nor throw).
// * A constructor could also declare an arbitrary number of return-paths, but
//   without a return-type.
// * Return-type handlers for object constructors are mostly similar to the
//   above. Not sure if a join statement should be allowed here?
// * One could also allow for return-path handlers to apply to compound
//   statements?
//      { parse(a); parse(b); } ->[parse_error] ...

// Open questions:
// * How to handle alternative return-paths in nested function calls?
// * How to handle alternate return-paths in copy-assignment?
// * How to handle x = make_an_x() if both "make_an_x" and x's assignment
//   operator have alternative return-paths. In particular, the
//   assignment operator might return bad_alloc errors.

// Alternative return-paths could be introduced via
// the "->[<identifier>] <type-id>" notation. An arbitrary
// number of alternative return-paths could be allowed.
int parse_positive_int(std::string_view const str)
    ->[parse_error] std::string
    ->[sign_error] void
{
  std::stringstream ss;
  ss << str;

  int i;
  ss >> i;

  if (ss.fail()) {
    return[parse_error] "parse_positive_int: not an int";
  }
  else {
    if (i > 0) {
      return i;
    }
    else {
      return[sign_error];
    }
  }
}

// The primary return-path is the one without an
// identifier (i.e., just "->"), compatible with current syntax.
auto parse_int_square_1(std::string_view const str)
    -> std::tuple<int, int>
    ->[error] std::string
{
  // Return values for the primary return-path can
  // be used as always. Returns values on alternative
  // return-paths can be used with the same
  // "->[<identifier>]" syntax, followed by a
  // parameter declaration.
  auto const i = parse_positive_int(str)
    ->[parse_error] auto msg {
      return[error] "parse_int_square_1: " + std::move(msg);
    }
    ->[sign_error] {
      return[error] "parse_int_square_1: wrong sign";
    };

  return i, i * i;
}

// This function does mostly the same as the previous.
// only the error message is shorter (pass-through).
auto parse_int_square_2(std::string_view const str)
    -> std::tuple<int, int>
    ->[parse_error] std::string
{
  // This wouldn't compile if parse_int_square_2 would not
  // declare the "->[parse_error]" return-path itself.
  auto const i = parse_int_sq(str)
    ->[sign_error] { return[parse_error] "wrong sign"; };

  return i, i * i;
}

int main_1() {
  std::string input;
  std::cin >> input;

  auto const [i, i_sq] = parse_int_sq_2(input) ->[parse_error] throw;

  auto const [j, j_sq] = parse_int_sq_2(input)
    ->[parse_error] auto const& msg {
      std::cerr << "Error: " << msg << '\n';
      return 1;
    };

  auto const [k, k_sq] = parse_int_sq_2(input)
    ->[parse_error] {
      join {0, 0};
    };

  assert(i * i == i_sq);
  assert(j * j == j_sq);
  assert(k * k == k_sq);
  std::cout << i_sq << ' ' << j_sq << ' ' << k_sq << '\n';
  return 0;
}

int main_2() {
  std::string input;
  std::cin >> input;

  // The new syntax for accepting return values on
  // alternative return-paths could be used for the
  // primary return-path as well.
  parse_int_sq_2(input)
    -> auto const [i, i_sq] {
      assert(i * i == i_sq);
      std::cout << "i_sq = " << i_sq << '\n';
      return 0;
    }
    ->[parse_error] auto const& msg {
      std::cerr << "Error: " << msg << '\n';
      return 1;
    };
}

// Memory allocation
class my_int_vector {
    int* m_data = nullptr;
    int m_size = 0;

  public:
    my_int_vector() = default;
    ~my_int_vector() {
      delete[] m_data;
    }

    my_int_vector(my_int_vector const&) = delete;
    my_int_vector& operator=(my_int_vector const&) = delete;

    void resize (int size) ->[bad_alloc] void;
};

void play_with_my_int_vector() ->[bad_alloc] {
  my_int_vector vec;
  vec.resize(1000);
}

// Constructor-level alternative return-paths
class FileReader {
    std::ifstream ifs;
  public:
    FileReader(std::string_view filename) ->[file_not_found];
};

FileReader open_file_1(std::string_view const filename) {
  FileReader retval(filename) ->[file_not_found] throw;
  return retval;
}

FileReader open_file_2(std::string_view const filename) {
  FileReader retval(filename) ->[file_not_found] {
    join FileReader("backup.txt") ->[file_not_found] throw;
  };
  return retval;
}

FileReader open_file_3(std::string_view const filename)
    ->[file_not_found]
{
  return FileReader(filename);
}
