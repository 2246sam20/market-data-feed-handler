// Wire format shared by publisher and receiver.
//
// Design notes (be ready to defend these):
//  * #pragma pack(1) removes padding so the struct IS the wire layout. Without
//    it the compiler inserts padding and sizeof() differs from bytes on the
//    wire. Cost: unaligned loads. Fine here, and x86/arm64 both tolerate them.
//  * Fixed-width types only. `int` is 4 bytes here but that is not guaranteed,
//    and a feed handler must not care what the compiler picked.
//  * Price is an int64 of ticks, not a double. Floating point cannot represent
//    0.01 exactly, and you must never accumulate rounding error on a price.
//    Real feeds (ITCH, Pillar) do the same: integer + implied decimal places.
//  * Symbol is a fixed char[8], space-padded, NOT null-terminated. Fixed size
//    keeps the struct trivially copyable and the layout constant.

#pragma once

#include <cstdint>
#include <cstring>
#include <string>
#include <type_traits>

namespace fh {

// Price is expressed in ticks: PRICE_SCALE ticks == 1.00 currency unit.
inline constexpr int64_t PRICE_SCALE = 10000;  // 4 implied decimal places

inline constexpr size_t SYMBOL_LEN = 8;

enum class MsgType : uint8_t {
    Add     = 'A',
    Execute = 'E',
    Cancel  = 'X',
};

enum class Side : uint8_t {
    Buy  = 'B',
    Sell = 'S',
};

#pragma pack(push, 1)

struct Message {
    uint64_t seq;                 // monotonic sequence number, gap detection
    uint64_t order_id;            // exchange order id
    int64_t  price;               // in ticks; see PRICE_SCALE
    uint32_t qty;
    char     symbol[SYMBOL_LEN];  // space padded, not null terminated
    MsgType  type;
    Side     side;
    uint8_t  _pad[2];             // explicit: keeps sizeof a round 40 bytes
};

#pragma pack(pop)

// If these fail the wire format changed and publisher/receiver disagree.
static_assert(sizeof(Message) == 40, "wire format changed");
static_assert(std::is_trivially_copyable_v<Message>,
              "must be memcpy-able to and from a socket buffer");

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

// Copy a symbol into the fixed field, space padded. Truncates if too long.
inline void set_symbol(Message& m, const char* sym) {
    std::memset(m.symbol, ' ', SYMBOL_LEN);
    const size_t n = std::strlen(sym);
    std::memcpy(m.symbol, sym, n < SYMBOL_LEN ? n : SYMBOL_LEN);
}

// Read the symbol back, trailing spaces stripped.
inline std::string get_symbol(const Message& m) {
    size_t n = SYMBOL_LEN;
    while (n > 0 && m.symbol[n - 1] == ' ') --n;
    return std::string(m.symbol, n);
}

inline double to_double(int64_t ticks) {
    return static_cast<double>(ticks) / static_cast<double>(PRICE_SCALE);
}

inline int64_t from_double(double px) {
    return static_cast<int64_t>(px * static_cast<double>(PRICE_SCALE) + 0.5);
}

// ---------------------------------------------------------------------------
// Endianness
// ---------------------------------------------------------------------------
// Both ends are the same machine, so this is a no-op today. It exists because
// "what about endianness?" is a near-certain follow-up: real feeds specify a
// byte order (ITCH is big-endian) and a handler must convert on decode. Call
// these at the wire boundary only, never on the hot path twice.

inline Message hton(Message m) {
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    m.seq      = __builtin_bswap64(m.seq);
    m.order_id = __builtin_bswap64(m.order_id);
    m.price    = static_cast<int64_t>(__builtin_bswap64(static_cast<uint64_t>(m.price)));
    m.qty      = __builtin_bswap32(m.qty);
#endif
    return m;
}

inline Message ntoh(Message m) {
    return hton(m);  // byte swapping is its own inverse
}

}  // namespace fh
