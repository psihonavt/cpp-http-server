#pragma once

#if defined(__APPLE__) && defined(__MACH__)
#    define OSX_PLATFORM
#else
#    error "Unsupported platform"
#endif
