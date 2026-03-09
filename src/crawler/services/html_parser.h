#pragma once

#include <concepts>
#include <string>
#include <vector>

#include "lexbor/html/parser.h"

namespace crawler {

namespace services {

namespace html {

class ParseResult {
 public:
  ParseResult() = default;
  std::vector<std::string>& outlinks() noexcept { return outlinks_; }

 private:
  std::vector<std::string> outlinks_;
};

template <typename T>
concept IHTMLParser = requires(T v, const std::string& url) {
  { v.parse(url) } -> std::convertible_to<ParseResult>;
};

template <IHTMLParser ParserType>
class HTMLParser {
 public:
  HTMLParser() = default;
  ParseResult parse(const std::string& url) { return parser_.parse(url); }

 private:
  ParserType parser_{};
};

class LexborParser {
 public:
  LexborParser();

  ParseResult parse(const std::string& htmlString);

  ~LexborParser();

 private:
  lxb_html_parser_t* parser_;
};

}  // namespace html

}  // namespace services

}  // namespace crawler
