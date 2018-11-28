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
CARD.LargeTexture   = "LargeCard.png";

CARD.EnableDeckHooks = false;
CARD.EnableHandHooks = false;
CARD.EnablePlayHooks = false;
CARD.EnableDeadHooks = false;

CARD.Abilities = {};

-- Test Ability
CARD.Abilities[ 1 ] = {};
CARD.Abilities[ 1 ].Name            = "Draw Card";
CARD.Abilities[ 1 ].Description     = "Draw a card";
CARD.Abilities[ 1 ].ManaCost        = 2;
CARD.Abilities[ 1 ].StaminaCost     = 1;
CARD.Abilities[ 1 ].PreCheck = function( thisCard )

    return Game.IsCardTurn( thisCard ) and thisCard:OnField();

end

CARD.Abilities[ 1 ].OnTrigger = function( thisCard )

    Action.DrawCard( thisCard:GetOwner(), ACTION_SERIAL )

end

CARD.Abilities[ 2 ] = {};
CARD.Abilities[ 2 ].Name        = "Blind Smite";
CARD.Abilities[ 2 ].Description = "Deal damage equal to this cards power to a random enemy";
CARD.Abilities[ 2 ].ManaCost    = 0;
CARD.Abilities[ 2 ].StaminaCost = 3;
CARD.Abilities[ 2 ].PreCheck = function( thisCard )

    -- Must be player's turn, and this card must be on the field
    if( !Game.IsCardTurn( thisCard ) or !thisCard:OnField() ) then return false end

    -- Opponent must have at least one card on the field
    local Opponent = Game.GetOpponent( thisCard );
    if not Opponent then return false end

    local Field = Opponent:GetField();
    if not Field then return false end

    return Field:GetCount() > 0;

end

CARD.Abilities[ 2 ].OnTrigger = function( thisCard )

    -- Damage random opponent card
    local Opponent = Game.GetOpponent( thisCard );
    local Field = Opponent:GetField();

    if( Field:GetCount() <= 0 ) then return end

    math.randomseed( tonumber( tostring( os.time() ):reverse():sub( 1,6 ) ) );

    local RandIndex = -1;
    while( !Field:IndexValid( RandIndex ) ) do 
        RandIndex = math.random( 0, Field:GetCount() - 1 );
    end

    local TargetCard = Field:GetIndex( RandIndex );
    Action.DamageCard( TargetCard, thisCard, thisCard:GetPower(), ACTION_SERIAL );

end

-- Passive Ability Hooks
CARD.Hooks = {};