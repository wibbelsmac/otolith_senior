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
        [self openStepDataFile];
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
    if(newCount->status & (1 << 31)) {
        self.currentCount = newCount->startTime;
        self.currentTime = [[NSDate alloc] init];
        NSLog(@"Associating count: %d with current time", self.currentCount);
    } else {
        StepObject* stepObjTemp = [[StepObject alloc] init];
        stepObjTemp.startDate = [self getTimeofCountwithInt:newCount->startTime];
        stepObjTemp.endDate = [self getTimeofCountwithInt:newCount->endTime];
        stepObjTemp.steps = newCount->steps;
        [self.stepArray addObject:stepObjTemp];
        NSDateFormatter *DateFormatter=[[NSDateFormatter alloc] init];
        [DateFormatter setDateFormat:@"yyyy-MM-dd hh:mm:ss"];
        NSLog(@"Received %d: steps starting at: %@  ending at: %@", stepObjTemp.steps,[DateFormatter stringFromDate:stepObjTemp.startDate],[DateFormatter stringFromDate:stepObjTemp.endDate]);
    }
        
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
    StepObject * sO = [[StepObject alloc] init];
    sO.steps = 1000000;
    [self.stepArray addObject:sO];
   // [self.stepArray writeToFile:filePath atomically:YES];
}
-(void) writeStepDataFile {
    NSString *filePath = [pathToDocuments() stringByAppendingPathComponent:@"/StepData.txt"];
    if(![self.stepArray writeToFile:filePath atomically:YES]) {
        NSLog(@"ERROR Saving Array!");
    }
}
-(void) pushStep:(StepData*) data {
    
}

// 32.768 kHz. 12 bit prescaler
-(NSDate*) getTimeofCountwithInt:(int) count {
    int secondsAgo = (count - self.currentCount) * 60;
    return [self.currentTime dateByAddingTimeInterval:secondsAgo];
    
}

@end

@implementation StepObject
/*
@interface StepObject : NSObject <NSCoding>
@property (nonatomic, retain) NSDate* startDate;
@property (nonatomic, retain) NSDate* endDate;
@property (nonatomic) int steps;
*/
- (void)encodeWithCoder:(NSCoder *)enCoder {
    [super encodeWithCoder:enCoder];
    [enCoder encodeObject:self.startDate forKey:@"startDate"];
    [enCoder encodeObject:self.endDate forKey:@"endDate"];
    [enCoder encodeObject:self.steps forKey:@"steps"];
}

- (id)initWithCoder:(NSCoder *)aDecoder {
    
    if(self = [super initWithCoder:aDecoder]) {
        self.startDate = [aDecoder decodeObjectForKey:@"startDate"];
        self.endDate = [aDecoder decodeObjectForKey:@"endDate"];
        self.steps = [aDecoder decodeObjectForKey:@"steps"];
    }
    
    return self;
}
@end
