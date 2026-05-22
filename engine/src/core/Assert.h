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

#define VK_CHECK_HANDLE(handle, type) \
    do { \
        if ((handle) == VK_NULL_HANDLE) { \
            LOG_ERROR("Invalid " #type); \
        } \
    } while(0)

#define CHECK_PTR(ptr, message) \
    do { \
        if ((ptr) == nullptr) { \
            LOG_ERROR((message)); \
        } \
    } while (0)