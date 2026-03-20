#include <gtest/gtest.h>

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
  EXPECT_TRUE(isRelativeURL(input));  // Invalid scheme start
}

TEST(IsRelativeURLTest, ColonNoSlashSlash) {
  std::string_view input{"foo:bar"};
  EXPECT_TRUE(isRelativeURL(input));  // No ://, treated as relative
}

// ============================================================================
// parse tests
// ============================================================================

TEST(ParseTest, ValidHttpUrl) {
  std::string url{"http://example.com"};
  ParseResult result{parse(url)};
  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(result->scheme, "http");
  EXPECT_EQ(result->domain, "example.com");
}

TEST(ParseTest, ValidHttpsUrlWithPath) {
  std::string url{"https://example.com/path/to/page"};
  ParseResult result{parse(url)};
  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(result->scheme, "https");
  EXPECT_EQ(result->domain, "example.com");
  EXPECT_FALSE(result->path.empty());
}

TEST(ParseTest, UrlWithQueryParams) {
  std::string url{"http://example.com/search?q=test&lang=en"};
  ParseResult result{parse(url)};
  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(result->queryParams["q"], "test");
  EXPECT_EQ(result->queryParams["lang"], "en");
}

TEST(ParseTest, UrlWithFragment) {
  std::string url{"http://example.com/page#section"};
  ParseResult result{parse(url)};
  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(result->fragment, "section");
}

TEST(ParseTest, UrlWithSubdomains) {
  std::string url{"http://www.blog.example.com"};
  ParseResult result{parse(url)};
  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(result->domain, "example.com");
  EXPECT_FALSE(result->subdomains.empty());
}

TEST(ParseTest, InvalidUrl) {
  std::string url{"://invalid"};
  ParseResult result{parse(url)};
  EXPECT_FALSE(result.has_value());
}

TEST(ParseTest, MissingAuthority) {
  std::string url{"http://"};
  ParseResult result{parse(url)};
  EXPECT_FALSE(result.has_value());
  EXPECT_EQ(result.error(), ParseError::INVALID_URL);
}

// ============================================================================
// parseRelativeURL tests
// ============================================================================

TEST(ParseRelativeURLTest, EmptyRelativeUrl) {
  std::vector<std::string> path{"/", "current", "path"};
  std::string relative{""};
  bool result{parseRelativeURL(relative, path)};
  EXPECT_TRUE(result);
  EXPECT_EQ(path.size(), 3);  // Unchanged
}

TEST(ParseRelativeURLTest, AbsolutePathFromRoot) {
  std::vector<std::string> path{"/", "old", "dir"};
  std::string relative{"/new/path"};
  bool result{parseRelativeURL(relative, path)};
  EXPECT_TRUE(result);
  EXPECT_GE(path.size(), 1);
  EXPECT_EQ(path[0], "/");
}

TEST(ParseRelativeURLTest, ParentDirectory) {
  std::vector<std::string> path{"/", "current", "dir"};
  std::string relative{"../parent"};
  bool result{parseRelativeURL(relative, path)};
  EXPECT_TRUE(result);
}

TEST(ParseRelativeURLTest, CurrentDirectory) {
  std::vector<std::string> path{"/", "current"};
  std::string relative{"./same"};
  bool result{parseRelativeURL(relative, path)};
  EXPECT_TRUE(result);
}

TEST(ParseRelativeURLTest, CannotGoAboveRoot) {
  std::vector<std::string> path{"/"};
  std::string relative{"../above"};
  bool result{parseRelativeURL(relative, path)};
  EXPECT_FALSE(result);
}

TEST(ParseRelativeURLTest, RejectsAbsoluteUrl) {
  std::vector<std::string> path{"/", "current"};
  std::string relative{"http://example.com"};
  bool result{parseRelativeURL(relative, path)};
  EXPECT_FALSE(result);  // isRelativeURL returns false for absolute URLs
}

}  // namespace

}  // namespace crawler::services::url
