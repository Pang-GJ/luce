#include "common/json/JObject.hpp"
#include <sstream>


namespace tinyjson {
    using std::get_if;

    void *JObject::Value() {
        switch (type_) {
            case T_NULL:
                return get_if<str_t>(&value_);
            case T_BOOL:
                return get_if<bool_t>(&value_);
            case T_INT:
                return get_if<int_t>(&value_);
            case T_DOUBLE:
                return get_if<double_t>(&value_);
            case T_LIST:
                return get_if<list_t>(&value_);
            case T_DICT:
                return get_if<dict_t>(&value_);
            case T_STR:
                return get_if<str_t>(&value_);
            default:
                return nullptr;
        }
    }

// 用于简化指针类型转换的宏
#define GET_VALUE(type, value) *((type *) (value))

    string JObject::ToString() {
        void *value = this->Value();
        std::ostringstream os;

        switch (type_) {
            case T_NULL:
                os << "null";
                break;
            case T_BOOL:
                if (GET_VALUE(bool_t, value)) {
                    os << "true";
                } else {
                    os << "false";
                }
                break;
            case T_INT:
                os << GET_VALUE(int_t, value);
                break;
            case T_DOUBLE:
                os << GET_VALUE(double_t, value);
                break;
            case T_STR:
                os << '\"' << GET_VALUE(str_t, value) << '\"';
                break;
            case T_LIST: {
                list_t &list = GET_VALUE(list_t, value);
                os << '[';
                for (auto i = 0; i < list.size(); ++i) {
                    if (i != list.size() - 1) {
                        os << ((list[i]).ToString());
                        os << ',';
                    } else {
                        os << ((list[i]).ToString());
                    }
                }
                os << ']';
                break;
            }
            case T_DICT: {
                dict_t &dict = GET_VALUE(dict_t, value);
                os << '{';
                for (auto it = dict.begin(); it != dict.end(); ++it) {
                    if (it != dict.begin()) {
                        os << ',';
                    }
                    os << '\"' << it->first << "\":" << it->second.ToString();
                }
                os << '}';
                break;
            }
            default:
                return "";
        }
        return os.str();
    }

}  // namespace tinyjson
