/// @file file.hpp
#include <fstream>
#include <string>
#include <rich/exception.hpp>

namespace rich {
  // https://kagasu.hatenablog.com/entry/2017/05/01/215219
  std::string get_file_contents(const char* fname) {
    std::ifstream ifs(fname);
    if(!ifs) throw runtime_error("Failed to read file");
    std::string ret((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
    return ret;
  }

  std::string get_file_contents(const std::string& fname) {
    return get_file_contents(fname.c_str());
  }
}
