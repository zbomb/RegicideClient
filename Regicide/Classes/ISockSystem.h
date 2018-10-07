#pragma once
#include "FSock.h"


enum ISOCK_TYPE
{
	ISOCK_TCP,
	ISOCK_UDP
};

class  ISockSystem
{
	
private:

	static ISockSystem* ActiveSystem;

public:
	virtual ~ISockSystem() {}
	virtual FSock* CreateSocket( ISOCK_TYPE SocketType ) = 0;
	virtual void DestroySocket( FSock* Sck ) = 0;

protected:
	virtual void DestroySystem() = 0;

public:
	static ISockSystem* Get();
	static void Close();

};