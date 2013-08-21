//
//  StepCounter.h
//  Otolith Controller
//
//  Created by Kevin Avery on 4/28/13.
//  Copyright (c) 2013 SAHA. All rights reserved.
//

#import <Foundation/Foundation.h>

@interface StepCounter : NSObject

typedef struct {
    uint32_t startTime;
    uint32_t endTime;
    uint32_t steps;
    uint32_t status;
} StepData;



@property (assign) int latestStepCount;
@property (assign) int totalStepCount;
@property (nonatomic, retain) NSMutableArray* stepArray;
@property (nonatomic) int currentCount;
@property (nonatomic) NSDate* currentTime;
-(void)resetStepCount;
-(void)updateWithCount: (int)newCount;
-(void) openStepDataFile;
static NSString *pathToDocuments(void);
-(void) openStepDataFile;
-(void) writeStepDataFile;
-(void) pushStep:(StepData*) data;
-(void)updateWithStepStruct: (StepData*)newCount;

@end

@interface StepObject : NSObject
@property (nonatomic, retain) NSDate* startDate;
@property (nonatomic, retain) NSDate* endDate;
@property (nonatomic) int steps;
@end

