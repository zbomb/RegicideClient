/*==========================================================================
        Regicide Mobile - Card
        © 2018 Zachary Berry
===========================================================================*/

CARD.Name = "Test Card 1";
CARD.Power = 5;
CARD.Stamina = 3;
CARD.Mana = 2;

CARD.Texture        = "CardFront.png";
CARD.LargeTexture   = "LargeCard.png";

CARD.EnableDeckHooks = true;
CARD.EnableHandHooks = false;
CARD.EnablePlayHooks = true;
CARD.EnableDeadHooks = false;

CARD.Abilities = {};

-- Test Ability
CARD.Abilities[ 1 ] = {};
CARD.Abilities[ 1 ].Name            = "Test Ability";
CARD.Abilities[ 1 ].Description     = "This is a test description for ability 1";
CARD.Abilities[ 1 ].ManaCost        = 2;
CARD.Abilities[ 1 ].StaminaCost     = 2;
CARD.Abilities[ 1 ].PreCheck = function( thisCard )

    return Game.IsCardTurn( thisCard ) and thisCard:OnField();

end

CARD.Abilities[ 1 ].OnTrigger = function( thisCard )

    print( "=======> ABILITY TRIGGERED" );
    Action.DrawCard( thisCard:GetOwner(), ACTION_SERIAL )

end

CARD.Abilities[ 2 ] = {};
CARD.Abilities[ 2 ].Name        = "Test Ability 2";
CARD.Abilities[ 2 ].Description = "This is another test description for ability 2";
CARD.Abilities[ 2 ].ManaCost    = 1;
CARD.Abilities[ 2 ].StaminaCost = 1;
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
    Action.DamageCard( TargetCard, thisCard, 5, ACTION_SERIAL );

end

-- Passive Ability Hooks
CARD.Hooks = {};