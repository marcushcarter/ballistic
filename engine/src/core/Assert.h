#ifdef _DEBUG
#define BE_ASSERT(expr) \
    do { \
        if (!(expr)) { \
            LOG_FATAL("Assertion failed: %s (%s:%d)", #expr, __FILE__, __LINE__); \
            __debugbreak(); \
        } \
    } while(0)
#else
#define BE_ASSERT(expr) ((void)(expr))
#endif

#define VK_CHECK_HANDLE(handle, type, retval) \
    do { \
        if ((handle) == VK_NULL_HANDLE) { \
            errorLog.Set("Invalid " #type, false); \
            return retval; \
        } \
    } while(0)

#define CHECK_PTR(ptr, message, retval) \
    do { \
        if ((ptr) == nullptr) { \
            errorLog.Set((message), false); \
            return retval; \
        } \
    } while (0)