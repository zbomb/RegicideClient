#pragma once
#include <string>
#include <vector>
#include <functional>

typedef unsigned char uint8;

class FSockError
{

public:

	virtual std::string GetErrorMessage() { return "Not Set"; }
	virtual operator bool() { return false; }
	virtual bool IsDisconnect() { return false; }

};

class FSock
{
	
public:
	
	enum ShutdownDir { SEND, RECV, BOTH };

	virtual bool SetRemoteAddress( std::string Address, unsigned int Port, bool bIsHostName = false ) = 0;
	virtual bool Connect() = 0;
	virtual bool IsConnected() const = 0;
	virtual unsigned int GetBytesAvailable() const = 0;
	virtual unsigned int Receive( std::vector< uint8 >& OutBuffer, FSockError& OutError = FSockError() ) = 0;
	virtual unsigned int Send( std::vector< uint8 > Buffer, FSockError& OutError = FSockError() ) = 0;
	virtual void ConnectAsync( std::function< void( FSockError ) > Callback ) = 0;
	virtual void ReceiveAsync( std::vector< uint8 >& Buffer, std::function< void( FSockError, unsigned int, std::vector< uint8 >& ) > Callback ) = 0;
	virtual void SendAsync( std::vector< uint8 > Buffer, std::function< void( FSockError, unsigned int ) > Callback ) = 0;
	virtual void Shutdown( FSock::ShutdownDir Direction ) = 0;
	virtual void Close() = 0;

};