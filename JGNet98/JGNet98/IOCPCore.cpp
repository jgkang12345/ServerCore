#include "pch.h"
#include "IOCPCore.h"
#include "Connection.h"

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
