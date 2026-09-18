// Program 1: synthetic market data over UDP multicast.
//
// STUB - Saturday item 3 fills this in. Today it only proves the wire format
// and the build work end to end.

#include "messages.hpp"

#include <cstdio>

int main() {
    fh::Message m{};
    m.seq      = 101;
    m.order_id = 5001;
    m.price    = fh::from_double(187.25);
    m.qty      = 300;
    m.type     = fh::MsgType::Add;
    m.side     = fh::Side::Buy;
    fh::set_symbol(m, "AAPL");

    std::printf("publisher stub: seq=%llu %s %c %u @ %.4f (%zu bytes on wire)\n",
                static_cast<unsigned long long>(m.seq),
                fh::get_symbol(m).c_str(),
                static_cast<char>(m.side),
                m.qty,
                fh::to_double(m.price),
                sizeof(fh::Message));
    return 0;
}
