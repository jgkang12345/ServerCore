#pragma once
class TCPConnector
{
private:
	tConnectionFactroy _tConnectionFactoryFunc;
	char			   _connectIp[256] = {};
	uint16			   _connectPort;

public:
	TCPConnector(const char* connectIp, uint16 connectPort, tConnectionFactroy tConnectionFactoryFunc);
	~TCPConnector();

	Connection* Connect();
};

