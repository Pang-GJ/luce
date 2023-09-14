#include "luce/codec/tinypb_coder.h"
#include <netinet/in.h>
#include <cstring>
#include <iterator>
#include <memory>
#include <string>
#include "luce/codec/tinypb_protocol.h"
#include "luce/common/logger.h"
#include "luce/net/util.h"

namespace codec {

void TinyPBCoder::Encode(AbstractProtocol::Ptr& in_message,
                         Buffer* out_buffer) {
  std::shared_ptr<TinyPBProtocol> msg =
      std::dynamic_pointer_cast<TinyPBProtocol>(in_message);
  int len = 0;
  const char* buf = EncodeTinyPB(msg, &len);
  if (buf != nullptr && len != 0) {
    for (auto i = 0; i < len; ++i) {
      out_buffer->emplace_back(buf[i]);
    }
  }
  if (buf != nullptr) {
    delete buf;
    buf = nullptr;
  }
}

void TinyPBCoder::Decode(const Buffer& in_buffer,
                         AbstractProtocol::Ptr* out_message) {
  const auto buffer_size = in_buffer.size();
  int start_index = 0;
  // PB_START 是第一个位置
  if (buffer_size <= start_index ||
      in_buffer[start_index] != TinyPBProtocol::PB_START) {
    LOG_ERROR("decode tinypb error, cannot found PB_START");
    return;
  }
  // 读4个字符获取长度
  int pk_len = net::GetInt32ForNetByte(&in_buffer[start_index + 1]);
  LOG_DEBUG("pk_len = {}", pk_len);
  int end_index = start_index + pk_len - 1;
  if (buffer_size - 1 != end_index ||
      in_buffer[end_index] != TinyPBProtocol::PB_END) {
    LOG_ERROR("decode tinypb error, cannot found PB_END");
    return;
  }

  std::shared_ptr<TinyPBProtocol> message = std::make_shared<TinyPBProtocol>();
  message->pk_len = pk_len;
  const auto msg_id_len_index =
      start_index + sizeof(char) + sizeof(message->pk_len);
  if (msg_id_len_index >= end_index) {
    message->parse_success = false;
    LOG_ERROR("parse error, msg_id_len_index[{}] >=  end_index[{}]",
              msg_id_len_index, end_index);
    return;
  }
  message->msg_id_len = net::GetInt32ForNetByte(&in_buffer[msg_id_len_index]);
  LOG_DEBUG("parse msg_id_len = {}", message->msg_id_len);

  const auto msg_id_index = msg_id_len_index + sizeof(message->msg_id_len);
  char msg_id[100] = {0};
  std::memcpy(&msg_id[0], &in_buffer[msg_id_index], message->msg_id_len);
  message->msg_id = std::string(msg_id);
  LOG_DEBUG("parse msg_id = {}", message->msg_id);

  const auto method_name_len_index = msg_id_index + message->msg_id_len;
  if (method_name_len_index >= end_index) {
    message->parse_success = false;
    LOG_ERROR("parse error, method_name_len_index[{}] >=  end_index[{}]",
              method_name_len_index, end_index);
    return;
  }
  message->method_name_len =
      net::GetInt32ForNetByte(&in_buffer[method_name_len_index]);

  const auto method_name_index =
      method_name_len_index + sizeof(message->method_name_len);
  char method_name[512] = {0};
  std::memcpy(&method_name[0], &in_buffer[method_name_index],
              message->method_name_len);
  message->method_name = std::string(method_name);
  LOG_DEBUG("parse method_name = {}", method_name);

  const auto err_code_index = method_name_index + message->method_name_len;
  if (err_code_index >= end_index) {
    message->parse_success = false;
    LOG_ERROR("parse error, err_code_index[{}] >=  end_index[{}]",
              err_code_index, end_index);
    return;
  }
  message->err_code = net::GetInt32ForNetByte(&in_buffer[err_code_index]);

  const auto err_info_len_index = err_code_index + sizeof(message->err_code);
  if (err_info_len_index >= end_index) {
    message->parse_success = false;
    LOG_ERROR("parse error, err_info_len_index[{}] >= end_index[{}]",
              err_info_len_index, end_index);
    return;
  }
  message->err_info_len =
      net::GetInt32ForNetByte(&in_buffer[err_info_len_index]);

  const auto err_info_index =
      err_info_len_index + sizeof(message->err_info_len);
  char err_info[512] = {0};
  std::memcpy(&err_info[0], &in_buffer[err_info_index], message->err_info_len);
  message->err_info = std::string(err_info);
  LOG_DEBUG("parse err_info = {}", message->err_info);

  const auto pb_data_len = message->pk_len - message->method_name_len -
                           message->msg_id_len - message->err_info_len - 2 - 24;
  const auto pb_data_index = err_info_index + message->err_info_len;
  message->pb_data = std::string(&in_buffer[pb_data_index], pb_data_len);

  // TODO(pgj): check sum
  message->parse_success = true;
  *out_message = message;
}

const char* TinyPBCoder::EncodeTinyPB(
    const std::shared_ptr<TinyPBProtocol>& message, int* len) {
  if (message->msg_id.empty()) {
    message->msg_id = "123456789";
  }
  LOG_DEBUG("msg_id = {}", message->msg_id);
  const int pk_len = 2 + 24 + message->msg_id.length() +  // NOLINT
                     message->method_name.length() +
                     message->err_info.length() + message->pb_data.length();
  LOG_DEBUG("pk_len = {}", pk_len);

  char* buf = new char[pk_len];
  char* tmp = buf;

  *tmp = TinyPBProtocol::PB_START;
  tmp++;

  int32_t pk_len_net = net::GetNetInt32(pk_len);
  std::memcpy(tmp, &pk_len_net, sizeof(pk_len_net));
  tmp += sizeof(pk_len_net);

  int msg_id_len = static_cast<int>(message->msg_id.length());
  int32_t msg_id_len_net = net::GetNetInt32(msg_id_len);
  std::memcpy(tmp, &msg_id_len_net, sizeof(msg_id_len_net));
  tmp += sizeof(msg_id_len_net);

  if (!message->msg_id.empty()) {
    std::memcpy(tmp, message->msg_id.data(), msg_id_len);
    tmp += msg_id_len;
  }

  int method_name_len = static_cast<int>(message->method_name.length());
  int32_t method_name_len_net = net::GetNetInt32(method_name_len);
  std::memcpy(tmp, &method_name_len_net, sizeof(method_name_len_net));
  tmp += sizeof(method_name_len_net);

  if (!message->method_name.empty()) {
    std::memcpy(tmp, message->method_name.data(), method_name_len);
    tmp += method_name_len;
  }

  int32_t err_code_net = net::GetNetInt32(message->err_code);
  std::memcpy(tmp, &err_code_net, sizeof(err_code_net));
  tmp += sizeof(err_code_net);

  int err_info_len = static_cast<int>(message->err_info.length());
  int32_t err_info_len_net = net::GetNetInt32(err_info_len);
  std::memcpy(tmp, &err_info_len_net, sizeof(err_info_len_net));
  tmp += sizeof(err_info_len_net);

  if (!message->err_info.empty()) {
    std::memcpy(tmp, message->err_info.data(), err_info_len);
    tmp += err_info_len;
  }

  if (!message->pb_data.empty()) {
    std::memcpy(tmp, message->pb_data.data(), message->pb_data.length());
    tmp += message->pb_data.length();
  }

  int32_t check_sum_net = net::GetNetInt32(1);
  std::memcpy(tmp, &check_sum_net, sizeof(check_sum_net));
  tmp += sizeof(check_sum_net);

  *tmp = TinyPBProtocol::PB_END;

  message->pk_len = pk_len;
  message->msg_id_len = msg_id_len;
  message->method_name_len = method_name_len;
  message->err_info_len = err_info_len;
  message->parse_success = true;
  *len = pk_len;
  return buf;
}

}  // namespace codec