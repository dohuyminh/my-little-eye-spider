# Services

## Overview

The Services directory provides core functionality for HTTP operations, HTML parsing, concurrency, and URL handling.

## HTTP Services (`curl/`)

### `CURLEasyHandler` (`curl_handler.h`)

Singleton wrapper around a libcurl `CURL*` easy handle.

- **`get()`**: Returns the singleton instance
- **`handler()`**: Accessor for the underlying `CURL*`
- Copy construction/assignment deleted (singleton pattern)

### `Response` (`curl_response.h`)

Immutable HTTP response wrapper.

- **Constructor**: Takes content string and response code
- **`content()`**: Returns response body
- **`code()`**: Returns HTTP status code (e.g., 200, 404)

## HTML Services (`html/`)

### `Downloader` (`html_downloader.h`)

Function object for downloading HTML from a URL.

- **`operator()(const types::URL&)`**: Performs HTTP GET request, returns `curl::Response`
- **`writeCallback()`**: Static libcurl callback for accumulating response data

### `ParseResult` (`html_parser.h`)

Container for parsing results.

- **`outlinks()`**: Returns mutable reference to extracted link list

### `IHTMLParser` (concept)

Concept for HTML parser implementations.

- Requires `parse(const std::string&)` method returning `ParseResult`

### `HTMLParser<T>` (`html_parser.h`)

CRTP wrapper for parser implementations.

- **`parse()`**: Delegates to underlying parser type

### `LexborParser` (`html_parser.h`)

Lexbor-based HTML parser implementation.

- **Constructor**: Initializes lexbor parser
- **`parse()`**: Extracts outlinks from HTML string
- **Destructor**: Cleans up lexbor resources

## Concurrency Services (`concurrency/`)

### `ThreadPool` (`thread_pool.h`)

Thread pool with task queue and worker threads.

- **Constructor**: Creates pool with specified thread count
- **`start()`**: Spawns worker threads
- **`stop()`**: Signals workers to stop and joins threads
- **`isBusy()`**: Checks if task queue is non-empty
- **`enqueue()`**: Submits callable, returns `std::future` for result
- Copy construction/assignment deleted

## URL Services (`url/`)

### `ParseError` (enum)

Error types for URL parsing failures.

- `INVALID_URL`, `SCHEME_ERROR`, `AUTHORITY_ERROR`, `PATH_ERROR`, `QUERY_ERROR`, `FRAGMENT_ERROR`

### `URLParseResult` (struct)

Parsed URL components.

- `scheme`, `userinfo`, `subdomains`, `domain`, `port`, `path`, `queryParams`, `fragment`, `encodedURL`

### `ParseResult` (type alias)

`std::expected<URLParseResult, ParseError>` — success or error.

### `parse()`

Parses a URL string into components.

### `parseAndApplyRelativeURL()`

Resolves relative URL paths against a base.
