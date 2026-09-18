#include "messages.hpp"

#include <gtest/gtest.h>

using namespace fh;

TEST(WireFormat, SizeIsStable) {
    // If this fails, publisher and receiver no longer agree on the layout.
    EXPECT_EQ(sizeof(Message), 40u);
}

TEST(Symbol, PadsAndStrips) {
    Message m{};
    set_symbol(m, "AAPL");
    EXPECT_EQ(m.symbol[3], 'L');
    EXPECT_EQ(m.symbol[4], ' ');       // space padded, not null
    EXPECT_EQ(get_symbol(m), "AAPL");
}

TEST(Symbol, TruncatesOversized) {
    Message m{};
    set_symbol(m, "VERYLONGSYMBOL");
    EXPECT_EQ(get_symbol(m).size(), SYMBOL_LEN);
}

TEST(Price, RoundTripsExactly) {
    // The reason price is an integer: this is exact, a double would not be.
    EXPECT_EQ(from_double(187.25), 1872500);
    EXPECT_DOUBLE_EQ(to_double(1872500), 187.25);
    EXPECT_EQ(from_double(0.01), 100);
}

TEST(Endian, SwapIsItsOwnInverse) {
    Message m{};
    m.seq   = 0x0102030405060708ull;
    m.price = 1872500;
    m.qty   = 300;

    const Message back = ntoh(hton(m));
    EXPECT_EQ(back.seq, m.seq);
    EXPECT_EQ(back.price, m.price);
    EXPECT_EQ(back.qty, m.qty);
}
