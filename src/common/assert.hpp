#pragma once

#define VV_ENABLE_CHECKS

#define VV_REL_ASSERT(expr, msg) \
    if (!(expr)) { \
        throw std::runtime_error(msg); \
    }

#ifdef VV_ENABLE_CHECKS
#define VV_DEB_ASSERT(expr, msg) VV_REL_ASSERT(expr, msg)
#else
#define VV_DEB_ASSERT(expr, msg)
#endif
