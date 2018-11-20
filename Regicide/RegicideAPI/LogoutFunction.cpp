//
//    LogoutFunction.cpp
//    Regicide Mobile
//
//    Created: 11/6/18
//    Updated: 11/20/18
//
//    © 2018 Zachary Berry, All Rights Reserved
//

#include "API.hpp"

using namespace Regicide;


LogoutResponse APIClient::Logout()
{
    LogoutResponse Output;
    Output.Result = LogoutResult::Error;
    
    if( !IsAuthorized() )
    {
        cocos2d::log( "[API] Warning: Attempt to run 'Logout' without authorization!" );
        return Output;
    }
    
    // Perform Syncronous API Call
    Document Response;
    
    auto Result = ExecutePostMethod( "account/logout", Response, Output.StatusCode, true, false );
    if( Result != ExecuteResult::Success )
    {
        cocos2d::log( "[API] Logout API call failed! Check logs for error source. Code: %d", (int) Output.StatusCode );
        return Output;
    }
    
    if( !Response.HasMember( "Result" ) )
    {
        cocos2d::log( "[API] Logout result json was invalid!" );
    }
    else
    {
        Output.Result = LogoutResult( Response[ "Result" ].GetInt() );
    }
    
    return Output;
}


bool APIClient::LogoutAsync( std::function<void (LogoutResponse)> Callback )
{
    return ExecutePostMethodAsync( "account/logout", true,
                                  [ this, Callback ]( long StatusCode, Document* ResponseBody, std::string Headers )
                                  {
                                      LogoutResponse Response;
                                      if( StatusCode != API_STATUS_SUCCESS )
                                      {
                                          cocos2d::log( "[API] Async Verify Token call failed! Code: %ld", StatusCode );
                                          Response.Result = LogoutResult::Error;
                                      }
                                      else
                                      {
                                          if( !ResponseBody || !ResponseBody->HasMember( "Result" ) )
                                          {
                                              cocos2d::log( "[API] Verify token result json was invalid!" );
                                              Response.Result = LogoutResult::Error;
                                          }
                                          else
                                          {
                                              Response.Result = LogoutResult( (*ResponseBody)[ "Result" ].GetInt() );
                                          }
                                      }
                                      
                                      if( ResponseBody ) { delete ResponseBody; }
                                      Response.StatusCode = StatusCode;
                                      Callback( Response );
                                      
                                  } );
}
