/*==========================================================================
        Regicide Mobile - Card
        © 2018 Zachary Berry
===========================================================================*/

CARD.Name = "The Butt Plunger";
CARD.Power = 4;
CARD.Stamina = 2;
CARD.Mana = 3;

CARD.Texture        = "ButtPlunger.png";
CARD.FullTexture   = "ButtPlunger_Large.png";

CARD.Abilities = {};

-- Test Ability
CARD.Abilities[ 1 ] = {};
CARD.Abilities[ 1 ].Name            = "Smoke Ciggarette";
CARD.Abilities[ 1 ].Description     = "Smoke a cigarette, get cancer and kill random opponent card with second hand smoke";
CARD.Abilities[ 1 ].ManaCost        = 1;
CARD.Abilities[ 1 ].StaminaCost     = 2;
CARD.Abilities[ 1 ].PreCheck = function( Context )

    return Context:IsTurn() and Context:GetState().Position == POSITION_FIELD;

end

CARD.Abilities[ 1 ].OnTrigger = function( Context )

    -- Kill Self
    local State = Context:GetState();
    Context:DealDamage( State.Id, State.Id, State.Power, ACTION_SERIAL );

    -- Kill Random Opponent
    local Field = Context:GetField( Context:GetOpponent() );

    if( #Field <= 0 ) then return end

    math.randomseed( tonumber( tostring( os.time() ):reverse():sub( 1,6 ) ) );

    local RandIndex = math.random( 1, #Field );
    local Target = Field[ RandIndex ];

    Context:DealDamage( Target.Id, State.Id, 5, ACTION_SERIAL );

end

-- Output the effects of triggering this ability so the AI can
-- accuratley make decisions
CARD.Abilities[ 1 ].Simulate = function( thisCard )

    local Output    = {}
    local Opponent  = Game.GetOpponent( thisCard );
    local Field     = Opponent:GetField();

    Output[ 1 ]             = {}
    Output[ 1 ].Type        = ACTION_TYPE_KILL;
    Output[ 1 ].TargetType  = ACTION_TARGET_SELF
    Output[ 1 ].Targets     = nil;
    Output[ 1 ].Param       = nil;

    Output[ 2 ]             = {}
    Output[ 2 ].Type        = ACTION_TYPE_KILL;
    Output[ 2 ].TargetType  = ACTION_TARGET_RANDOM
    Output[ 2 ].Targets     = Field:GetCards();
    Output[ 2 ].Param       = nil;

    return Output;

end

-- Passive Ability Hooks
CARD.Hooks = {};
