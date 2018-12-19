/*==========================================================================
        Regicide Mobile - Card
        © 2018 Zachary Berry
===========================================================================*/

CARD.Name = "Test Card 1";
CARD.Description = "This is a test description. Hopefully it works well and doesnt look terrible!";
CARD.Power = 3;
CARD.Stamina = 3;
CARD.Mana = 2;

CARD.Texture        = "CardFront.png";
CARD.FullTexture   = "LargeCard.png";

CARD.Abilities = {};

-- Test Ability
CARD.Abilities[ 1 ] = {};
CARD.Abilities[ 1 ].Name            = "Draw Card";
CARD.Abilities[ 1 ].Description     = "Draw a card";
CARD.Abilities[ 1 ].ManaCost        = 2;
CARD.Abilities[ 1 ].StaminaCost     = 1;
CARD.Abilities[ 1 ].PreCheck = function( Client, Card )

    return( Client:IsPlayerTurn( Card:GetOwner() ) and Card:OnField() );

end

CARD.Abilities[ 1 ].OnTrigger = function( Auth, Card )

    if( Auth:IsPlayerTurn( Card:GetOwner() ) and Card:OnField() ) then

        local Owner = Auth:GetOwner( Card );
        if( Owner ) then

            Auth:DrawCard( Owner, 1 );
            return true;

        end
    end

    return false;
    
end

CARD.Abilities[ 2 ] = {};
CARD.Abilities[ 2 ].Name        = "Blind Smite";
CARD.Abilities[ 2 ].Description = "Deal damage equal to this cards power to a random enemy";
CARD.Abilities[ 2 ].ManaCost    = 0;
CARD.Abilities[ 2 ].StaminaCost = 3;
CARD.Abilities[ 2 ].PreCheck = function( Client, Card )

    if( Client:IsPlayerTurn( Card:GetOwner() ) and Card:OnField() ) then

        local Opponent = Client:GetOtherPlayer( Card:GetOwner() );
        if( Opponent ) then

            local Field = Opponent:GetField();
            return( Field and Field:GetCount() > 0 );

        end
    end

    return false;

end

CARD.Abilities[ 2 ].OnTrigger = function( Auth, Card )

    local Field = Auth:GetOpponent( Card ):GetField();
    if( #Field > 0 ) then

        math.randomseed( tonumber( tostring( os.time() ):reverse():sub( 1, 6 ) ) );

        local Index = math.random( 1, #Field );
        local Target = Field[ Index ];

        if( Target ) then

            Auth:DamageCard( Target, Card, Target:GetPower() );
            return true;

        end
    end

    return false;

end

-- Passive Ability Hooks
CARD.Hooks = {};