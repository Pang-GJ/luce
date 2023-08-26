#pragma once

#include "luce/codec/abstract_protocol.h"
namespace codec {

class TinyPBProtocol : public AbstractProtocol {
 public:
  TinyPBProtocol() = default;
  ~TinyPBProtocol() override = default;

 public:
  static char PB_START;
  static char PB_END;

 public:
  int32_t pk_len{0};
  int32_t msg_id_len{0};

  int32_t method_name_len{0};
  std::string method_name;
  int32_t err_code{0};
  int32_t err_info_len{0};
  std::string err_info;
  std::string pb_data;
  int32_t check_sum{0};

  bool parse_success{false};
};

}  // namespace codec