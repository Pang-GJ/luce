#pragma once

#include <vector>
#include "luce/codec/abstract_protocol.h"
namespace codec {

using Buffer = std::vector<char>;

class AbstractCoder {
 public:
  virtual ~AbstractCoder() = default;

  virtual void Encode(AbstractProtocol::Ptr& in_message,
                      Buffer* out_buffer) = 0;

  virtual void Decode(const Buffer& in_buffer,
                      AbstractProtocol::Ptr* out_messages) = 0;
};

}  // namespace codec