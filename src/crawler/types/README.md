# Types

## Overview

Core type definitions used throughout the crawler codebase.

## Classes

### `Runnable` (`runnable.h`)

Event loop base class for long-running components (e.g., `WorkerPool`, `Frontier`).

Provides a thread-safe run/stop lifecycle.

- **`preLoop()`**: Optional hook called before main loop (override for setup)
- **`run()`**: Public entry point — spawns `eventLoopThread_`, calls `runImpl()`
- **`runImpl()`**: Pure virtual; implement main loop logic here
- **`stop()`**: Sets `isRunning_` to false, joins thread
- **`isRunning()`**: Returns current running state

**Usage pattern**:
```cpp
class MyComponent : public Runnable {
  void runImpl() override {
    while (isRunning()) {
      // do work
    }
  }
};
```

### `URL` (`url.h`)

Immutable URL wrapper with parsed components.

**Construction**:
- Default constructor
- `URL(const std::string&)` — copy from lvalue
- `URL(std::string&&)` — move from rvalue
- Copy/move constructors and assignment operators

**Accessors**:
- **`url()`**: Full URL string
- **`scheme()`**: Protocol (e.g., `http`, `https`)
- **`subdomains()`**: Subdomain list
- **`domain()`**: Registered domain
- **`port()`**: Port string (if specified)
- **`path()`**: Reconstructed path string
- **`root()`**: Scheme + domain prefix (e.g., `https://example.com/`)
- **`query(key)`**: Optional query param value by key
- **`fragment()`**: URL fragment
- **`to_string()`**: Alias for `url()`

**Internal storage**:
- `userinfo_`, `scheme_`, `subdomains_`, `domain_`, `port_`, `path_`, `queryParams_`, `fragment_`
