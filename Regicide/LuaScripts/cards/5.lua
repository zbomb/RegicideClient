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
CARD.Abilities[ 1 ].PreCheck = function( Context )

    local State = Context:GetState();
    print( "=========> %d", Context:GetState().Position );
    return Context:IsTurn() and Context:GetState().Position == POSITION_FIELD;

end

CARD.Abilities[ 1 ].OnTrigger = function( Context )

    -- Draw Card
    Context:DrawCard( Context:GetState().Owner, ACTION_SERIAL );

end

-- TODO: Deprecated
CARD.Abilities[ 1 ].Simulate = function( thisCard )

    local Output = {}

    Output[ 1 ] = {}
    Output[ 1 ].Type        = ACTION_TYPE_DRAW
    Output[ 1 ].TargetType  = ACTION_TARGET_SINGLE;
    Output[ 1 ].Targets     = thisCard:GetOwner();
    Output[ 1 ].Param       = 1

    return Output;

end

CARD.Abilities[ 2 ] = {};
CARD.Abilities[ 2 ].Name        = "Blind Smite";
CARD.Abilities[ 2 ].Description = "Deal damage equal to this cards power to a random enemy";
CARD.Abilities[ 2 ].ManaCost    = 0;
CARD.Abilities[ 2 ].StaminaCost = 3;
CARD.Abilities[ 2 ].PreCheck = function( Context )

    -- Must be player's turn, and this card must be on the field
    if( !Context:IsTurn() or Context:GetState().Position != POSITION_FIELD ) then return false end

    -- Opponent must have at least one card on the field
    local Field = Context:GetField( Context:GetOpponent() );

    return #Field > 0;

end

CARD.Abilities[ 2 ].OnTrigger = function( Context )

    -- Damage random opponent card
    local Field = Context:GetField( Context:GetOpponent() );

    if( #Field <= 0 ) then return end

    math.randomseed( tonumber( tostring( os.time() ):reverse():sub( 1,6 ) ) );

    local RandIndex = math.random( 1, #Field );
    local Target = Field[ RandIndex ];

    Context:DealDamage( Target.Id, Target.Power, ACTION_SERIAL );

end

-- Simulate Function
-- Used by the AI to determine what this ability actually does when making decisions
CARD.Abilities[ 2 ].Simulate = function( thisCard )

    local Opponent = Game.GetOpponent( thisCard );
    local Field = Opponent:GetField();

    local Output = {}

    Output[ 1 ]             = {}
    Output[ 1 ].Type        = ACTION_TYPE_POWER;
    Output[ 1 ].TargetType  = ACTION_TARGET_RANDOM;
    Output[ 1 ].Targets     = Field:GetCards();
    Output[ 1 ].Param       = -thisCard:GetPower();

    return Output;

end

-- Passive Ability Hooks
CARD.Hooks = {};