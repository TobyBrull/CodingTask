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

  void resize () ->(memory) void;
};

void play_with_my_int_vector() ->(memory) {
  my_int_vector vec;
  vec.resize(1000);
}

// How to handle alternate return-paths in assignment operator?
// How to handle x = make_an_x() if both "make_an_x" and x's assignment operator
// have alternative return-paths. In particular, the assignment operator
// might return memory errors.

class Borrower {
  int m_id;
  std::string m_name;

  void reset_aaa(int const id, char const* name) {
    m_id = id;
    m_name = name ->(memory);
  }

  void reset_bbb(int const id, std::string name) {
    m_id = id;
    m_name = std::move(name);
  }
};

// Constructor-level alternative return-paths

class FileReader {
    std::ifstream ifs;
  public:
    FileReader(std::string_view filename) ->(file_not_found);
};

FileReader open_file_aaa(std::string_view const filename) {
  FileReader retval(filename) ->(file_not_found) throw;
  return retval;
}

FileReader open_file_bbb(std::string_view const filename) {
  FileReader retval(filename) ->(file_not_found)
        { -> FileReader("backup.txt") ->(file_not_found) throw; };
  return retval;
}

FileReader open_file_ccc(std::string_view const filename)
    ->(file_not_found) void
{
  return FileReader(filename);
}

// Alternative return-paths could be introduced via
// the "->(<identifier>)" notation. An arbitrary number
// of alternative return-paths could be allowed, but "error"
// would probably be the one most often used.
int parse_int(std::string_view const str)
    ->(error) std::string
{
  int i;
  std::stringstream ss;
  ss << str;
  ss >> i;
  if (ss.fail()) {
    return (error) "parse_int: nope";
  }
  else {
    return i;
  }
}

// The primary return-path is the one without an
// identifier (i.e., just "->"), compatible with current syntax.
auto parse_int_square_aaa(std::string_view const str)
    -> std::tuple<int, int> //, (?)
    ->(error) std::string
{
  // Return values for the primary return-path can
  // be used as always. Returns values on alternative
  // return-paths can be used with the same
  // "->(<identifier>)" syntax, followed by a
  // variable declaration.
  auto const i = parse_int_sq(str)
    ->(error) auto msg {
    return (error) "parse_int_sq: " + std::move(msg);
    };
  return i, i * i;
}

// This function does mostly the same as the previous;
// only the error message is shorter (pass-through).
auto parse_int_square_bbb(std::string_view const str)
    -> std::tuple<int, int>
    ->(error) std::string
{
  // This wouldn't compile if this function would not
  // declare the "->(error)" return-path.
  // But since this function _does_ declare the "->(error)"
  // return-path, if this call returns via the error return-path
  // the whole function will return 
  auto const i = parse_int_sq(str);
  return i, i * i;
}

int main_aaa() {
  std::string input;
  std::cin >> input;

  // This would throw the object returned on the ->(error)
  // return-path as an exception. Necessary, because
  // this function does not declare the ->(error) return-path
  // itself.
  auto const [i, i_sq] = parse_int_sq_aaa(input) ->(error) throw;
//auto const [i, i_sq] = parse_int_sq_aaa(input) ->(error) { return(default) 1; };
//auto const [i, i_sq] = parse_int_sq_aaa(input) ->(error) { -> {0, 0}; };

  assert(i * i == i_sq);
  std::cout << "i_sq = " << i_sq << '\n';
  return 0;
}

int main_bbb() {
  std::string input;
  std::cin >> input;

  // The new syntax for accepting return values on
  // alternative return-paths can be used for the
  // primary return-path as well.
  parse_int_sq_bbb(input)
    -> auto const [i, i_sq] {
      assert(i * i == i_sq);
      std::cout << "i_sq = " << i_sq << '\n';
      return 0;
  }
    ->(error) auto const msg {
      std::cerr << "Error: " << msg << '\n';
      return 1;
    };
}

int main_ccc() {
  inspect (parse_int_sq_ccc(input)) {
    <0> [i, i_sq] : std::cout << "i_sq = " << i_sq << '\n';
    <std::error> err : std::cout << "Error: " << err << '\n';
  }
}
