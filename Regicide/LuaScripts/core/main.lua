-- Test file
-- Regicide Mobile
include( "core/globals.lua" );

print( "=======> LUA ENTRY POINT <=========" );

local account = reg.cms.GetAccount();
if( account == nil ) then
    prin( "====> No local account!" );
else
    print( "====> Account: %s", account.Username );
    print( "====> Coins: %s", tostring( account.Coins ) );
    print( "====> Email: %s", account.Email );
    print( "====> Verified: %s", tostring( account.Verified ) );

    for Id, Ct in pairs( account.Cards ) do
        print( "====> Card: %s", tostring( Id ) .. " " .. tostring( Ct ) );
    end

    for i, Deck in pairs( account.Decks ) do
        print( "===> Deck: %s", Deck.Name );
        print( "===> Id: %s", tostring( Deck.Id ) );

        for Id, Ct in pairs( Deck.Cards ) do
            print( "=======> Card: %s", tostring( Id ) .. " " .. tostring( Ct ) );
        end
    end

    for i, Achv in pairs( account.Achievements ) do
        print( "===> Achv: %s", tostring( Achv.Id ) );
        print( "=======> State: %s", Achv.State );
        print( "=======> Complete: %s", tostring( Achv.Complete ) );
    end

    print( "====== Finish Account ========" );
end