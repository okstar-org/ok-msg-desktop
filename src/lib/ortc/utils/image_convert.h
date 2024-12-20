#pragma once

#include <stdint.h>

namespace lib::ortc {
class image_convert {
public:
    static bool yuv420_to_argb(const uint8_t* src_y,
                               int src_stride_y,
                               const uint8_t* src_u,
                               int src_stride_u,
                               const uint8_t* src_v,
                               int src_stride_v,
                               uint8_t* dst_abgr,
                               int dst_stride_abgr,
                               int width,
                               int height);
};
}  // namespace lib::ortc