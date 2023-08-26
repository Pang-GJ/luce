#pragma once

#include <memory>
#include "luce/codec/abstract_coder.h"
#include "luce/codec/tinypb_protocol.h"
namespace codec {

class TinyPBCoder : AbstractCoder {
 public:
  TinyPBCoder() = default;
  ~TinyPBCoder() override = default;

  void Encode(AbstractProtocol::Ptr &in_message, Buffer *out_buffer) override;

  void Decode(const Buffer &in_buffer,
              AbstractProtocol::Ptr *out_message) override;

 private:
  const char *EncodeTinyPB(const std::shared_ptr<TinyPBProtocol> &message,
                           int *len);
};

}  // namespace codec