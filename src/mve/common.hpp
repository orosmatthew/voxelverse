#pragma once

// #define MVE_ENABLE_VALIDATION

#define MVE_ASSERT(expr, msg) \
    if (!(expr)) { \
        throw std::runtime_error(msg); \
    }

#ifdef MVE_ENABLE_VALIDATION
#define MVE_VAL_ASSERT(expr, msg) MVE_ASSERT(expr, msg)
#else
#define MVE_VAL_ASSERT(expr, msg)
#endif