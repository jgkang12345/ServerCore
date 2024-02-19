#pragma once

// 디버그용 에러 발생 매크로
#define DEBUG_ERROR(condition, message) \
    if (!(condition)) { \
        std::cerr << "Error: " << message << std::endl; \
        assert(condition); \
    }

// RecvBuffer 사이즈
#define RECV_BUFFER_CHUNK 4096
#define RECV_BUFFER_COUNT 7
#define RECV_BUFFER_LEN 4096 * RECV_BUFFER_COUNT