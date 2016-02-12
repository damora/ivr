//
//  ViewController.h
//  iOS-client
//
//  Created by Bruce D'Amora on 6/18/14.
//  Copyright (c) 2014 Bruce D'Amora. All rights reserved.
//

#import <UIKit/UIKit.h>
#import <GLKit/GLKit.h>

@interface ViewController : GLKViewController
@property (strong, nonatomic) IBOutlet UISegmentedControl *ToolbarButtons;
- (IBAction)indexChanged:(id)sender;

@end
