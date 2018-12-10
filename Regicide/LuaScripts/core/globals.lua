--  Regicide Mobile
--  LuaScripts/core/globals.lua
--  Defines some basic globals that are used throughout the lua scripting engine
--  © 2018 Zachary Berry

/////////////////////////////////////////////////////////////////////////
/////// Basic Globals
/////////////////////////////////////////////////////////////////////////

ACTION_ROOT             = 0;
ACTION_SERIAL           = 1;
ACTION_PARALLEL         = 2;
ACTION_SERIAL_PARALLEL  = 3;

POSITION_DECK = 0;
POSITION_HAND = 1;
POSITION_FIELD = 2;
POSITION_KING = 3;
POSITION_GRAVEYARD = 4;
POSITION_NONE = 5;

PLAYER_OWNER = true;
PLAYER_OPPONENT = false;

// Deprecated
ACTION_TYPE_POWER   = 0;
ACTION_TYPE_DRAW    = 1;
ACTION_TYPE_STAMINA = 2;
ACTION_TYPE_KILL    = 3;

ACTION_TARGET_SELF      = 0;
ACTION_TARGET_ANY       = 1;
ACTION_TARGET_LIST      = 2;
ACTION_TARGET_SINGLE    = 3;
ACTION_TARGET_RANDOM    = 4;



/////////////////////////////////////////////////////////////////////////
/////// Print Override
/////////////////////////////////////////////////////////////////////////

function print( str, ... )
    local argc = select( "#", ... );
    if( argc == 0 or type( str ) != "string" ) then
        __print( tostring( str ) );
    else
        __print( string.format( tostring( str ), ... ) );
    end
end

-- Alias for print
function printf( ... )
    print( ... );
end


/////////////////////////////////////////////////////////////////////////
/////// File Loading Overrides
/////////////////////////////////////////////////////////////////////////

function include( file )
    __include( file );
end

function require( file )
    __require( file );
end

/////////////////////////////////////////////////////////////////////////
/////// Type Checking
/////////////////////////////////////////////////////////////////////////

function istable( t )       return type( t ) == "table"     end
function isstring( t )      return type( t ) == "string"    end
function isfunction( t )    return type( t ) == "function"  end
function isnumber( t )      return type( t ) == "number"    end
function isbool( t )        return type( t ) == "bool"      end

function typecheck( ... )

    local lastType = nil;
    for k, v in ipairs( { ... } ) do
        if( k == 1 ) then
            lastType = type( v );
        else
            if( type( v ) != lastType ) then
                return false;
            end
        end
    end

end

/////////////////////////////////////////////////////////////////////////
/////// Hook System
/////////////////////////////////////////////////////////////////////////

local _GAME = {};
hook = {};


function hook.Add( hookEvent, hookName, callback )

    if( _GAME[ hookEvent ] == nil ) then 
        _GAME[ hookEvent ] = {};
    end

    if( _GAME[ hookEvent ][ hookName ] != nil ) then

        print( "Warning! Attempt to overwrite hook '%s'", hookName );
        return;
    end

    _GAME[ hookEvent ][ hookName ] = callback;

end

function hook.Remove( hookEvent, hookName )

    if( _GAME[ hookEvent ] == nil ) then return end
    if( _GAME[ hookEvent ][ hookName ] == nil ) then
        print( "Warning! Attempt to remove non-existant hook '%s'", hookName );
    end

    _GAME[ hookEvent ][ hookName ] = nil;
end

function hook.Call( hookEvent, ... )

    if( _GAME[ hookEvent ] != nil ) then
        for Name, Func in pairs( _GAME[ hookEvent ] ) do
            if( Func( ... ) == false ) then return end
        end
    end

end

function hook.CallEngine( hookEvent, Data )

    if( !isstring( hookEvent ) ) then
        print( "[ERROR] Failed to call engine hook with '%s' argument is not a string!", type( hookEvent ) );
        return false;
    end

    -- Depending on the type, we will have different C++ functions
    -- to call, there is no perfect way to accomplish what we want here

    if( isstring( Data ) ) then
        __c_hook_string( hookEvent, Data );
    elseif( isnumber( Data ) ) then
        __c_hook_number( hookEvent, Data );
    elseif( Data == nil ) then
        __c_hook_null( hookEvent );
    else
        print( "[ERROR] Failed to call engine hook '%s' with argument type '%s' because its unsupported!", hookEvent, type( Data ) );
    end

end

/////////////////////////////////////////////////////////////////////////
/////// Table Copying
/////////////////////////////////////////////////////////////////////////

-- DeepCopy
-- Supports both metatables and nested tables
-- When calling, ignore the 'seen' parameter, its used for recursion
function DeepCopy( obj, seen )

    if type(obj) ~= 'table' then return obj end
    if seen and seen[obj] then return seen[obj] end

    local s = seen or {}
    local res = setmetatable({}, getmetatable(obj))
    s[obj] = res

    for k, v in pairs(obj) do res[DeepCopy(k, s)] = DeepCopy(v, s) end
    return res

end

-- ShallowCopy
-- Doenst support nested tables, or metatables
function ShallowCopy( obj )

  if type(obj) ~= 'table' then return obj end
  local res = {}
  for k, v in pairs(obj) do res[ShallowCopy(k)] = ShallowCopy(v) end
  return res

end