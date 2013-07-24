//
//  StepCounter.m
//  Otolith Controller
//
//  Created by Kevin Avery on 4/28/13.
//  Copyright (c) 2013 SAHA. All rights reserved.
//

#import "StepCounter.h"




@implementation StepCounter

-(id)init
{
    self = [super init];
    if (self)
    {
        [self resetStepCount];
    }
    return self;
}

-(void) resetStepCount
{
    [self setLatestStepCount:0];
    [self setTotalStepCount:0];
}

-(void)updateWithCount: (int)newCount
{
    [self setLatestStepCount:newCount];
    [self setTotalStepCount:([self totalStepCount] + [self latestStepCount])];
}

-(void)updateWithStepStruct: (StepData*)newCount
{
    self.currentTime = StepData->;
}

static NSString *pathToDocuments(void) {
    NSArray *paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
    return [paths objectAtIndex:0];
}
-(void) openStepDataFile {
    NSString *filePath = [pathToDocuments() stringByAppendingPathComponent:@"/StepData.txt"];
    self.stepArray  = [NSMutableArray arrayWithContentsOfFile:filePath];
    if(!self.stepArray) {
         self.stepArray = [[NSMutableArray alloc] init];
    }
}
-(void) writeStepDataFile {
    NSString *filePath = [pathToDocuments() stringByAppendingPathComponent:@"/StepData.txt"];
    [self.stepArray writeToFile:filePath atomically:YES];
}
-(void) pushStep:(StepData*) data {
    
}

@end
