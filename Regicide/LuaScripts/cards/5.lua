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
--CARD.BackTexture  = "OptionalTexture"

CARD.EnableDeckHooks = true;
CARD.EnableHandHooks = false;
CARD.EnablePlayHooks = true;
CARD.EnableDeadHooks = false;

CARD.Hooks = {};
CARD.Hooks.TestHook = function()

    print( "=====> TEST HOOK <=====" );

end