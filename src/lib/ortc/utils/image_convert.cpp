#include "image_convert.h"
#include "libyuv.h"

bool lib::ortc::image_convert::yuv420_to_argb(const uint8_t* src_y, int src_stride_y,
                                              const uint8_t* src_u, int src_stride_u,
                                              const uint8_t* src_v, int src_stride_v,
                                              uint8_t* dst_abgr, int dst_stride_abgr, int width,
                                              int height) {
    if (libyuv::I420ToARGB(src_y, src_stride_y, src_u, src_stride_u, src_v, src_stride_v, dst_abgr,
                           dst_stride_abgr, width, height) == 0) {
        return true;
    }
    return false;
}