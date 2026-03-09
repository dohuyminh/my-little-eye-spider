#include <curl/curl.h>

#include <cstdlib>
#include <format>
#include <iostream>
#include <stdexcept>
#include <string>

#include "crawler/services/html_downloader.h"
#include "crawler/services/html_parser.h"
#include "crawler/types/url.h"

using namespace crawler::services::html;
using namespace crawler::services::curl;

int main() {
  HTMLParser<LexborParser> parser;
  std::string url;
  do {
    std::cout << "Insert a valid URL here:\n> ";

    bool valid{false};
    while (!valid) {
      std::getline(std::cin, url);
      try {
        if (url != "x") {
          crawler::types::URL{url};
        }
        valid = true;
      } catch (std::invalid_argument const&) {
        std::cerr << std::format(
            "\"{}\" is not a valid URL; please try again\n", url);
        std::cout << "> ";
      }
    }

    if (url == "x") {
      break;
    }

    ParseResult result;
    try {
      Response resp{Downloader()(url)};
      if (resp.code() != 200) {
        std::cerr << std::format("[HTTP] GET method returned code {}\n",
                                 resp.code());
        continue;
      }
      result = parser.parse(resp.content());
    } catch (std::runtime_error const& e) {
      std::cerr << std::format(
          "[HTTP] cannot perform GET request on URL \"{}\"\n", url);
      continue;
    }

    for (const auto& outlink : result.outlinks()) {
      std::cout << std::format("[Outlink] {}\n", outlink);
    }

  } while (url != "x");

  std::cout << "Exiting Program...\n";

  return 0;
}
