// Program 2: joins the multicast group, detects sequence gaps.
//
// STUB - Saturday items 3 and 4 fill this in. The gap logic below is the shape
// the real receive loop will use, exercised here against a fake packet stream.

#include "messages.hpp"

#include <cstdio>

namespace {

// Tracks the next sequence number we expect. Returns how many were missed.
class GapDetector {
public:
    // Returns 0 when in order, >0 when a gap opened, -1 for a duplicate or
    // an out-of-order packet we have already seen.
    int64_t on_packet(uint64_t seq) {
        if (!started_) {                 // first packet sets the baseline
            started_ = true;
            expected_ = seq + 1;
            return 0;
        }
        if (seq < expected_) return -1;  // duplicate / late
        const int64_t missed = static_cast<int64_t>(seq - expected_);
        expected_ = seq + 1;
        return missed;
    }

    uint64_t expected() const { return expected_; }

private:
    uint64_t expected_ = 0;
    bool     started_  = false;
};

}  // namespace

int main() {
    GapDetector gd;

    // Fake stream: 100, 101, then 102-104 dropped, then a duplicate.
    const uint64_t stream[] = {100, 101, 105, 106, 106};

    for (uint64_t seq : stream) {
        const uint64_t want = gd.expected();
        const int64_t  n    = gd.on_packet(seq);
        if (n > 0) {
            std::printf("GAP: expected %llu, got %llu (%lld missing)\n",
                        static_cast<unsigned long long>(want),
                        static_cast<unsigned long long>(seq),
                        static_cast<long long>(n));
        } else if (n < 0) {
            std::printf("DUP: %llu already seen\n",
                        static_cast<unsigned long long>(seq));
        } else {
            std::printf("OK : %llu\n", static_cast<unsigned long long>(seq));
        }
    }
    return 0;
}
