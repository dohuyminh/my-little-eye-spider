# Worker Component

## Overview

The Worker component executes the actual crawling work: downloading HTML content from URLs and extracting outlinks for re-insertion into the frontier.

## Architecture

```
┌─────────────────┐     ┌─────────────────┐     ┌─────────────────┐
│   WorkerPool    │────▶│   Worker        │────▶│   Services      │
│ (thread pool)   │     │ (single task)   │     │ (download/parse)│
└─────────────────┘     └─────────────────┘     └─────────────────┘
```

## Classes

### `Worker` (`worker.h`)

Represents a single crawling task for one URL.

- **Constructor**: Takes a shared queue for producing URLs (outlinks) and the URL to crawl
- **`doWork()`**: Main work function:
  1. Downloads HTML via `Downloader`
  2. Returns early if HTTP status != 200
  3. Parses HTML via `HTMLParser<LexborParser>`
  4. Extracts outlinks (placeholder for frontier re-insertion)

### `WorkerPool` (`worker_pool.h`)

Manages a pool of worker threads that consume URLs from the frontier and produce outlinks.

Inherits from `types::Runnable` (event loop pattern).

- **Constructor**:
  - `numWorkers`: Thread pool size
  - `consumingURLs`: Shared queue receiving URLs from frontier
  - `producingURLs`: Shared queue sending extracted outlinks back to frontier
  - `batchSize`: Number of URLs to dequeue per iteration
- **`preLoop()`**: Starts the thread pool
- **`runImpl()`**: Main loop:
  1. Dequeues a batch of URLs from `consumingURLs`
  2. Enqueues each URL as a `Worker` job to the thread pool
- **`stop()`**: Stops the thread pool and marks runnable as stopped

## Concurrency Model

- Uses `moodycamel::ConcurrentQueue` for lock-free communication with frontier
- `WorkerPool` runs on an event loop thread (inherited from `Runnable`)
- Individual `Worker` tasks execute on the thread pool
