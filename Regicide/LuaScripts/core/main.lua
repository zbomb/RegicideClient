/*==========================================================================
        Regicide Mobile - Script System
        © 2018 Zachary Berry
===========================================================================*/
__require( "core/globals.lua" );

REGMOBILE_VERSION = 0.1;
REGMOBILE_RELEASE = "Beta";
REGMOBILE_UPDATED = "11.7.18";

print( "======================== Regicide Script System ========================" );
print( "=> Initialized!" );
print( "=> Version: %s (%s) - %s", tostring( REGMOBILE_VERSION ), REGMOBILE_RELEASE, REGMOBILE_UPDATED );

hook.Add( "OnLogin", "LoginTest", function( Username )

    print( "The username is %s", Username );

end )

hook.Add( "MainMenuOpen", "InitTest", function()

    print( "Init Test 1!" );

end )

hook.Add( "MainMenuOpen", "InitOther", function()

    print( "Init Test 2!" );

end )

hook.CallEngine( "MainMenuOpen" );