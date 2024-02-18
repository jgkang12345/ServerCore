#pragma once
class Connection;
class IOCPCore;
class DummyClientApp
{
protected:
	IOCPCore*		   _iocpCore;
	int32			   _maxDummyConnection;
	SpinLock		   _spinLock;
	tConnectionFactroy _tConnectionFactoryFunc;

public:
	DummyClientApp(
		int32 maxDummyConnection
		,const char* ip
		, tConnectionFactroy tConnectionFactoryFunc
	);

	virtual ~DummyClientApp();

	virtual void Start() abstract;

public:
	IOCPCore* GetIOCPCore() { return _iocpCore; }
};

