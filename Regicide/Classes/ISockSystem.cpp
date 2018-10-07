
#include "ISockSystem.h"

#if WIN32
#include "FSockWin.h"
#elif OSX
#include "FSockOSX.h"
#elif IOS
#include "FSockIOS.h"
#elif ANDRIOD
#include "FSockAndroid.h"
#endif

ISockSystem* ISockSystem::ActiveSystem;
ISockSystem* ISockSystem::Get()
{
	// Check if we need to create a system
	if( ActiveSystem == nullptr )
	{
#if WIN32
		ActiveSystem = new FSockSystemWin();
#elif OSX
		ActiveSystem = new FSockSystemOSX();
#elif IOS
		ActiveSystem = new FSockSystemIOS();
#elif ANDROID
		ActiveSystem = new FSockSystemAndroid();
#endif
	}

	return ActiveSystem;
}

void ISockSystem::Close()
{
	if( ActiveSystem )
	{
		ActiveSystem->DestroySystem();
	}
}
