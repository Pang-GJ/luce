#include "luce/codec/tinypb_protocol.h"

namespace codec {

char TinyPBProtocol::PB_START = 0x02;
char TinyPBProtocol::PB_END = 0x03;

}  // namespace codec