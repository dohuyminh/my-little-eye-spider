#include "html_parser.h"

#include <queue>
#include <stdexcept>

#include "lexbor/dom/dom.h"

namespace crawler {

namespace services {

namespace html {

LexborParser::LexborParser() : parser_(lxb_html_parser_create()) {
  // verify if parser is initialized successfully
  if (lxb_html_parser_init(parser_) != LXB_STATUS_OK) {
    lxb_html_parser_destroy(parser_);
    throw std::runtime_error("Initializing Lexbor parser failed");
  }
}

ParseResult LexborParser::parse(const std::string& htmlString) {
  ParseResult result;

  // perform the parsing
  lxb_html_document_t* doc = lxb_html_parse(
      parser_, reinterpret_cast<const lxb_char_t*>(htmlString.c_str()),
      htmlString.size());

  if (doc == nullptr) {
    lxb_html_parser_clean(parser_);
    throw std::invalid_argument(
        "LexborParser: failed to parse the HTML string");
  }

  // iterate over tree to extract outlinks
  lxb_dom_element_t* body{lxb_dom_interface_element(doc->body)};
  lxb_dom_node_t* root{lxb_dom_interface_node(body)};

  // perform level-order traversal on tree to avoid stack overflow
  // which easily occurs compared to depth-first recursion-based approach
  std::queue<lxb_dom_node_t*> levelOrder;
  levelOrder.push(root);

  while (!levelOrder.empty()) {
    lxb_dom_node_t* curr{levelOrder.front()};
    levelOrder.pop();

    // if the current node is of type anchor <a>, extract outlink
    if (lxb_dom_node_tag_id(curr) == lxb_tag_id_enum_t::LXB_TAG_A) {
      lxb_dom_element_t* element{lxb_dom_interface_element(curr)};
      std::size_t outlinkLen;

      const lxb_char_t* outlinkRaw{lxb_dom_element_get_attribute(
          element, reinterpret_cast<const lxb_char_t*>("href"), 4,
          &outlinkLen)};

      if (outlinkRaw != nullptr) {
        std::string outlink;
        outlink.assign(reinterpret_cast<const char*>(outlinkRaw), outlinkLen);

        result.outlinks().push_back(outlink);
      }
    }

    lxb_dom_node_t* child{lxb_dom_node_first_child(curr)};
    while (child != nullptr) {
      levelOrder.push(child);
      child = lxb_dom_node_next(child);
    }
  }

  // destroy DOM tree
  lxb_html_document_destroy(doc);

  // clean parser for reuse
  lxb_html_parser_clean(parser_);

  // return result
  return result;
}

LexborParser::~LexborParser() { lxb_html_parser_destroy(parser_); }

}  // namespace html

}  // namespace services

}  // namespace crawler
