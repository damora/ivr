//
//  main.m
//  iOS-client
//
//  Created by Bruce D'Amora on 6/18/14.
//  Copyright (c) 2014 Bruce D'Amora. All rights reserved.
//

#import <UIKit/UIKit.h>

#import "AppDelegate.h"

int main(int argc, char * argv[])
{
    /*
    
    NSUserDefaults *standardDefaults = [NSUserDefaults standardUserDefaults];
    
    NSString *ipaddr = [standardDefaults stringForKey:@"ipaddr"];
    
    NSLog(@"Value of ipaddr = %@", ipaddr);
    NSInteger width = [standardDefaults integerForKey:@"width"];
    NSInteger height = [standardDefaults integerForKey:@"height"];
    NSInteger depth = [standardDefaults integerForKey:@"depth"];
    
  
    fprintf(stderr,"width=%ld, height=%ld,depth=%ld\n", width, height, depth);
    
   */
    @autoreleasepool {
        return UIApplicationMain(argc, argv, nil, NSStringFromClass([AppDelegate class]));
    }
}
