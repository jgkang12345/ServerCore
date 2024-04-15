#include "pch.h"
#include "JGOverlapped.h"

JGOverlapped::JGOverlapped(JGOverlapped::IOCPType _iocpType) 
	: iocpType(_iocpType), acceptOwner(nullptr)
{
	memset(this, 0, sizeof(WSAOVERLAPPED));
	hEvent = NULL;
}

JGOverlapped::~JGOverlapped()
{

}
