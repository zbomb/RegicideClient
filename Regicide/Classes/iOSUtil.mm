//
//  iOSUtil.mm
//  Regicide-mobile
//
//  Created by Zachary Berry on 11/1/18.
//

#import <Foundation/Foundation.h>
#import <string>
#include "Utils.h"

/*========================================================
    iOS Platform Specific
    - Used to locate the sandbox directory
==========================================================*/
std::string Regicide::Utils::GetSandboxDirectory()
{
    NSString* HomeDir = NSHomeDirectory();
    return std::string( [HomeDir UTF8String] );
}

bool Regicide::Utils::IsFirstTimeLaunch( int Version )
{
    // Check if app never launched before, and set current version if not, then return true
    if (![[NSUserDefaults standardUserDefaults] boolForKey:@"hasLaunched"])
    {
        [[NSUserDefaults standardUserDefaults] setBool:TRUE forKey:@"hasLaunched"];
        [[NSUserDefaults standardUserDefaults] setInteger: Version forKey:@"launchVersion"];
        [[NSUserDefaults standardUserDefaults] synchronize];
        
        return true;
    }
    else if( Version == 0 )
    {
        // If version == 0, were only checking for a first time launch, and not a new version launch
        return false;
    }
    
    // Check if the last launch version is lower than the current, if so
    // then update the value and return true
    NSInteger LaunchVersion = [[NSUserDefaults standardUserDefaults] integerForKey:@"launchVersion"];
    if( (int) LaunchVersion < Version )
    {
        [[NSUserDefaults standardUserDefaults] setInteger:Version forKey:@"launchVersion"];
        [[NSUserDefaults standardUserDefaults] synchronize];
        
        return true;
    }
    
    return false;
}
