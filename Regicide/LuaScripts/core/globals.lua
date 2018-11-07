--  Regicide Mobile
--  LuaScripts/core/globals.lua
--  Defines some basic globals that are used throughout the lua scripting engine
--  © 2018 Zachary Berry

function print( str, ... )
    -- We need to format the string and call the print function from c++
    local argc = select( "#", ... );
    if( argc == 0 or type( str ) ~= "string" ) then
        __print( tostring( str ) );
    else
        __print( string.format( tostring( str ), ... ) );
    end
end

function printf( ... )
    print( ... );
end

