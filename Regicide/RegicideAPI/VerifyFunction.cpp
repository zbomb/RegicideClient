//
//  VerifyFunction.cpp
//  Regicide-mobile
//
//  Created by Zachary Berry on 11/6/18.
//

#include "API.h"

using namespace Regicide;

VerifyResponse APIClient::VerifyToken()
{
    VerifyResponse Output;
    Output.Result = false;
    
    if( !IsAuthorized() )
    {
        cocos2d::log( "[API] Warning: Attempt to run 'VerifyToken' without authorization!" );
        return Output;
    }
    
    // Perform Syncronous API Call
    Document Response;
    
    auto Result = ExecutePostMethod( "account/verify", Response, Output.StatusCode, true, false );
    if( Result != ExecuteResult::Success )
    {
        cocos2d::log( "[API] VerifyToken API call failed! Check logs for error source. Code: %d", (int) Output.StatusCode );
        return Output;
    }
    
    if( !Response.HasMember( "Result" ) )
    {
        cocos2d::log( "[API] Verify token result json was invalid!" );
    }
    else
    {
        Output.Result = Response[ "Result" ].GetBool();
    }
    
    return Output;
}

bool APIClient::VerifyTokenAsync( std::function<void (VerifyResponse)> Callback )
{
    return ExecutePostMethodAsync( "account/verify", true,
                  [ this, Callback ]( long StatusCode, Document* ResponseBody, std::string Headers )
                  {
                      VerifyResponse Response;
                      if( StatusCode != API_STATUS_SUCCESS )
                      {
                          cocos2d::log( "[API] Async Verify Token call failed! Code: %ld", StatusCode );
                          Response.Result = false;
                      }
                      else
                      {
                          if( !ResponseBody || !ResponseBody->HasMember( "Result" ) )
                          {
                              cocos2d::log( "[API] Verify token result json was invalid!" );
                              Response.Result = false;
                          }
                          else
                          {
                              Response.Result = (*ResponseBody)[ "Result" ].GetBool();
                          }
                      }
                      
                      if( ResponseBody ) { delete ResponseBody; }
                      Response.StatusCode = StatusCode;
                      Callback( Response );
                      
                  } );
}
