#pragma once
#include "FSock.h"
#include "ISockSystem.h"
#include <asio.hpp>
#include <functional>

using asio::ip::tcp;
using namespace std::placeholders;

class FSockWinTCP : public FSock
{
	enum AddressType { NotSet, Hostname, Endpoint };

private:

	asio::io_context* LinkedContext;
	std::shared_ptr< tcp::socket > InternalSock;
	tcp::endpoint RemoteEndpoint;
	tcp::resolver::iterator EndpointIterator;
	AddressType AddressMode;
	std::shared_ptr< std::thread > AsyncThread;

public:

	FSockWinTCP( asio::io_context* inContext );
	
	// FSocket Interface
	virtual bool SetRemoteAddress( std::string Address, unsigned int Port, bool bIsHostName = false ) override;
	virtual bool Connect() override;
	virtual bool IsConnected() const override;
	virtual unsigned int GetBytesAvailable() const override;
	virtual unsigned int Receive( std::vector<uint8>& OutBuffer, FSockError& OutError = FSockError() ) override;
	virtual unsigned int Send( std::vector< uint8 > Buffer, FSockError& OutError = FSockError() ) override;
	virtual void ConnectAsync( std::function< void( FSockError ) > Callback ) override;
	virtual void ReceiveAsync( std::vector<uint8>& Buffer, std::function< void( FSockError, unsigned int, std::vector< uint8 >& ) > Callback ) override;
	virtual void SendAsync( std::vector< uint8 > Buffer, std::function< void( FSockError, unsigned int ) > Callback ) override;
	virtual void Shutdown( FSock::ShutdownDir Direction ) override;
	virtual void Close() override;

};


class FSockWinUDP : public FSock
{
	
};


class FSockSystemWin : public ISockSystem
{

private:
	asio::io_context Context;	

public:

	FSockSystemWin();
	virtual FSock* CreateSocket( ISOCK_TYPE Type ) override;
	virtual void DestroySocket( FSock* Sck ) override;
	virtual void DestroySystem() override;

};

class FSockWinError : public FSockError
{
	
private:

	asio::error_code Error;

public:
	
	FSockWinError( asio::error_code inErr );
	FSockWinError();

	virtual std::string GetErrorMessage() override;
	virtual operator bool() override;
	virtual bool IsDisconnect() override;

};

