#include "pch.h"
#include "IOCPCore.h"
#include "Connection.h"
#include "LockBasedQueue.h"

IOCPCore::IOCPCore()
{
    SYSTEM_INFO sysInfo;
    GetSystemInfo(&sysInfo);

    int32 threadCount = sysInfo.dwNumberOfProcessors * 2;
    _iocpHandle = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, threadCount);
}

IOCPCore::~IOCPCore()
{
    CloseHandle(_iocpHandle);
}

void IOCPCore::RegisterIOCP(Connection* connection)
{
    CreateIoCompletionPort(reinterpret_cast<HANDLE>(connection->GetSocket()), _iocpHandle, 0, 0);
    connection->Recv(0);
}

void IOCPCore::Dispatch()
{
    DWORD           numOfBytes = 0;
    LPOVERLAPPED    overlapped;
    ULONG_PTR       ulCompletionKey;

    bool ret = GetQueuedCompletionStatus(_iocpHandle, &numOfBytes, &ulCompletionKey, &overlapped, INFINITE);
    JGOverlapped* expendedOverlapped = reinterpret_cast<JGOverlapped*>(overlapped);
    Connection* con = expendedOverlapped->connection;

    switch (expendedOverlapped->iocpType)
    {
    case JGOverlapped::IOCPType::Send:
        SendProc(con,ret,numOfBytes);
        break;

    case JGOverlapped::IOCPType::Recv:
        RecvPorc(con, ret, numOfBytes);
        break;
    }
}

void IOCPCore::RecvPorc(Connection* con, bool ret, int32 numOfBytes)
{
    if (ret == false || numOfBytes == 0)
    {
        con->OnDisconnect();
        delete con;
    }
    else
    {
        con->Recv(numOfBytes);
    }
}

void IOCPCore::SendProc(Connection* con, bool ret, int32 numOfBytes)
{
    LockBasedQueue<byte*>& sendQ = con->GetSendQueue();
    if (ret == FALSE)
    {
        // 치명적인 에러 접속을 끊어야 할 경우
        int errorCode = WSAGetLastError();
        if (errorCode == WSAECONNRESET
            || errorCode == WSAECONNABORTED
            || errorCode == WSAENOBUFS
            || errorCode == WSAESHUTDOWN
            || errorCode == WSAENETDOWN
            || errorCode == WSAENETRESET
            || errorCode == WSAEHOSTUNREACH
            || errorCode == WSAEMSGSIZE
            || errorCode == WSAETIMEDOUT
            )
        {
            while (sendQ.Empty() == false)
            {
                sendQ.Pop();
            }
            con->OnDisconnect();
            delete con;
            wprintf_s(L"Send Socket Error\n");
        }
        else
        {
            // 단순 에러, 다시 재전송
            byte* packet = sendQ.Front();
            const int32 packetSize = reinterpret_cast<PacketHeader*>(packet)->_pktSize;
            con->SendEx(packet, packetSize);
        }
    }
    else
    {
        sendQ.Pop();
        if (sendQ.Empty() == false)
        {
            byte* sendPacket = sendQ.Front();
            const int32 packetSize = reinterpret_cast<PacketHeader*>(sendPacket)->_pktSize;
            con->SendEx(sendPacket, packetSize);
        }
    }

}
