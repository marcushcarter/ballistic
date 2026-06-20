#pragma once
#include <cstdio>

#define BALLISTIC_ERR_FAIL_COND_V(m_cond, m_retval) \
    if (m_cond) { \
        fprintf(stderr, "[Ballistic] Condition \"%s\" failed at %s:%d\n", #m_cond, __FILE__, __LINE__); \
        return m_retval; \
    } else \
        ((void)0)

#define BALLISTIC_ERR_FAIL_COND_V_MSG(m_cond, m_retval, m_msg) \
    if (m_cond) { \
        fprintf(stderr, "[Ballistic] Condition \"%s\" failed at %s:%d\n%s\n", #m_cond, __FILE__, __LINE__, m_msg); \
        return m_retval; \
    } else \
        ((void)0)

#define BALLISTIC_ERR_FAIL_COND(m_cond) \
    if (m_cond) { \
        fprintf(stderr, "[Ballistic] Condition \"%s\" failed at %s:%d\n", #m_cond, __FILE__, __LINE__); \
        return; \
    } else \
        ((void)0)