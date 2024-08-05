#ifndef TGCALLS_UWP_PLATFORM_CONTEXT_H
#define TGCALLS_UWP_PLATFORM_CONTEXT_H

#include <winrt/Windows.Graphics.Capture.h>
#include "../PlatformContext.h"

using namespace winrt::Windows::Graphics::Capture;

namespace lib::ortc {

class UwpContext : public PlatformContext {
public:
    UwpContext(GraphicsCaptureItem item) : item(item) {}

    virtual ~UwpContext() = default;

    GraphicsCaptureItem item;
};

}  // namespace lib::ortc

#endif
