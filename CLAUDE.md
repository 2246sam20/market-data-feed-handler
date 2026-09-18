# Feed Handler

A mini market data feed handler in C++17, built as hands-on practice for an ICE
C++ developer interview. Learning project, not a production system: keep code
small and readable, and explain design choices briefly — I have to defend them
out loud.

Interview prep material (script, timeline, topic checklist, resources) lives in
[PREP.md](PREP.md). Read it when I ask prep or interview questions; you don't
need it to write code.

## Dev environment — container only
The host is an **arm64 MacBook Air**, where Valgrind, GDB, `pthread_setaffinity_np`
and `taskset` do **not** work. All building, running and debugging happens inside a
Linux container. **Never build this project directly on the Mac.**

```bash
./dev.sh                  # interactive shell in the container (repo at /work)
./dev.sh <cmd>            # run one command inside it
```

Build and test:
```bash
./dev.sh bash -c 'cmake -S . -B build && cmake --build build -j4'
./dev.sh ./build/fh_tests
```

- Rebuild the image only if `.docker/Dockerfile` changes:
  `docker build -t feed-handler-dev -f .docker/Dockerfile .`
- Docker runs on **Colima** (`colima start --cpu 4 --memory 4` if the daemon is down).
- Verified in-container: GCC 13.3, CMake 3.28, Valgrind 3.22 (Memcheck, Helgrind and
  Callgrind all produce real findings on arm64), GDB 15.1, 4 cores,
  `pthread_setaffinity_np`, loopback multicast with `IP_ADD_MEMBERSHIP` on `127.0.0.1`.
- Run publisher and receiver in **one** container (two shells via `docker exec`, or
  background one) so multicast stays in a single network namespace.
- `fh_tests` only exists if CMake's `find_package(GTest)` succeeded — check the
  configure output before assuming a build failure.

**Mount gotcha:** the real home is `/Users/SAM` (uppercase). macOS opens `/Users/sam`
too, but Colima mounts only the true casing — the wrong spelling silently mounts an
**empty** directory and cmake reports a missing CMakeLists.txt. `dev.sh` normalises
this; don't hand-write `docker run -v`.

**arm64 caveat:** `rdtsc` is x86-only. Use `std::chrono::steady_clock` for timing.

**Multicast gotcha:** if localhost multicast receives nothing, set `IP_MULTICAST_IF`
to `127.0.0.1` on the publisher and join the group on `127.0.0.1` in the receiver.

## Design rules (non-negotiable — they're the interview story)
- **No locks** on the hot path. Threads communicate through SPSC ring buffers using
  acquire/release atomics, with cache-line alignment to avoid false sharing.
- **No allocations** on the hot path. Use a pre-allocated memory pool.
- **No moving threads.** Pin each thread to its own core with `pthread_setaffinity_np`
  or `taskset`.
- Busy-poll a non-blocking socket.
- Every performance number in docs or the interview script must be **measured**, never
  estimated. If it hasn't been run, leave the placeholder.

## What we're building
Four pieces, all inside one Linux container over localhost:

1. **Publisher** (`src/publisher.cpp`) — generates synthetic order messages (add,
   execute, cancel), each with a sequence number, over **UDP multicast**. Drops some
   packets at random on purpose. Later also runs a small **TCP retransmit server**.
2. **Receiver** (`src/receiver.cpp`) — joins the multicast group, reads packets,
   **detects sequence gaps**, requests missing ranges over TCP, buffers newer packets
   until the gap is filled.
3. **SPSC queue** (`include/spsc_queue.hpp`) — lock-free ring buffer, coded along with
   the Charles Frasch CppCon 2023 talk. Connects receiver thread to book thread.
4. **Book thread** (inside the receiver) — decodes messages, keeps best bid/ask per symbol.

Then measure and break it: p50/p99 latency from receive to publish using
`steady_clock`; a deliberate data race for **Helgrind**, a leak for **Memcheck**, a
deadlock or core dump for **GDB** (`thread apply all bt`); **Callgrind** profiling;
**GTest** tests for gaps, duplicates and out-of-order packets.

### Repo layout
```
feed-handler/
├── CMakeLists.txt
├── include/
│   ├── spsc_queue.hpp    # NOT YET WRITTEN - from the Frasch video
│   └── messages.hpp      # packed message structs shared by both programs
├── src/
│   ├── publisher.cpp
│   └── receiver.cpp
└── tests/
```

## Build order
1. `spsc_queue.hpp` (Frasch talk). Test with two threads doing 1M pushes/pops, nothing lost.
2. `messages.hpp`: packed struct with seq, type, symbol, price, qty.
3. Publisher, then receiver, until messages flow.
4. Gap detection printing `GAP: expected 102, got 105`.
5. Receiver thread → SPSC queue → book thread (best bid/ask).
6. TCP retransmit: publisher serves missing ranges, receiver buffers then drains in order.
7. Latency measurement (p50/p99) and a memory pool for messages.
8. Break it on purpose — race for Helgrind, leak for Memcheck, deadlock for GDB. Fix each.

**If short on time:** cut item 6 (TCP retransmit) first. Never cut gap detection, the
SPSC queue, or the Valgrind/GDB drills.

## Working style
- C++17 or later, CMake, GCC on Linux, GTest for tests.
- Build incrementally, following the build order above.
- **After each milestone, quiz me with 2–3 likely interview cross-questions about what
  we just built.** Don't skip this and don't ask permission — it's how I retain the
  design. Ask hard ones; the answers are in PREP.md if I'm stuck.
