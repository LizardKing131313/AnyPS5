#include "prx/libc/include/General.hpp"

extern "C" [[noreturn]] void NotImplementedAt_nid_no_patch(const char* function, const char*, int) {
    throw std::runtime_error(function);
}
