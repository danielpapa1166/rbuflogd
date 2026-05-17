# rbuflogd

A lightweight logging daemon for Linux that uses a shared memory ring buffer to collect log messages from producers and write them to a log sink.

## How it works

- **Producers** (client processes) write log messages into a shared memory ring buffer via `librbuflogd_producer`.
- **Daemon** (`rbuflogd`) continuously consumes messages from the ring buffer and writes them to the log sink (stdout / file).
- Communication uses a POSIX shared memory segment (`/rbuflogd_shmem`) with an atomic lock-free ring buffer.

## Project structure

| Path | Description |
|------|-------------|
| `src/daemon/` | Daemon main loop, consumer, log sink |
| `src/producer/` | Producer and logger shared library |
| `src/common/` | Ring buffer and shared memory primitives |
| `include/rbuflogd/` | Public headers |
| `test/` | Smoke tests |

## Build

```sh
cmake -S . -B build
cmake --build build
```

## Usage

Start the daemon:
```sh
./build/rbuflogd
```

Link your application against `librbuflogd_producer`.

### Logger API (recommended)

The logger API manages a single process-wide producer instance behind a static handle. This is the simplest way to integrate logging.

```c
#include "rbuflogd/logger.h"

// Call once at startup — producer_name must be < 8 chars
rbuflogd_logger_init("myapp");

// Log at different levels: (category must be < 8 chars)
rbuflogd_logger_debug("net", "socket opened");
rbuflogd_logger_info("net", "connection established");
rbuflogd_logger_warning("net", "retrying connection");
rbuflogd_logger_error("net", "connection failed");

// Convenience macros (same signatures):
log_debug("net", "socket opened");
log_info("net", "connection established");
log_warning("net", "retrying connection");
log_error("net", "connection failed");

// Call once at shutdown
rbuflogd_logger_close();
```

All log functions return `0` on success or `-1` on failure (e.g. daemon not running or logger not initialised).

### Producer API (low-level)

Use this if you need multiple named producer instances within the same process.

```c
#include "rbuflogd/producer.h"

rbuflogd_producer_t p;
rbuflogd_producer_open(&p, "myapp");  // producer_name must be < 8 chars
rbuflogd_producer_log(&p, RBUF_LOG_LEVEL_INFO, "net", "connection established");
rbuflogd_producer_close(&p);
```

### Log levels

| Constant | Logger function | Macro |
|----------|----------------|-------|
| `RBUF_LOG_LEVEL_DEBUG` | `rbuflogd_logger_debug()` | `log_debug()` |
| `RBUF_LOG_LEVEL_INFO` | `rbuflogd_logger_info()` | `log_info()` |
| `RBUF_LOG_LEVEL_WARNING` | `rbuflogd_logger_warning()` | `log_warning()` |
| `RBUF_LOG_LEVEL_ERROR` | `rbuflogd_logger_error()` | `log_error()` |

Each log entry carries a producer name, log level, category (up to 8 chars), message, and both realtime and monotonic timestamps.
