#include <gtest/gtest.h>

#include <fstream>
#include <sstream>
#include <string>
#include <string_view>

#include "crawler/services/url.h"
#include "crawler/types/url.h"

namespace crawler::services::url {

namespace {

// ============================================================================
// percentEncode tests
// ============================================================================

TEST(PercentEncodeTest, EmptyString) {
  std::string input{""};
  std::string result{percentEncode(input)};
  EXPECT_TRUE(result.empty());
}

TEST(PercentEncodeTest, UnreservedChars) {
  std::string input{"abcdefghijklmnopqrstuvwxyz0123456789-_.~"};
  std::string result{percentEncode(input)};
  EXPECT_EQ(input, result);
}

TEST(PercentEncodeTest, ReservedChars) {
  std::string input{"hello world"};
  std::string result{percentEncode(input)};
  EXPECT_EQ(result, "hello%20world");
}

TEST(PercentEncodeTest, SlashIgnored) {
  std::string input{"path/to/file"};
  std::string result{percentEncode(input, '/')};
  EXPECT_EQ(result, "path/to/file");
}

// ============================================================================
// percentDecode tests
// ============================================================================

TEST(PercentDecodeTest, EmptyString) {
  std::string input{""};
  std::string result{percentDecode(input)};
  EXPECT_TRUE(result.empty());
}

TEST(PercentDecodeTest, NoEncoding) {
  std::string input{"hello"};
  std::string result{percentDecode(input)};
  EXPECT_EQ(result, "hello");
}

TEST(PercentDecodeTest, SingleEncoded) {
  std::string input{"%20"};
  std::string result{percentDecode(input)};
  EXPECT_EQ(result, " ");
}

TEST(PercentDecodeTest, MultipleEncoded) {
  std::string input{"%20%21%22"};
  std::string result{percentDecode(input)};
  EXPECT_EQ(result, " !\"");
}

TEST(PercentDecodeTest, MixedContent) {
  std::string input{"hello%20world"};
  std::string result{percentDecode(input)};
  EXPECT_EQ(result, "hello world");
}

// ============================================================================
// isPercentEncoded tests
// ============================================================================

TEST(IsPercentEncodedTest, NotEncoded) {
  std::string input{"hello"};
  EXPECT_FALSE(isPercentEncoded(input));
}

TEST(IsPercentEncodedTest, IsEncoded) {
  std::string input{"%20"};
  EXPECT_TRUE(isPercentEncoded(input));
}

// ============================================================================
// isRelativeURL tests
// ============================================================================

TEST(IsRelativeURLTest, EmptyString) {
  std::string_view input{""};
  EXPECT_TRUE(isRelativeURL(input));
}

TEST(IsRelativeURLTest, PathOnly) {
  std::string_view input{"/path/to/page"};
  EXPECT_TRUE(isRelativeURL(input));
}

TEST(IsRelativeURLTest, RelativeWithDotDot) {
  std::string_view input{"../parent"};
  EXPECT_TRUE(isRelativeURL(input));
}

TEST(IsRelativeURLTest, RelativeWithDot) {
  std::string_view input{"./current"};
  EXPECT_TRUE(isRelativeURL(input));
}

TEST(IsRelativeURLTest, SimpleFilename) {
  std::string_view input{"page.html"};
  EXPECT_TRUE(isRelativeURL(input));
}

TEST(IsRelativeURLTest, AbsoluteHttpUrl) {
  std::string_view input{"http://example.com"};
  EXPECT_FALSE(isRelativeURL(input));
}

TEST(IsRelativeURLTest, AbsoluteHttpsUrl) {
  std::string_view input{"https://example.com"};
  EXPECT_FALSE(isRelativeURL(input));
}

TEST(IsRelativeURLTest, AbsoluteFtpUrl) {
  std::string_view input{"ftp://ftp.example.com"};
  EXPECT_FALSE(isRelativeURL(input));
}

TEST(IsRelativeURLTest, ProtocolRelativeUrl) {
  std::string_view input{"//example.com"};
  EXPECT_TRUE(isRelativeURL(input));
}

TEST(IsRelativeURLTest, InvalidSchemeStart) {
  std::string_view input{"1http://example.com"};
  EXPECT_FALSE(isRelativeURL(input));  // Contains ':' which is not allowed in path-noscheme first segment
}

TEST(IsRelativeURLTest, ColonNoSlashSlash) {
  std::string_view input{"foo:bar"};
  EXPECT_FALSE(isRelativeURL(input));  // Colon not allowed in segment-nz-nc per RFC 3986
}

TEST(IsRelativeURLTest, PathWithQuery) {
  std::string_view input{"/path/to/page?key=value"};
  EXPECT_TRUE(isRelativeURL(input));
}

TEST(IsRelativeURLTest, PathWithFragment) {
  std::string_view input{"/path/to/page#section"};
  EXPECT_TRUE(isRelativeURL(input));
}

TEST(IsRelativeURLTest, PathWithQueryAndFragment) {
  std::string_view input{"/path/to/page?key=value&other=123#section"};
  EXPECT_TRUE(isRelativeURL(input));
}

TEST(IsRelativeURLTest, QueryOnly) {
  std::string_view input{"?key=value"};
  EXPECT_TRUE(isRelativeURL(input));
}

TEST(IsRelativeURLTest, QueryAndFragment) {
  std::string_view input{"?key=value#section"};
  EXPECT_TRUE(isRelativeURL(input));
}

TEST(IsRelativeURLTest, FragmentOnly) {
  std::string_view input{"#section"};
  EXPECT_TRUE(isRelativeURL(input));
}

TEST(IsRelativeURLTest, RelativePathWithQuery) {
  std::string_view input{"../parent?id=42"};
  EXPECT_TRUE(isRelativeURL(input));
}

TEST(IsRelativeURLTest, RelativePathWithFragment) {
  std::string_view input{"./current#top"};
  EXPECT_TRUE(isRelativeURL(input));
}

TEST(IsRelativeURLTest, RelativePathWithQueryAndFragment) {
  std::string_view input{"../parent/page?key=val#section"};
  EXPECT_TRUE(isRelativeURL(input));
}

TEST(IsRelativeURLTest, QueryWithSpecialChars) {
  std::string_view input{"/search?q=hello%20world&lang=en"};
  EXPECT_TRUE(isRelativeURL(input));
}

TEST(IsRelativeURLTest, FragmentWithSpecialChars) {
  std::string_view input{"/page#section-1.2"};
  EXPECT_TRUE(isRelativeURL(input));
}

// ============================================================================
// parse tests
// ============================================================================

TEST(ParseTest, ValidHttpUrl) {
  std::string url{"http://example.com"};
  ParseResult<URLParseResult> result{parse(url)};
  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(result->scheme, "http");
  EXPECT_EQ(result->domain, "example.com");
}

TEST(ParseTest, ValidHttpsUrlWithPath) {
  std::string url{"https://example.com/path/to/page"};
  ParseResult<URLParseResult> result{parse(url)};
  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(result->scheme, "https");
  EXPECT_EQ(result->domain, "example.com");
  EXPECT_FALSE(result->path.empty());
}

TEST(ParseTest, UrlWithQueryParams) {
  std::string url{"http://example.com/search?q=test&lang=en"};
  ParseResult<URLParseResult> result{parse(url)};
  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(result->queryParams["q"], "test");
  EXPECT_EQ(result->queryParams["lang"], "en");
}

TEST(ParseTest, UrlWithFragment) {
  std::string url{"http://example.com/page#section"};
  ParseResult<URLParseResult> result{parse(url)};
  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(result->fragment, "section");
}

TEST(ParseTest, UrlWithSubdomains) {
  std::string url{"http://www.blog.example.com"};
  ParseResult<URLParseResult> result{parse(url)};
  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(result->domain, "example.com");
  EXPECT_FALSE(result->subdomains.empty());
}

TEST(ParseTest, InvalidUrl) {
  std::string url{"://invalid"};
  ParseResult<URLParseResult> result{parse(url)};
  EXPECT_FALSE(result.has_value());
}

TEST(ParseTest, MissingAuthority) {
  std::string url{"http://"};
  ParseResult<URLParseResult> result{parse(url)};
  EXPECT_FALSE(result.has_value());
  EXPECT_EQ(result.error(), ParseError::INVALID_URL);
}

// ============================================================================
// parseAndApplyRelativeURL tests
// ============================================================================

TEST(parseAndApplyRelativeURLTest, EmptyRelativeUrl) {
  std::vector<std::string> path{"/", "current/", "path/"};
  std::unordered_map<std::string, std::string> queryParams;
  std::string fragment;
  std::string relative{""};
  bool result{parseAndApplyRelativeURL(relative, path, queryParams, fragment)};
  EXPECT_TRUE(result);
  EXPECT_EQ(path.size(), 3);  // Unchanged
}

TEST(parseAndApplyRelativeURLTest, AbsolutePathFromRoot) {
  std::vector<std::string> path{"/", "old/", "dir/"};
  std::unordered_map<std::string, std::string> queryParams;
  std::string fragment;
  std::string relative{"/new/path"};
  bool result{parseAndApplyRelativeURL(relative, path, queryParams, fragment)};
  EXPECT_TRUE(result);
  EXPECT_EQ(path.size(), 3);
  EXPECT_EQ(path[0], "/");
  EXPECT_EQ(path[1], "new/");  // Segment with trailing slash
  EXPECT_EQ(path[2], "path");  // Last segment without trailing slash
}

TEST(parseAndApplyRelativeURLTest, ParentDirectory) {
  std::vector<std::string> path{"/", "current/", "dir"};
  std::unordered_map<std::string, std::string> queryParams;
  std::string fragment;
  std::string relative{"../parent"};
  bool result{parseAndApplyRelativeURL(relative, path, queryParams, fragment)};
  EXPECT_TRUE(result);
  // "../parent" pops "dir", then adds "parent"
  EXPECT_EQ(path.size(), 3);
  EXPECT_EQ(path[0], "/");
  EXPECT_EQ(path[1], "current/");
  EXPECT_EQ(path[2], "parent");
}

TEST(parseAndApplyRelativeURLTest, CurrentDirectory) {
  std::vector<std::string> path{"/", "current"};
  std::unordered_map<std::string, std::string> queryParams;
  std::string fragment;
  std::string relative{"./same"};
  bool result{parseAndApplyRelativeURL(relative, path, queryParams, fragment)};
  EXPECT_TRUE(result);
  // "./same" adds "same" at the same level as current
  EXPECT_EQ(path.size(), 3);
  EXPECT_EQ(path[0], "/");
  EXPECT_EQ(path[1], "current/");
  EXPECT_EQ(path[2], "same");
}

TEST(parseAndApplyRelativeURLTest, CannotGoAboveRoot) {
  std::vector<std::string> path{"/"};
  std::unordered_map<std::string, std::string> queryParams;
  std::string fragment;
  std::string relative{"../above"};
  bool result{parseAndApplyRelativeURL(relative, path, queryParams, fragment)};
  EXPECT_FALSE(result);
}

TEST(parseAndApplyRelativeURLTest, RejectsAbsoluteUrl) {
  std::vector<std::string> path{"/", "current"};
  std::unordered_map<std::string, std::string> queryParams;
  std::string fragment;
  std::string relative{"http://example.com"};
  bool result{parseAndApplyRelativeURL(relative, path, queryParams, fragment)};
  EXPECT_FALSE(result);  // isRelativeURL returns false for absolute URLs
}

TEST(parseAndApplyRelativeURLTest, PathWithQuery) {
  std::vector<std::string> path{"/", "old/"};
  std::unordered_map<std::string, std::string> queryParams{{"old", "param"}};
  std::string fragment{"oldFrag"};
  std::string relative{"/new/path?key=value&count=42"};
  bool result{parseAndApplyRelativeURL(relative, path, queryParams, fragment)};
  EXPECT_TRUE(result);
  EXPECT_EQ(path.size(), 3);
  EXPECT_EQ(path[0], "/");
  EXPECT_EQ(path[1], "new/");
  EXPECT_EQ(path[2], "path");
  EXPECT_EQ(queryParams.size(), 2);
  EXPECT_EQ(queryParams["key"], "value");
  EXPECT_EQ(queryParams["count"], "42");
  EXPECT_TRUE(fragment.empty());  // Fragment cleared since not present in relative
}

TEST(parseAndApplyRelativeURLTest, PathWithFragment) {
  std::vector<std::string> path{"/", "old/"};
  std::unordered_map<std::string, std::string> queryParams;
  std::string fragment;
  std::string relative{"/page#section1"};
  bool result{parseAndApplyRelativeURL(relative, path, queryParams, fragment)};
  EXPECT_TRUE(result);
  EXPECT_EQ(path.size(), 2);
  EXPECT_EQ(path[0], "/");
  EXPECT_EQ(path[1], "page");
  EXPECT_TRUE(queryParams.empty());
  EXPECT_EQ(fragment, "section1");
}

TEST(parseAndApplyRelativeURLTest, PathWithQueryAndFragment) {
  std::vector<std::string> path{"/", "old/"};
  std::unordered_map<std::string, std::string> queryParams{{"old", "value"}};
  std::string fragment{"oldSection"};
  std::string relative{"/new/page?q=search&id=123#results"};
  bool result{parseAndApplyRelativeURL(relative, path, queryParams, fragment)};
  EXPECT_TRUE(result);
  EXPECT_EQ(path.size(), 3);
  EXPECT_EQ(path[0], "/");
  EXPECT_EQ(path[1], "new/");
  EXPECT_EQ(path[2], "page");
  EXPECT_EQ(queryParams.size(), 2);
  EXPECT_EQ(queryParams["q"], "search");
  EXPECT_EQ(queryParams["id"], "123");
  EXPECT_EQ(fragment, "results");
}

TEST(parseAndApplyRelativeURLTest, QueryOnly) {
  std::vector<std::string> path{"/", "page"};
  std::unordered_map<std::string, std::string> queryParams;
  std::string fragment;
  std::string relative{"?key=newvalue&lang=en"};
  bool result{parseAndApplyRelativeURL(relative, path, queryParams, fragment)};
  EXPECT_TRUE(result);
  EXPECT_EQ(path.size(), 2);
  EXPECT_EQ(path[0], "/");
  EXPECT_EQ(path[1], "page");
  EXPECT_EQ(queryParams.size(), 2);
  EXPECT_EQ(queryParams["key"], "newvalue");
  EXPECT_EQ(queryParams["lang"], "en");
  EXPECT_TRUE(fragment.empty());
}

TEST(parseAndApplyRelativeURLTest, QueryAndFragment) {
  std::vector<std::string> path{"/", "page"};
  std::unordered_map<std::string, std::string> queryParams;
  std::string fragment;
  std::string relative{"?search=test#findings"};
  bool result{parseAndApplyRelativeURL(relative, path, queryParams, fragment)};
  EXPECT_TRUE(result);
  EXPECT_EQ(path.size(), 2);
  EXPECT_EQ(path[0], "/");
  EXPECT_EQ(path[1], "page");
  EXPECT_EQ(queryParams.size(), 1);
  EXPECT_EQ(queryParams["search"], "test");
  EXPECT_EQ(fragment, "findings");
}

TEST(parseAndApplyRelativeURLTest, FragmentOnly) {
  std::vector<std::string> path{"/", "page/"};
  std::unordered_map<std::string, std::string> queryParams{{"key", "value"}};
  std::string fragment;
  std::string relative{"#newSection"};
  bool result{parseAndApplyRelativeURL(relative, path, queryParams, fragment)};
  EXPECT_TRUE(result);
  EXPECT_EQ(path.size(), 2);
  EXPECT_EQ(path[0], "/");
  EXPECT_EQ(path[1], "page/");
  EXPECT_TRUE(queryParams.empty());  // Query cleared since not present in relative
  EXPECT_EQ(fragment, "newSection");
}

TEST(parseAndApplyRelativeURLTest, RelativePathWithQuery) {
  std::vector<std::string> path{"/", "dir/", "old"};
  std::unordered_map<std::string, std::string> queryParams;
  std::string fragment;
  std::string relative{"../new?action=update&id=99"};
  bool result{parseAndApplyRelativeURL(relative, path, queryParams, fragment)};
  EXPECT_TRUE(result);
  EXPECT_EQ(path.size(), 3);
  EXPECT_EQ(path[0], "/");
  EXPECT_EQ(path[1], "dir/");
  EXPECT_EQ(path[2], "new");
  EXPECT_EQ(queryParams.size(), 2);
  EXPECT_EQ(queryParams["action"], "update");
  EXPECT_EQ(queryParams["id"], "99");
}

TEST(parseAndApplyRelativeURLTest, RelativePathWithFragment) {
  std::vector<std::string> path{"/", "docs/"};
  std::unordered_map<std::string, std::string> queryParams;
  std::string fragment;
  std::string relative{"./intro#beginning"};
  bool result{parseAndApplyRelativeURL(relative, path, queryParams, fragment)};
  EXPECT_TRUE(result);
  EXPECT_EQ(path.size(), 3);
  EXPECT_EQ(path[0], "/");
  EXPECT_EQ(path[1], "docs/");
  EXPECT_EQ(path[2], "intro");
  EXPECT_TRUE(queryParams.empty());
  EXPECT_EQ(fragment, "beginning");
}

TEST(parseAndApplyRelativeURLTest, RelativePathWithQueryAndFragment) {
  std::vector<std::string> path{"/", "content/", "post"};
  std::unordered_map<std::string, std::string> queryParams;
  std::string fragment;
  std::string relative{"../../home?page=1&sort=date#top"};
  bool result{parseAndApplyRelativeURL(relative, path, queryParams, fragment)};
  EXPECT_TRUE(result);
  EXPECT_EQ(path.size(), 2);
  EXPECT_EQ(path[0], "/");
  EXPECT_EQ(path[1], "home");
  EXPECT_EQ(queryParams.size(), 2);
  EXPECT_EQ(queryParams["page"], "1");
  EXPECT_EQ(queryParams["sort"], "date");
  EXPECT_EQ(fragment, "top");
}

TEST(parseAndApplyRelativeURLTest, QueryReplacesExisting) {
  std::vector<std::string> path{"/", "search"};
  std::unordered_map<std::string, std::string> queryParams{
      {"old", "param"}, {"another", "value"}};
  std::string fragment;
  std::string relative{"?q=newquery&filter=active"};
  bool result{parseAndApplyRelativeURL(relative, path, queryParams, fragment)};
  EXPECT_TRUE(result);
  EXPECT_EQ(queryParams.size(), 2);
  EXPECT_EQ(queryParams["q"], "newquery");
  EXPECT_EQ(queryParams["filter"], "active");
  EXPECT_FALSE(queryParams.count("old"));  // Old params replaced
  EXPECT_FALSE(queryParams.count("another"));
}

TEST(parseAndApplyRelativeURLTest, FragmentReplacesExisting) {
  std::vector<std::string> path{"/", "page"};
  std::unordered_map<std::string, std::string> queryParams;
  std::string fragment{"oldSection"};
  std::string relative{"#newSection"};
  bool result{parseAndApplyRelativeURL(relative, path, queryParams, fragment)};
  EXPECT_TRUE(result);
  EXPECT_EQ(fragment, "newSection");
}

TEST(parseAndApplyRelativeURLTest, ComplexRelativeWithDotDotQueryFragment) {
  std::vector<std::string> path{"/", "app/", "users/", "profile"};
  std::unordered_map<std::string, std::string> queryParams;
  std::string fragment;
  std::string relative{"../../settings?theme=dark&lang=en-US#privacy"};
  bool result{parseAndApplyRelativeURL(relative, path, queryParams, fragment)};
  EXPECT_TRUE(result);
  EXPECT_EQ(path.size(), 3);
  EXPECT_EQ(path[0], "/");
  EXPECT_EQ(path[1], "app/");
  EXPECT_EQ(path[2], "settings");
  EXPECT_EQ(queryParams.size(), 2);
  EXPECT_EQ(queryParams["theme"], "dark");
  EXPECT_EQ(queryParams["lang"], "en-US");
  EXPECT_EQ(fragment, "privacy");
}

// ============================================================================
// parseRobotsTxt tests
// ============================================================================

std::string loadRobotsTxtFile(const std::string& filename) {
  std::ifstream file{"/home/hminhdo/Desktop/coding-stuffs/my-little-eye-spider/tests/services/robots_txt_cases/" + filename};
  EXPECT_TRUE(file.good()) << "Failed to open file: " << filename;
  std::stringstream buffer;
  buffer << file.rdbuf();
  return buffer.str();
}

TEST(ParseRobotsTxtTest, NormalBasicRobotsTxt) {
  std::string content{loadRobotsTxtFile("normal_basic_robots.txt")};
  ParseResult<RobotsTxtRepr> result{parseRobotsTxt(content)};
  ASSERT_TRUE(result.has_value());

  EXPECT_TRUE(result->contains("allow"));
  EXPECT_TRUE(result->contains("disallow"));
  EXPECT_TRUE(result->contains("crawl-delay"));

  EXPECT_EQ(result->at("allow").size(), 2);
  EXPECT_EQ(result->at("disallow").size(), 2);
  EXPECT_EQ(result->at("crawl-delay").get<double>(), 10.0);

  bool foundAdmin{false}, foundPrivate{false};
  for (const auto& d : result->at("disallow")) {
    if (d.get<std::string>() == "/admin/") foundAdmin = true;
    if (d.get<std::string>() == "/private/") foundPrivate = true;
  }
  EXPECT_TRUE(foundAdmin);
  EXPECT_TRUE(foundPrivate);
}

TEST(ParseRobotsTxtTest, MultipleAgents) {
  std::string content{loadRobotsTxtFile("multiple_agents.txt")};
  ParseResult<RobotsTxtRepr> result{parseRobotsTxt(content)};
  ASSERT_TRUE(result.has_value());

  // Should only contain directives from User-Agent: * section
  EXPECT_EQ(result->at("allow").size(), 1);
  EXPECT_EQ(result->at("disallow").size(), 1);
  EXPECT_EQ(result->at("crawl-delay").get<double>(), 2.0);

  EXPECT_EQ(result->at("allow")[0].get<std::string>(), "/allowed/");
  EXPECT_EQ(result->at("disallow")[0].get<std::string>(), "/blocked/");
}

TEST(ParseRobotsTxtTest, TrailingComments) {
  std::string content{loadRobotsTxtFile("trailing_comments.txt")};
  ParseResult<RobotsTxtRepr> result{parseRobotsTxt(content)};
  ASSERT_TRUE(result.has_value());

  // Trailing comments should be stripped
  EXPECT_EQ(result->at("allow").size(), 1);
  EXPECT_EQ(result->at("disallow").size(), 1);
  EXPECT_EQ(result->at("crawl-delay").get<double>(), 5.0);

  EXPECT_EQ(result->at("allow")[0].get<std::string>(), "/public/");
  EXPECT_EQ(result->at("disallow")[0].get<std::string>(), "/admin/");
}

TEST(ParseRobotsTxtTest, EmptyRobotsTxt) {
  std::string content{loadRobotsTxtFile("empty_robots.txt")};
  ParseResult<RobotsTxtRepr> result{parseRobotsTxt(content)};
  ASSERT_TRUE(result.has_value());

  // Empty result - no directives found
  EXPECT_TRUE(result->contains("allow"));
  EXPECT_TRUE(result->contains("disallow"));
  EXPECT_EQ(result->at("allow").size(), 0);
  EXPECT_EQ(result->at("disallow").size(), 0);
  EXPECT_FALSE(result->contains("crawl-delay"));
}

TEST(ParseRobotsTxtTest, NoCrawlDelay) {
  std::string content{loadRobotsTxtFile("no_crawl_delay.txt")};
  ParseResult<RobotsTxtRepr> result{parseRobotsTxt(content)};
  ASSERT_TRUE(result.has_value());

  EXPECT_EQ(result->at("allow").size(), 1);
  EXPECT_EQ(result->at("disallow").size(), 1);
  EXPECT_FALSE(result->contains("crawl-delay"));

  EXPECT_EQ(result->at("allow")[0].get<std::string>(), "/");
  EXPECT_EQ(result->at("disallow")[0].get<std::string>(), "/tmp/");
}

TEST(ParseRobotsTxtTest, WhitespaceHandling) {
  std::string content{loadRobotsTxtFile("whitespace_handling.txt")};
  ParseResult<RobotsTxtRepr> result{parseRobotsTxt(content)};
  ASSERT_TRUE(result.has_value());

  EXPECT_EQ(result->at("allow").size(), 1);
  EXPECT_EQ(result->at("disallow").size(), 1);
  EXPECT_EQ(result->at("crawl-delay").get<double>(), 15.0);

  EXPECT_EQ(result->at("allow")[0].get<std::string>(), "/allowed/");
  EXPECT_EQ(result->at("disallow")[0].get<std::string>(), "/blocked/");
}

// ============================================================================
// urlIsDisallowed tests
// ============================================================================

TEST(UriIsDisallowedTest, EmptyPattern) {
  std::string url{"http://example.com/path"};
  std::string pattern{""};
  EXPECT_FALSE(urlIsDisallowed(url, pattern));
}

TEST(UriIsDisallowedTest, RootPatternDisallowsAll) {
  std::string url{"http://example.com/any/path"};
  std::string pattern{"/"};
  EXPECT_TRUE(urlIsDisallowed(url, pattern));
}

TEST(UriIsDisallowedTest, ExactPrefixMatch) {
  std::string url{"http://example.com/admin/secret"};
  std::string pattern{"/admin/"};
  EXPECT_TRUE(urlIsDisallowed(url, pattern));
}

TEST(UriIsDisallowedTest, NoMatchDifferentPrefix) {
  std::string url{"http://example.com/public/page"};
  std::string pattern{"/admin/"};
  EXPECT_FALSE(urlIsDisallowed(url, pattern));
}

TEST(UriIsDisallowedTest, PartialPathMatch) {
  std::string url{"http://example.com/api"};
  std::string pattern{"/api"};
  EXPECT_TRUE(urlIsDisallowed(url, pattern));
}

TEST(UriIsDisallowedTest, PatternLongerThanPath) {
  std::string url{"http://example.com/ap"};
  std::string pattern{"/api"};
  EXPECT_FALSE(urlIsDisallowed(url, pattern));
}

TEST(UriIsDisallowedTest, RootPathOnly) {
  std::string url{"http://example.com/"};
  std::string pattern{"/"};
  EXPECT_TRUE(urlIsDisallowed(url, pattern));
}

TEST(UriIsDisallowedTest, RootPathWithNoTrailingSlash) {
  std::string url{"http://example.com"};
  std::string pattern{"/"};
  EXPECT_FALSE(urlIsDisallowed(url, pattern));
}

TEST(UriIsDisallowedTest, WithQueryParams) {
  std::string url{"http://example.com/admin?token=abc"};
  std::string pattern{"/admin"};
  EXPECT_TRUE(urlIsDisallowed(url, pattern));
}

TEST(UriIsDisallowedTest, WithFragment) {
  std::string url{"http://example.com/private#section"};
  std::string pattern{"/private"};
  EXPECT_TRUE(urlIsDisallowed(url, pattern));
}

TEST(UriIsDisallowedTest, HttpsUrl) {
  std::string url{"https://example.com/restricted/content"};
  std::string pattern{"/restricted/"};
  EXPECT_TRUE(urlIsDisallowed(url, pattern));
}

TEST(UriIsDisallowedTest, UrlWithPort) {
  std::string url{"http://example.com:8080/api/internal"};
  std::string pattern{"/api/"};
  EXPECT_TRUE(urlIsDisallowed(url, pattern));
}

TEST(UriIsDisallowedTest, SubdomainUrl) {
  std::string url{"http://blog.example.com/private/post"};
  std::string pattern{"/private/"};
  EXPECT_TRUE(urlIsDisallowed(url, pattern));
}

TEST(UriIsDisallowedTest, CaseSensitiveMatch) {
  std::string url{"http://example.com/Admin/page"};
  std::string pattern{"/admin/"};
  EXPECT_FALSE(urlIsDisallowed(url, pattern));
}

TEST(UriIsDisallowedTest, DeepPathMatch) {
  std::string url{"http://example.com/a/b/c/d/e"};
  std::string pattern{"/a/b/"};
  EXPECT_TRUE(urlIsDisallowed(url, pattern));
}

TEST(UriIsDisallowedTest, FilenamePattern) {
  std::string url{"http://example.com/path/config.json"};
  std::string pattern{"/path/config"};
  EXPECT_TRUE(urlIsDisallowed(url, pattern));
}

// ============================================================================
// urlIsDisallowed wildcard tests (* and $)
// ============================================================================

TEST(UriIsDisallowedWildcardTest, AsteriskMatchesAnySuffix) {
  std::string url{"http://example.com/atlanta-101"};
  std::string pattern{"/atlanta*"};
  EXPECT_TRUE(urlIsDisallowed(url, pattern));
}

TEST(UriIsDisallowedWildcardTest, AsteriskMatchesMultipleSuffixVariations) {
  std::string url1{"http://example.com/atlanta-101"};
  std::string url2{"http://example.com/atlanta-102"};
  std::string url3{"http://example.com/atlanta"};
  std::string url4{"http://example.com/atlanta/foo"};
  std::string pattern{"/atlanta*"};
  EXPECT_TRUE(urlIsDisallowed(url1, pattern));
  EXPECT_TRUE(urlIsDisallowed(url2, pattern));
  EXPECT_TRUE(urlIsDisallowed(url3, pattern));
  EXPECT_TRUE(urlIsDisallowed(url4, pattern));
}

TEST(UriIsDisallowedWildcardTest, AsteriskInMiddleMatchesRest) {
  std::string url{"http://example.com/path/file.html"};
  std::string pattern{"/path/*.html"};
  EXPECT_TRUE(urlIsDisallowed(url, pattern));
}

TEST(UriIsDisallowedWildcardTest, AsteriskMatchesEmptyString) {
  std::string url{"http://example.com/path/file.html"};
  std::string pattern{"/path/file*"};
  EXPECT_TRUE(urlIsDisallowed(url, pattern));
}

TEST(UriIsDisallowedWildcardTest, AsteriskWithExtensionNoMatch) {
  std::string url{"http://example.com/path/file.txt"};
  std::string pattern{"/path/*.html"};
  EXPECT_FALSE(urlIsDisallowed(url, pattern));
}

TEST(UriIsDisallowedWildcardTest, AsteriskAtStart) {
  std::string url{"http://example.com/path/file.html"};
  std::string pattern{"*.html"};
  EXPECT_TRUE(urlIsDisallowed(url, pattern));
}

TEST(UriIsDisallowedWildcardTest, MultipleAsterisks) {
  std::string url{"http://example.com/a/b/c/d"};
  std::string pattern{"/a/*/c/*"};
  EXPECT_TRUE(urlIsDisallowed(url, pattern));
}

TEST(UriIsDisallowedWildcardTest, MultipleAsterisksNoMatch) {
  std::string url{"http://example.com/a/x/c/y"};
  std::string pattern{"/a/*/d/*"};
  EXPECT_FALSE(urlIsDisallowed(url, pattern));
}

TEST(UriIsDisallowedWildcardTest, DollarSignEndAnchorMatch) {
  std::string url{"http://example.com/path/file"};
  std::string pattern{"/path/file$"};
  EXPECT_TRUE(urlIsDisallowed(url, pattern));
}

TEST(UriIsDisallowedWildcardTest, DollarSignEndAnchorNoMatch) {
  std::string url{"http://example.com/path/file/extra"};
  std::string pattern{"/path/file$"};
  EXPECT_FALSE(urlIsDisallowed(url, pattern));
}

TEST(UriIsDisallowedWildcardTest, AsteriskBeforeDollarSign) {
  std::string url{"http://example.com/path/anything"};
  std::string pattern{"/path/*$"};
  EXPECT_TRUE(urlIsDisallowed(url, pattern));
}

TEST(UriIsDisallowedWildcardTest, AsteriskBeforeDollarSignNoMatch) {
  std::string url{"http://example.com/path/anything/more"};
  std::string pattern{"/path/anything$"};
  EXPECT_FALSE(urlIsDisallowed(url, pattern));
}

TEST(UriIsDisallowedWildcardTest, DollarSignWithExtension) {
  std::string url{"http://example.com/path/file.html"};
  std::string pattern{"*.html$"};
  EXPECT_TRUE(urlIsDisallowed(url, pattern));
}

TEST(UriIsDisallowedWildcardTest, DollarSignPreventsPrefixMatch) {
  std::string url{"http://example.com/path/file.html.bak"};
  std::string pattern{"*.html$"};
  EXPECT_FALSE(urlIsDisallowed(url, pattern));
}

TEST(UriIsDisallowedWildcardTest, ComplexPatternWithAsteriskAndDollar) {
  std::string url{"http://example.com/admin/secret"};
  std::string pattern{"/admin/*$"};
  EXPECT_TRUE(urlIsDisallowed(url, pattern));
}

TEST(UriIsDisallowedWildcardTest, ComplexPatternNoMatch) {
  std::string url{"http://example.com/admin/secret/extra"};
  std::string pattern{"/admin/*/secret$"};
  EXPECT_FALSE(urlIsDisallowed(url, pattern));
}

TEST(UriIsDisallowedWildcardTest, AtlantaExampleFromSpec) {
  std::string url1{"http://example.com/atlanta-101"};
  std::string url2{"http://example.com/atlanta-102"};
  std::string pattern{"/atlanta*"};
  EXPECT_TRUE(urlIsDisallowed(url1, pattern));
  EXPECT_TRUE(urlIsDisallowed(url2, pattern));
}

TEST(UriIsDisallowedWildcardTest, AsteriskMatchesDeepPath) {
  std::string url{"http://example.com/private/deep/nested/path"};
  std::string pattern{"/private*"};
  EXPECT_TRUE(urlIsDisallowed(url, pattern));
}

TEST(UriIsDisallowedWildcardTest, OnlyAsterisk) {
  std::string url{"http://example.com/anything"};
  std::string pattern{"*"};
  EXPECT_TRUE(urlIsDisallowed(url, pattern));
}

TEST(UriIsDisallowedWildcardTest, OnlyAsteriskWithSlash) {
  std::string url{"http://example.com/"};
  std::string pattern{"/*"};
  EXPECT_TRUE(urlIsDisallowed(url, pattern));
}

}  // namespace

}  // namespace crawler::services::url
