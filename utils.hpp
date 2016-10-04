#ifndef _UTILS_HPP_
#define _UTILS_HPP_

#include <iostream>
#include <fstream>
#include <sstream>

#include <algorithm>
#include <functional>
#include <string>

namespace io {
  using string = std::wstring;
  using char_t = string::value_type;

  using ifs = std::wifstream;
  using ofs = std::wofstream;

  using iss = std::wistringstream;
  using oss = std::wostringstream;

  using isi = std::istream_iterator <string, char_t>;
  using osi = std::ostream_iterator <string, char_t>;
}

namespace util {

  template <typename T>
  struct key_hash : public std::unary_function <T, std::size_t> {
    std::size_t operator() (const T& k) const {
      static auto fn = std::hash <io::string> {};
      io::oss oss;
      oss << k;
      return fn (oss.str());
    }
  };

  template <typename T>
  struct key_equal : public std::binary_function <T, T, bool> {
    bool operator() (const T& v0, const T& v1) const {
      static key_hash <T> hasher;
      return hasher (v0) == hasher (v1);
    }
  };

  template <typename WordKey,
            typename Container,
            size_t SIZE = std::tuple_size<WordKey>::value>
  WordKey createFromContainer (const Container & deque, int offset) {
    WordKey arr;
    std::copy (std::begin (deque) + offset,
               std::begin (deque) + offset + SIZE,
               std::begin (arr));
    return arr;
  }

  template <typename String, typename Function>
  String trim (const String & s, Function f) {
    auto front = std::find_if_not (s.begin(), s.end(), f);
    auto back = std::find_if_not (s.rbegin(), s.rend(), f).base();
    if (back > front)
      return { front, back };
    return { };
  }

  template <typename Iter, typename String>
  io::string print (Iter begin, Iter end, const String SPACE) {
    // create output stream
    io::oss oss;
    // copy all words separated by a space
    std::copy (begin, end, io::osi { oss, SPACE });
    io::string output { oss.str() };
    // remove trailing space (if exists)
    if (output.size() > 0)
      output.resize (output.size() - 1);
    return output;
  }
}

#endif
