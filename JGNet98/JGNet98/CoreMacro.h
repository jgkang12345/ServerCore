#pragma once

// ����׿� ���� �߻� ��ũ��
#define DEBUG_ERROR(condition, message) \
    if (!(condition)) { \
        std::cerr << "Error: " << message << std::endl; \
        assert(condition); \
    }

// RecvBuffer ������
#define RECV_BUFFER_CHUNK 4096
#define RECV_BUFFER_COUNT 7
#define RECV_BUFFER_LEN 4096 * RECV_BUFFER_COUNT