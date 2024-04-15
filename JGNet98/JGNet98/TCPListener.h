#pragma once
class Connection;
/*
	This is a class that performs TCP Listen, Bind, and Accept tasks.
*/
class TCPListener
{
private:
	SOCKET			_listenSocket;
	SOCKADDR_IN		_serverAddrIn;
	tConnectionFactroy2 _connectorFactoryFunc;
	std::vector<JGOverlapped> _acceptOverlapped;

public:
	TCPListener(const char* ip, uint16 port, tConnectionFactroy2 connectorFactoryFunc);
	~TCPListener();

	void Listen();
	void Accept();
	LPFN_ACCEPTEX acceptEx;

	SOCKET GetListenSocket() { return _listenSocket; };
};

