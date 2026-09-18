# ICE Interview Prep — study material

Reference material for me (Sam) to read, not instructions for Claude Code.
Build/environment instructions live in [CLAUDE.md](CLAUDE.md).

## The interview
- **L1 round, Tue 22 Sep 2026, 3–4 PM IST**, on Teams.
- Keep your government ID ready. Don't record the call.
- **Recruiter's focus areas:** C++, Linux, Market Data, TCP/IP, UDP, Valgrind, GDB.
- **What past ICE L1 rounds looked like:** mostly conceptual C++ questions, asked
  quickly. Examples: `delete this`, the diamond problem, `extern "C"` and linking,
  C vs C++ structs, writing a thread pool, easy DSA problems.
- **Resume project this backs up:** "Low-Latency Market Data Feed Handler" (C++17,
  multithreading, sockets, lock-free queues, memory pools, CPU pinning, sub-µs
  processing, Valgrind/Callgrind, GTest). Every claim must be defensible, and every
  number must be measured.

## Full timeline
| Day | Plan |
|---|---|
| Fri 18 (eve) | Order book basics video + Keysight multicast-gaps article. Send your ID to the recruiter. |
| Sat 19 | AM: C++ concepts. PM: Frasch video + publisher/receiver/gap detection. Eve: Carl Cook talk. |
| Sun 20 | AM: SPSC → book thread, retransmit, latency numbers. PM: TCP/UDP/multicast theory + NYSE Pillar recovery section. Eve: David Gross talk. |
| Mon 21 | AM: Greg Law debugging talk + Valgrind/GDB drills on own code. PM: Linux topics + update script with real numbers. Eve: mock interview. |
| Tue 22 | AM: revision only; say the walkthrough out loud 2–3 times. 2:40 PM test Teams. **3 PM interview.** |

## Topic checklist
- **C++:** vtables/vptr, virtual inheritance and the diamond problem, `delete this`,
  `extern "C"` and name mangling, rule of 5, move semantics, smart pointer internals,
  RAII, memory model (acquire/release), false sharing, `alignas`, placement new,
  CRTP vs virtual. Write a thread pool, an SPSC ring and a thread-safe queue from memory.
- **Linux:** process vs thread, `fork`/`exec`, shared memory/`mmap`, `select`/`poll`/`epoll`,
  context switches, CPU isolation and `taskset`, huge pages, `/proc`, core dumps, `perf`, `strace`.
- **Networking:** TCP handshake, TIME_WAIT, Nagle and `TCP_NODELAY`, flow vs congestion
  control, UDP loss and reordering, multicast IGMP joins and `IP_ADD_MEMBERSHIP`,
  `SO_RCVBUF` and drops, shared vs source multicast trees, kernel bypass (OpenOnload, DPDK).
- **Market data:** order book L1/L2/L3, add/modify/delete/execute messages, sequence gaps,
  A/B line arbitration, snapshot and retransmit recovery, binary decoding and endianness.
  ICE's own feed is **ICE iMpact** (UDP multicast).
- **Valgrind/GDB:** Memcheck leak categories (definitely lost / indirectly lost /
  possibly lost / still reachable), Helgrind race reports, Callgrind, `bt`,
  `thread apply all bt`, watchpoints, attaching to a process, reading core dumps,
  trouble debugging `-O2` builds, ASan/TSan as alternatives.

## Interview script (2 min, simple version)
"Market data is basically the stock exchange shouting every price change at you,
millions of times a second, over UDP multicast. UDP doesn't wait for anyone. If you're
slow, you miss messages, and a missed message means a wrong price. So my feed handler
had one goal: **keep up, and never lose track.**

I split the work into **three jobs**, each on its own thread.
- **Catch:** one thread listens on the multicast socket. Every packet has a sequence
  number, so if I see 101 and then 105, I know 102 to 104 are missing. I request those
  over a TCP recovery channel and hold newer packets until the gap is filled.
- **Read:** this thread decodes the compact binary messages in place, with no copying.
- **Translate:** this thread converts each exchange's format into one clean internal format.

To keep it fast, I followed **three rules**.
- **No locks:** threads pass data through lock-free ring buffers.
- **No allocations:** memory is pre-allocated in a pool.
- **No moving threads:** each thread is pinned to its own core.

Result: processing a message took under a microsecond [median X ns, p99 Y ns]. I found
slow spots with Callgrind and tested dropped, duplicated and out-of-order packets with
GTest. Next, I'd bypass the kernel network stack, because that's now where most of the
time goes."

**Delivery:** pause after "keep up, and never lose track," count the jobs and rules on
your fingers, and let the interviewer pull out the deeper technical details.

> The X ns / Y ns placeholders must be replaced with numbers you actually measured
> before Tuesday. Never quote a number you haven't seen come out of your own build.

## Likely follow-ups
- **Why SPSC and not MPMC?** Each queue has one producer and one consumer, so SPSC needs
  no CAS loops and has no ABA problem. It only needs two atomics.
- **Why acquire/release and not seq_cst?** The release store on tail publishes the data,
  and the acquire load on tail makes it visible to the consumer. seq_cst would add
  unnecessary fences.
- **What if the queue is full?** Either drop and flag, or apply backpressure, which risks
  socket drops. Know which one you chose and why.
- **Why busy-poll?** It avoids wakeup latency, at the cost of one core per thread.
- **What happens during an open gap?** Buffer the later packets, apply the recovered range
  in order, then drain the buffer. If the gap is too big, fall back to a snapshot.
- **How did you measure sub-µs?** Explain what you measured (inside the handler, not
  wire-to-wire), which clock you used, and which percentile you're quoting.
- **Why `steady_clock` and not `rdtsc`?** `rdtsc` is x86-only (this was built on arm64),
  and it needs an invariant-TSC guarantee plus a serialising instruction to be
  trustworthy anyway. The arm64 equivalent is `mrs cntvct_el0`.

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
