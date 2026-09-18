# Feed Handler — ICE Interview Prep Project

## Context
- **Goal:** Build a mini market data feed handler in C++ as hands-on practice for an ICE (Intercontinental Exchange) C++ developer interview. Building it helps me remember the design and answer cross-questions with confidence.
- **Interview:** L1 round, **Tue 22 Sep 2026, 3–4 PM IST**, on Teams. Keep your government ID ready and don't record the call.
- **Focus areas from the recruiter:** C++, Linux, Market Data, TCP/IP, UDP, Valgrind, GDB.
- **What past ICE L1 rounds looked like:** mostly conceptual C++ questions, asked quickly. Examples: `delete this`, the diamond problem, `extern "C"` and linking, C vs C++ structs, writing a thread pool, easy DSA problems.
- **Resume project this backs up:** "Low-Latency Market Data Feed Handler" (C++17, multithreading, sockets, lock-free queues, memory pools, CPU pinning, sub-µs processing, Valgrind/Callgrind, GTest). Every claim must be defensible, and every number must be measured.

## Dev environment (verified Fri 18 Sep 2026)
The host is an **arm64 MacBook Air**, where Valgrind, GDB, `pthread_setaffinity_np`
and `taskset` do **not** work. All building, running and debugging therefore happens
inside a Linux container. Never build this project directly on the Mac.

- `./dev.sh` — interactive shell in the container (repo mounted at `/work`).
- `./dev.sh <cmd>` — run one command inside it.
- Rebuild the image only if `.docker/Dockerfile` changes:
  `docker build -t feed-handler-dev -f .docker/Dockerfile .`
- Docker runs on **Colima** (`colima start --cpu 4 --memory 4` if the daemon is down).

Verified working inside the container: GCC 13.3, CMake 3.28, Valgrind 3.22
(Memcheck **and** Helgrind **and** Callgrind all produce real findings on arm64),
GDB 15.1, 4 cores, `pthread_setaffinity_np`, and loopback multicast with
`IP_ADD_MEMBERSHIP` on `127.0.0.1`.

Run the publisher and receiver in **one** container (two shells via `docker exec`,
or background one of them) so multicast stays inside a single network namespace.

Build and test:
```
./dev.sh bash -c 'cmake -S . -B build && cmake --build build -j4'
./dev.sh ./build/fh_tests
```

Mount gotcha: the real home is `/Users/SAM` (uppercase). macOS opens
`/Users/sam` too, but Colima mounts only the true casing — the wrong spelling
silently mounts an **empty** directory and cmake reports a missing
CMakeLists.txt. `dev.sh` normalises this; don't hand-write `docker run -v`.

arm64 caveat: `rdtsc` is x86-only. Use `std::chrono::steady_clock`, or
`mrs cntvct_el0` if asked about cycle counters. Say "steady_clock" in the
interview unless they push — and know why: `rdtsc` needs an invariant-TSC
guarantee and a serialising instruction to be trustworthy anyway.

## What we're building
Four pieces, all running inside one Linux container over localhost:

1. **Publisher** (`src/publisher.cpp`, its own program). Generates synthetic order messages (add, execute, cancel), each with a sequence number, and sends them over **UDP multicast**. It drops some packets at random on purpose. Later it also runs a small **TCP retransmit server**.
2. **Receiver** (`src/receiver.cpp`, its own program). Joins the multicast group, reads packets, and **detects sequence gaps**. It requests the missing range over TCP and buffers newer packets until the gap is filled.
3. **SPSC queue** (`include/spsc_queue.hpp`). A lock-free ring buffer built by coding along with the Charles Frasch CppCon 2023 talk. It connects the receiver thread to the book thread.
4. **Book thread** (inside the receiver). Decodes messages and keeps the best bid and ask per symbol.

Then **measure and break it**:
- Measure p50/p99 latency from receive to publish using `rdtsc` or `steady_clock`.
- Add bugs on purpose and catch them: a data race with **Helgrind**, a leak with **Memcheck**, and a deadlock or core dump with **GDB** (`thread apply all bt`).
- Profile with **Callgrind**, and write **GTest** tests for gaps, duplicates and out-of-order packets.

### Repo layout
```
feed-handler/
├── CMakeLists.txt
├── include/
│   ├── spsc_queue.hpp    # from the Frasch video
│   └── messages.hpp      # packed message structs shared by both programs
├── src/
│   ├── publisher.cpp     # program 1
│   └── receiver.cpp      # program 2 (later adds the book thread)
└── tests/
```

### Design rules (the "three rules" in the interview script)
- **No locks** on the hot path. Threads communicate through SPSC ring buffers using acquire/release atomics, with cache-line alignment to avoid false sharing.
- **No allocations** on the hot path. Use a pre-allocated memory pool.
- **No moving threads.** Pin each thread to its own core with `pthread_setaffinity_np` or `taskset`.
- Busy-poll a non-blocking socket.

### Gotcha
If localhost multicast receives nothing, set `IP_MULTICAST_IF` to `127.0.0.1` on the publisher and join the group on `127.0.0.1` in the receiver.

## Build order
**Sat afternoon**
1. `spsc_queue.hpp`, coded along with the Frasch talk. Test it with two threads doing 1M pushes and pops, checking nothing is lost.
2. `messages.hpp`: a packed struct with seq, type, symbol, price and qty.
3. Publisher, then receiver, until messages are flowing.
4. Gap detection in the receiver, printing `GAP: expected 102, got 105`.

**Sun**
5. Receiver thread → SPSC queue → book thread (best bid and ask).
6. TCP retransmit: the publisher serves missing ranges and the receiver buffers, then drains in order.
7. Latency measurement (p50/p99) and a memory pool for messages.

**Mon morning**
8. Break it on purpose: a race for Helgrind, a leak for Memcheck, a deadlock for GDB. Then fix each one.

**If short on time:** cut item 6 (TCP retransmit) first. Never cut gap detection, the SPSC queue, or the Valgrind/GDB drills.

## Full timeline
| Day | Plan |
|---|---|
| Fri 18 (eve) | Order book basics video + Keysight multicast-gaps article. Send your ID to the recruiter. |
| Sat 19 | AM: C++ concepts. PM: Frasch video + publisher/receiver/gap detection. Eve: Carl Cook talk. |
| Sun 20 | AM: SPSC → book thread, retransmit, latency numbers. PM: TCP/UDP/multicast theory + NYSE Pillar recovery section. Eve: David Gross talk. |
| Mon 21 | AM: Greg Law debugging talk + Valgrind/GDB drills on own code. PM: Linux topics + update script with real numbers. Eve: mock interview. |
| Tue 22 | AM: revision only; say the walkthrough out loud 2–3 times. 2:40 PM test Teams. **3 PM interview.** |

## Topic checklist
- **C++:** vtables/vptr, virtual inheritance and the diamond problem, `delete this`, `extern "C"` and name mangling, rule of 5, move semantics, smart pointer internals, RAII, memory model (acquire/release), false sharing, `alignas`, placement new, CRTP vs virtual. Write a thread pool, an SPSC ring and a thread-safe queue from memory.
- **Linux:** process vs thread, `fork`/`exec`, shared memory/`mmap`, `select`/`poll`/`epoll`, context switches, CPU isolation and `taskset`, huge pages, `/proc`, core dumps, `perf`, `strace`.
- **Networking:** TCP handshake, TIME_WAIT, Nagle and `TCP_NODELAY`, flow vs congestion control, UDP loss and reordering, multicast IGMP joins and `IP_ADD_MEMBERSHIP`, `SO_RCVBUF` and drops, shared vs source multicast trees, kernel bypass (OpenOnload, DPDK).
- **Market data:** order book L1/L2/L3, add/modify/delete/execute messages, sequence gaps, A/B line arbitration, snapshot and retransmit recovery, binary decoding and endianness. ICE's own feed is **ICE iMpact** (UDP multicast).
- **Valgrind/GDB:** Memcheck leak categories (definitely lost / indirectly lost / possibly lost / still reachable), Helgrind race reports, Callgrind, `bt`, `thread apply all bt`, watchpoints, attaching to a process, reading core dumps, trouble debugging `-O2` builds, ASan/TSan as alternatives.

## Interview script (2 min, simple version)
"Market data is basically the stock exchange shouting every price change at you, millions of times a second, over UDP multicast. UDP doesn't wait for anyone. If you're slow, you miss messages, and a missed message means a wrong price. So my feed handler had one goal: **keep up, and never lose track.**

I split the work into **three jobs**, each on its own thread.
- **Catch:** one thread listens on the multicast socket. Every packet has a sequence number, so if I see 101 and then 105, I know 102 to 104 are missing. I request those over a TCP recovery channel and hold newer packets until the gap is filled.
- **Read:** this thread decodes the compact binary messages in place, with no copying.
- **Translate:** this thread converts each exchange's format into one clean internal format.

To keep it fast, I followed **three rules**.
- **No locks:** threads pass data through lock-free ring buffers.
- **No allocations:** memory is pre-allocated in a pool.
- **No moving threads:** each thread is pinned to its own core.

Result: processing a message took under a microsecond [median X ns, p99 Y ns]. I found slow spots with Callgrind and tested dropped, duplicated and out-of-order packets with GTest. Next, I'd bypass the kernel network stack, because that's now where most of the time goes."

Delivery: pause after "keep up, and never lose track," count the jobs and rules on your fingers, and let the interviewer pull out the deeper technical details.

### Likely follow-ups
- **Why SPSC and not MPMC?** Each queue has one producer and one consumer, so SPSC needs no CAS loops and has no ABA problem. It only needs two atomics.
- **Why acquire/release and not seq_cst?** The release store on tail publishes the data, and the acquire load on tail makes it visible to the consumer. seq_cst would add unnecessary fences.
- **What if the queue is full?** Either drop and flag, or apply backpressure, which risks socket drops. Know which one you chose and why.
- **Why busy-poll?** It avoids wakeup latency, at the cost of one core per thread.
- **What happens during an open gap?** Buffer the later packets, apply the recovered range in order, then drain the buffer. If the gap is too big, fall back to a snapshot.
- **How did you measure sub-µs?** Explain what you measured (inside the handler, not wire-to-wire), which clock you used, and which percentile you're quoting.

## Resources
**Code references (use them, don't read cover to cover):**
- github.com/PacktPublishing/Building-Low-Latency-Applications-with-CPP (lock-free queue, memory pool, multicast publisher and consumer)
- github.com/cjramsey/NASDAQ-ITCH-LOB (ITCH decoding, direct processing vs SPSC trade-off)
- github.com/aanrv/Order-Book (memory pool, profiling notes)
- github.com/CharlesFrasch/cppcon2023 (SPSC code from the talk)
- Skip real ITCH data files (about 13GB per day); use synthetic messages.

**Videos (in order):**
1. Limit order book fundamentals: youtube.com/watch?v=BJt6PGKeO9I
2. Carl Cook, "When a Microsecond Is an Eternity": youtube.com/watch?v=NH1Tta7purM
3. Charles Frasch, "SPSC Lock-free FIFO From the Ground Up": youtube.com/watch?v=K3P_Lmq6pw0
4. David Gross, "When Nanoseconds Matter": youtube.com/watch?v=sX2nF1fW7kI
5. Greg Law, "Back to Basics: Debugging" (CppCon 2023)

**Reading:**
- Keysight, "Why You Can't Trust Your Market Data Feed: Multicast Gaps"
- Databento, "What is NYSE Pillar?" (databento.com/microstructure/nyse-pillar)
- NYSE Pillar Common Client Specification, gap recovery and retransmission sections
- ICE iMpact feed handler feature list (onixs.biz/ice-impact-multicast-price-feed.html)
- Glassdoor ICE Senior Developer interview reports

## Instructions for Claude Code
- Target C++17 or later, CMake, GCC on Linux, and GTest for tests.
- Help me build this incrementally, following the build order above. Explain design choices briefly, since I need to defend them in the interview.
- Keep code small and readable. This is a learning project, not a production system.
- After each milestone, quiz me with 2–3 likely interview cross-questions about what we just built.
