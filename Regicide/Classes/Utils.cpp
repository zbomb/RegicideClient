//
//  Utils.cpp
//  Regicide
//
//  Created by Zachary Berry on 11/1/18.
//

#include "Utils.h"
#include <memory>
#include "RegicideAPI/Account.h"
#include "external/json/prettywriter.h"

using namespace Regicide;
using namespace rapidjson;

bool Utils::ReadAccount( JsonObject Input, std::shared_ptr< UserAccount > &Output )
{
    // Create empty account info
    Output = std::make_shared< UserAccount >();
    
    if( Input.HasMember( "Info" ) && Input[ "Info" ].IsObject() )
    {
        auto Info = Input[ "Info" ].GetObject();
        if( !ReadAccountInfo( Info, Output->Info ) )
        {
            cocos2d::log( "[RegAPI] Invalid basic info contained within account json!" );
            return false;
        }
    }
    if( Input.HasMember( "Cards" ) && Input[ "Cards" ].IsArray() )
    {
        auto Cards = Input[ "Cards" ].GetArray();
        Input[ "Cards" ].GetArray();
        if( !ReadCards( Cards, Output->Cards ) )
        {
            cocos2d::log( "[RegAPI] Invalid card list contained within account json!" );
            return false;
        }
    }
    if( Input.HasMember( "Decks" ) && Input[ "Decks" ].IsArray() )
    {
        auto Decks = Input[ "Decks" ].GetArray();
        if( !ReadDecks( Decks, Output->Decks ) )
        {
            cocos2d::log( "[RegAPI] Invalid deck list contained within account json!" );
            return false;
        }
    }
    if( Input.HasMember( "Achievements" ) && Input[ "Achievements" ].IsArray() )
    {
        auto Achv = Input[ "Achievements" ].GetArray();
        if( !ReadAchievements( Achv, Output->Achievements ) )
        {
            cocos2d::log( "[RegAPI] Invalid achievement list contained within account json!" );
            return false;
        }
    }
    if( Input.HasMember( "AuthToken" ) && Input[ "AuthToken" ].IsString() )
    {
        Output->AuthToken = Input[ "AuthToken" ].GetString();
    }
    
    return true;
}


bool Utils::WriteAccount( std::shared_ptr<UserAccount> &Input, std::string &Output )
{
    // Were going to serialize the final output to a string
    // If were calling this, its to serialize to file, so we will return a string
    Document AccountDoc;
    AccountDoc.SetObject();
    Value InfoDoc( kObjectType );
    Value CardDoc( kArrayType );
    Value DeckDoc( kArrayType );
    Value AchvDoc( kArrayType );
    
    WriteAccountInfo( Input->Info, InfoDoc, AccountDoc.GetAllocator() );
    WriteCards( Input->Cards, CardDoc, AccountDoc.GetAllocator() );
    WriteDecks( Input->Decks, DeckDoc, AccountDoc.GetAllocator() );
    WriteAchievements( Input->Achievements, AchvDoc, AccountDoc.GetAllocator() );
    
    AccountDoc.AddMember( "Info", InfoDoc, AccountDoc.GetAllocator() );
    AccountDoc.AddMember( "Cards", CardDoc, AccountDoc.GetAllocator() );
    AccountDoc.AddMember( "Decks", DeckDoc, AccountDoc.GetAllocator() );
    AccountDoc.AddMember( "Achievements", AchvDoc, AccountDoc.GetAllocator() );
    AccountDoc.AddMember( "AuthToken", Input->AuthToken, AccountDoc.GetAllocator() );
    
    StringBuffer Buffer;
    PrettyWriter<StringBuffer> Writer( Buffer );
    AccountDoc.Accept( Writer );
    
    Output = Buffer.GetString();
    return true;
}


bool Utils::ReadAccountInfo( JsonObject& Input, AccountInfo& Output )
{
    if( Input.HasMember( "Username" ) && Input[ "Username" ].IsString() )
    {
        Output.Username = Input[ "Username" ].GetString();
    }
    if( Input.HasMember( "Email" ) && Input[ "Email" ].IsString() )
    {
        Output.Email = Input[ "Email" ].GetString();
    }
    if( Input.HasMember( "Coins" ) && Input[ "Coins" ].IsUint64() )
    {
        Output.Coins = Input[ "Coins" ].GetUint64();
    }
    if( Input.HasMember( "DisplayName" ) && Input[ "DisplayName" ].IsString() )
    {
        Output.DisplayName = Input[ "DisplayName" ].GetString();
    }
    if( Input.HasMember( "Verified" ) && Input[ "Verified" ].IsBool() )
    {
        Output.bVerified = Input[ "Verified" ].GetBool();
    }
    
    // No real reason to return false, if the value is there, we read it, if not, then we skip it
    return true;
}


bool Utils::WriteAccountInfo( AccountInfo &Input, Value &Output, MemoryPoolAllocator<>& Alloc )
{
    Output.SetObject();
    Output.AddMember( "Username", Input.Username, Alloc );
    Output.AddMember( "Email", Input.Email, Alloc );
    Output.AddMember( "Coins", Input.Coins, Alloc );
    Output.AddMember( "DisplayName", Input.DisplayName, Alloc );
    Output.AddMember( "Verified", Input.bVerified, Alloc );
    
    return true;
}


bool Utils::ReadCards( JsonArray& Input, std::vector< Card >& Output )
{
    Output.clear(); // TODO: Reevaluate if this needed
    
    for( auto It = Input.Begin(); It != Input.End(); It++ )
    {
        if( It->HasMember( "Id" ) && It->HasMember( "Ct" ) &&
           (*It)[ "Id" ].IsUint() && (*It)[ "Ct" ].IsUint() )
        {
            Card newCard;
            newCard.Id = (uint16_t) (*It)[ "Id" ].GetUint();
            newCard.Ct = (uint16_t) (*It)[ "Ct" ].GetUint();
            
            Output.push_back( newCard );
        }
        else
        {
            // Log the error, we wont break though
            cocos2d::log( "[RegAPI] Invalid card found in account json!" );
        }
    }
    
    return true;
}


bool Utils::WriteCards( std::vector<Card> &Input, Value &Output, MemoryPoolAllocator<> &Allocator )
{
    Output.SetArray();
    
    for( auto& C : Input )
    {
        Value NewCard( kObjectType );
        NewCard.AddMember( "Id", C.Id, Allocator );
        NewCard.AddMember( "Ct", C.Ct, Allocator );
        Output.PushBack( NewCard, Allocator );
    }
    
    return true;
}


bool Utils::ReadDecks( JsonArray& Input, std::vector<Deck> &Output )
{
    Output.clear();
    
    for( auto It = Input.Begin(); It != Input.End(); It++ )
    {
        if( It->HasMember( "Id" ) && It->HasMember( "Name" ) && It->HasMember( "Cards" ) &&
           (*It)[ "Id" ].IsUint() && (*It)[ "Name" ].IsString() && (*It)[ "Cards" ].IsArray() )
        {
            Deck newDeck = Deck();
            newDeck.Id = (*It)[ "Id" ].GetUint();
            newDeck.Name = std::string( (*It)[ "Name" ].GetString(), (*It)[ "Name" ].GetStringLength() );
            
            auto CardList = (*It)[ "Cards" ].GetArray();
            
            for( auto C = CardList.Begin(); C != CardList.End(); C++ ) // lol
            {
                if( C->HasMember( "Id" ) && C->HasMember( "Ct" ) &&
                   (*C)[ "Id" ].IsUint() && (*C)[ "Ct" ].IsUint() )
                {
                    Card newCard;
                    newCard.Id = (uint16_t) (*C)[ "Id" ].GetUint();
                    newCard.Ct = (uint16_t) (*C)[ "Ct" ].GetUint();
                    
                    newDeck.Cards.push_back( newCard );
                }
                else
                {
                    cocos2d::log( "[RegAPI] Invalid card found in deck '%s' while reading account json", newDeck.Name.c_str() );
                }
            }
            
            Output.push_back( newDeck );
        }
        else
        {
            cocos2d::log( "[RegAPI] Invalid deck found in account json" );
        }
    }
    
    return true;
}


bool Utils::WriteDecks( std::vector<Deck> &Input, Value &Output, MemoryPoolAllocator<> &Allocator )
{
    Output.SetArray();
    
    for( auto& D : Input )
    {
        Value NewDeck( kObjectType );
        Value DeckCards( kArrayType );
        
        NewDeck.AddMember( "Id", D.Id, Allocator );
        NewDeck.AddMember( "Name", D.Name, Allocator );
        
        for( auto& C : D.Cards )
        {
            Value NewCard( kObjectType );
            NewCard.AddMember( "Id", C.Id, Allocator );
            NewCard.AddMember( "Ct", C.Ct, Allocator );
            DeckCards.PushBack( NewCard, Allocator );
        }
        
        NewDeck.AddMember( "Cards", DeckCards, Allocator );
        Output.PushBack( NewDeck, Allocator );
    }
    
    return true;
}


bool Utils::ReadAchievements( JsonArray& Input, std::vector<Achievement> &Output )
{
    Output.clear();
    
    for( auto It = Input.Begin(); It != Input.End(); It++ )
    {
        if( It->HasMember( "Id" ) && It->HasMember( "Complete" ) && It->HasMember( "State" ) &&
           (*It)[ "Id" ].IsUint() && (*It)[ "Complete" ].IsBool() && (*It)[ "State" ].IsInt() )
        {
            Achievement newAchv;
            newAchv.Id = (*It)[ "Id" ].GetUint();
            newAchv.Complete = (*It) [ "Complete" ].GetBool();
            newAchv.State = (*It)[ "State" ].GetInt();
            
            Output.push_back( newAchv );
        }
        else
        {
            cocos2d::log( "[RegAPI] Invalid achievement found in account json!" );
        }
    }
    
    return true;
}


bool Utils::WriteAchievements( std::vector<Achievement> &Input, Value &Output, MemoryPoolAllocator<> &Allocator )
{
    Output.SetArray();
    
    for( auto& A : Input )
    {
        Value NewAchv( kObjectType );
        NewAchv.AddMember( "Id", A.Id, Allocator );
        NewAchv.AddMember( "Complete", A.Complete, Allocator );
        NewAchv.AddMember( "State", A.State, Allocator );
        
        Output.PushBack( NewAchv, Allocator );
    }
    
    return true;
}


static std::string CachedRoot;

std::string Utils::GetPlatformDir()
{
    if( CachedRoot.size() == 0 )
    {
#if CC_TARGET_PLATFORM == CC_PLATFORM_IOS
        CachedRoot = GetSandboxDirectory();
#endif
    }
    
    return CachedRoot;
}

static std::string ContentRoot;
std::string Utils::GetContentDir()
{
    if( ContentRoot.size() == 0 )
    {
        std::string Root = CachedRoot.size() > 0 ? CachedRoot : GetPlatformDir();
#if CC_TARGET_PLATFORM == CC_PLATFORM_IOS
        ContentRoot = Root.append( "/Library/Application Support/Content" );
#else
        ContentRoot = Root.append( "/Content" );
#endif
        auto file = cocos2d::FileUtils::getInstance();
        if( !file->isDirectoryExist( ContentRoot ) )
            file->createDirectory( ContentRoot );
    }
    
    return ContentRoot;
}


std::string Utils::ByteString( uint64 Bytes )
{
    if( Bytes < uint64( 1000 ) )
    {
        return Utils::FormatString( "%lu bytes", Bytes );
    }
    if( Bytes < uint64( 1000 ) * 1000 )
    {
        return Utils::FormatString( "%.2f kB", Bytes / 1000.0 );
    }
    if( Bytes < uint64( 1000 ) * 1000 * 1000 )
    {
        return Utils::FormatString( "%.2f MB", Bytes / ( 1000.0 * 1000.0 ) );
    }
    if( Bytes < uint64( 1000 ) * 1000 * 1000 * 1000 )
    {
        return Utils::FormatString( "%.2f GB", Bytes / ( 1000.0 * 1000.0 * 100.0 ) );
    }
    
    return Utils::FormatString( "%.2f TB", Bytes / ( 1000.0 * 1000.0 * 1000.0 * 1000.0 ) );
}
