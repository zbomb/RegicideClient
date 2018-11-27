/*==========================================================================
        Regicide Mobile - Card
        © 2018 Zachary Berry
===========================================================================*/

CARD.Name = "The Butt Plunger";
CARD.Power = 4;
CARD.Stamina = 2;
CARD.Mana = 3;

CARD.Texture        = "ButtPlunger.png";
CARD.LargeTexture   = "ButtPlunger_Large.png";

CARD.EnableDeckHooks = false;
CARD.EnableHandHooks = false;
CARD.EnablePlayHooks = true;
CARD.EnableDeadHooks = false;

CARD.Abilities = {};

-- Test Ability
CARD.Abilities[ 1 ] = {};
CARD.Abilities[ 1 ].Name            = "Smoke Ciggarette";
CARD.Abilities[ 1 ].Description     = "Smoke a cigarette, get cancer and kill random opponent card with second hand smoke";
CARD.Abilities[ 1 ].ManaCost        = 1;
CARD.Abilities[ 1 ].StaminaCost     = 2;
CARD.Abilities[ 1 ].PreCheck = function( thisCard )

    return Game.IsCardTurn( thisCard ) and thisCard:OnField();

end

CARD.Abilities[ 1 ].OnTrigger = function( thisCard )

    -- Kill Self
    Action.DamageCard( thisCard, thisCard, thisCard:GetPower(), ACTION_SERIAL );

    -- Kill Random Opponent
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
