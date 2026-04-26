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
| `src/producer/` | Producer shared library |
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

Link your application against `librbuflogd_producer` and use the producer API:
```c
rbuflogd_producer_t p;
rbuflogd_producer_open(&p);
rbuflogd_producer_log(&p, "hello from producer");
rbuflogd_producer_close(&p);
```
