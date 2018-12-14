//
//	GameState.cpp
//	Regicide Mobile
//
//	Created: 12/1/18
//	Updated: 12/1/18
//
//	© 2018 Zachary Berry, All Rights Reserved
//

#include "GameStateBase.hpp"
#include "CardEntity.hpp"

using namespace Game;



GameStateBase::GameStateBase()
{
    mState = MatchState::PreMatch;
    pState = PlayerTurn::None;
    tState = TurnState::None;
    
    LocalPlayer.DisplayName = "Unnamed Player";
    Opponent.DisplayName    = "Unnmaed Opponent";
}


void GameStateBase::CopyTo( GameStateBase &Other )
{
    mState = Other.mState;
    pState = Other.pState;
    tState = Other.tState;
    
    LocalPlayer = Other.LocalPlayer;
    Opponent    = Other.Opponent;
}

void GameStateBase::OnCardKilled( CardState* Target )
{
    if( !Target )
        return;
    
    // Find card owner
    PlayerState& Owner = Target->Owner == LocalPlayer.EntId ? LocalPlayer : Opponent;
    
    // Find this card
    std::vector< CardState >::iterator Position = Owner.Field.end();
    for( auto It = Owner.Field.begin(); It != Owner.Field.end(); It++ )
    {
        if( It->EntId == Target->EntId )
        {
            Position = It;
            break;
        }
    }
    
    if( Position == Owner.Field.end() )
    {
        // Failed to find card
        cocos2d::log( "[GameState] Failed to kill card.. card couldnt be found on owners field!" );
        return;
    }
    
    Owner.Graveyard.push_back( *Position );
    Owner.Field.erase( Position );
}



void GameStateBase::ShuffleDeck( PlayerState* Target )
{
    if( !Target )
        return;
    
    // Create new vector
    std::vector< CardState > NewDeck;
    
    // Add cards into new deck in random order
    while( !Target->Deck.empty() )
    {
        auto Index = Target->Deck.size() > 1 ? cocos2d::random( 0, (int)Target->Deck.size() - 1 ) : 0;
        auto It = Target->Deck.begin();
        std::advance( It, Index );
        
        NewDeck.push_back( *It );
        Target->Deck.erase( It );
    }
    
    // Assign new deck to player
    Target->Deck = NewDeck;
}

void GameStateBase::DrawCard( PlayerState* Target, uint32_t Count )
{
    if( !Target || Count <= 0 )
        return;
    
    for( int i = 0; i < Count; i++ )
    {
        if( Target->Deck.size() < 1 )
        {
            // TODO: Call out to win function
            return;
        }
        
        auto It = Target->Deck.begin();
        
        It->Position    = CardPos::HAND;
        It->FaceUp      = false;
        
        Target->Hand.push_back( *It );
        Target->Deck.erase( It );
    }
}

void GameStateBase::TakeMana( PlayerState* Target, CardState* Origin, uint32_t Amount )
{
    if( !Target || !Origin || Amount <= 0 )
        return;
    
    // TODO: Better Damage/Stamina/Mana System
    
    Target->Mana = Amount > Target->Mana ? 0 : Target->Mana - Amount;
}

void GameStateBase::GiveMana( PlayerState* Target, CardState* Origin, uint32_t Amount )
{
    if( !Target || !Origin || Amount <= 0 )
        return;
    
    // TODO: Better Damage/Stamina/Mana System
    
    Target->Mana += Amount;
}

void GameStateBase::DamageKing( PlayerState* Target, CardState* Origin, uint32_t Amount )
{
    if( !Target || !Origin || Amount <= 0 )
        return;
    
    // TODO
    
    Target->Health -= Amount;
    if( Target->Health <= 0 )
    {
        // TODO: Call out to win function
    }
}

void GameStateBase::HealKing( PlayerState* Target, CardState* Origin, uint32_t Amount )
{
    if( !Target || !Origin || Amount <= 0 )
        return;
    
    // TODO
    
    Target->Health += Amount;
}

void GameStateBase::DoCombat( CardState* Target, CardState* Origin, uint32_t Amount, int StaminaChange )
{
    if( !Target || !Origin || Amount <= 0 )
        return;
    
    // TODO
    
}

void GameStateBase::DamageCard( CardState* Target, CardState* Origin, uint32_t Amount )
{
    if( !Target || !Origin || Amount <= 0 )
        return;
    
    // TODO: Better System
    
    Target->Power -= Amount;
    if( Target->Power <= 0 )
    {
        OnCardKilled( Target );
    }
}

void GameStateBase::HealCard( CardState* Target, CardState* Origin, uint32_t Amount )
{
    if( !Target || !Origin || Amount <= 0 )
        return;
    
    // TODO: Better System
    
    Target->Power += Amount;
}

void GameStateBase::TakeStamina( CardState* Target, CardState* Origin, uint32_t Amount )
{
    if( !Target || !Origin || Amount <= 0 )
        return;
    
    // TODO: Better System
    
    Target->Stamina -= Amount;
    if( Target->Stamina <= 0 )
    {
        OnCardKilled( Target );
    }
}

void GameStateBase::GiveStamina( CardState* Target, CardState* Origin, uint32_t Amount )
{
    if( !Target || !Origin || Amount <= 0 )
        return;
    
    // TODO: Better System
    
    Target->Stamina += Amount;
}

bool GameStateBase::PlayCard( PlayerState* Player, CardState* Target )
{
    if( !Player || !Target )
        return false;
    
    return PlayCard( Player, Target->EntId );
}

bool GameStateBase::PlayCard( PlayerState* Player, uint32_t Target )
{
    if( !Player )
        return false;
    
    auto Card = Player->Hand.end();
    for( auto It = Player->Hand.begin(); It != Player->Hand.end(); It++ )
    {
        if( It->EntId == Target )
        {
            Card = It;
            break;
        }
    }
    
    if( Card == Player->Hand.end() )
        return false;
    
    if( Card->ManaCost > Player->Mana )
        return false;
    
    Card->Position = CardPos::FIELD;
    Card->FaceUp = true;
    
    Player->Mana -= Card->ManaCost;
    
    Player->Field.push_back( *Card );
    Player->Hand.erase( Card );
    
    return true;
}

uint32_t GameStateBase::DrawSingle( PlayerState* Target )
{
    if( !Target || Target->Deck.size() <= 0 )
        return 0;
    
    auto It = Target->Deck.begin();
    auto Output = It->EntId;
    
    It->Position = CardPos::HAND;
    It->FaceUp = false;
    
    Target->Hand.push_back( *It );
    Target->Deck.erase( It );
    
    return Output;
}

void GameStateBase::SetStartingPlayer( PlayerTurn In )
{
    pState = In;
    StartingPlayer = In;
    TurnNumber = 1;
}

PlayerTurn GameStateBase::SwitchPlayerTurn()
{
    if( pState == PlayerTurn::LocalPlayer )
    {
        pState = PlayerTurn::Opponent;
    }
    else if( pState == PlayerTurn::Opponent )
    {
        pState = PlayerTurn::LocalPlayer;
    }
    
    if( pState == StartingPlayer )
        TurnNumber++;
    
    return pState;
}

bool GameStateBase::GetCard( uint32_t Id, CardState* Out )
{
    // Check both players, starting with local player
    if( GetCard( Id, &LocalPlayer, Out ) )
        return true;
    if( GetCard( Id, &Opponent, Out ) )
        return true;
    
    return false;
}

bool CheckContainer( uint32_t In, std::vector< CardState >& Container, CardState* Out )
{
    auto Card = Container.end();
    for( auto It = Container.begin(); It != Container.end(); It++ )
    {
        if( It->EntId == In )
        {
            Card = It;
            break;
        }
    }
    
    if( Card != Container.end() )
    {
        Out = std::addressof( *Card );
        return true;
    }
    
    return false;
}

bool GameStateBase::GetCard( uint32_t In, PlayerState* Owner, CardState* Out, bool bFieldOnly /* = false */ )
{
    if( !Owner )
        return false;
    
    if( CheckContainer( In, Owner->Field, Out ) )
        return true;
    if( bFieldOnly )
        return false;
    if( CheckContainer( In, Owner->Hand, Out ) )
        return true;
    if( CheckContainer( In, Owner->Deck, Out ) )
        return true;
    if( CheckContainer( In, Owner->Graveyard, Out ) )
        return true;
    
    return false;
}

void GameStateBase::OnCardAdded( CardState* Card )
{
    if( !Card )
        return;
    
    // Check if this card exists
    for( auto It = CardStore.begin(); It != CardStore.end(); It++ )
    {
        if( *It == Card )
            return;
    }
    
    CardStore.push_back( Card );
}

void GameStateBase::OnCardRemoved( CardState* Card )
{
    if( !Card )
        return;
    
    for( auto It = CardStore.begin(); It != CardStore.end(); It++ )
    {
        if( *It == Card )
        {
            CardStore.erase( It );
            return;
        }
    }
}

PlayerState* GameStateBase::GetCardOwner( CardState* Card )
{
    if( !Card )
        return nullptr;
    
    if( Card->Owner == LocalPlayer.EntId )
        return &LocalPlayer;
    else if( Card->Owner == Opponent.EntId )
        return &Opponent;
    
    return nullptr;
}

PlayerState* GameStateBase::GetCardOpponent( CardState* Card )
{
    if( !Card )
        return nullptr;
    
    if( Card->Owner == LocalPlayer.EntId )
        return &Opponent;
    else if( Card->Owner == Opponent.EntId )
        return &LocalPlayer;
    
    return nullptr;
}

PlayerState* GameStateBase::GetOtherPlayer( PlayerState* Target )
{
    if( !Target )
        return nullptr;
    
    if( Target->EntId == LocalPlayer.EntId )
        return &Opponent;
    else if( Target->EntId == Opponent.EntId )
        return &LocalPlayer;
    
    return nullptr;
}

bool GameStateBase::GetPlayer( uint32_t Id, PlayerState* Output )
{
    Output = nullptr;
    
    if( Id == LocalPlayer.EntId )
        Output = std::addressof( LocalPlayer );
    else if( Id == Opponent.EntId )
        Output = std::addressof( Opponent );
    else
        return false;
    
    return true;
}

bool GameStateBase::PreHook( const std::string& HookName )
{
    // Override, for some additional action queue functionality in AuthState
    // Return true to call hook
    return true;
}

void GameStateBase::PostHook()
{
    // OVerride, for some additional action queue functionality in AuthState
}

void GameStateBase::CallHook( const std::string& HookName )
{
    if( !PreHook( HookName ) )
        return;
    
    // Ensure theres an active action queue, we will create one if needed
    auto& CM = CardManager::GetInstance();
    
    // Loop through all cards in game, and call the hook
    for( auto It = CardStore.begin(); It != CardStore.end(); It++ )
    {
        if( *It )
        {
            // We need to get info for this card
            CardInfo Info;
            PlayerState* Owner = nullptr;
            
            if( CM.GetInfo( (*It)->Id, Info ) && GetPlayer( (*It)->Owner, Owner ) && Owner )
            {
                if( Info.Hooks && ( *Info.Hooks )[ HookName ] && (*Info.Hooks )[ HookName ].isFunction() )
                {
                    // Call hook using current active queue, card owner, and this card
                    try
                    {
                        ( *Info.Hooks )[ HookName ]( *this, *(*It) );
                    }
                    catch( std::exception& e )
                    {
                        cocos2d::log( "[State] Failed to run card hook! %s", e.what() );
                    }
                }
            }
        }
    }
    
    PostHook();
}
