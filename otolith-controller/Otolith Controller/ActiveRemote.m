//
//  ActiveRemote.m
//  Otolith Controller
//
//  Created by Travis Elnicky on 8/22/13.
//  Copyright (c) 2013 SAHA. All rights reserved.
//

#import "ActiveRemote.h"
#import <AFNetworking.h>

@implementation ActiveRemote

+ (void)create:(NSDictionary*) record
{
    NSURL *base_url = [NSURL URLWithString:@"http://codoscopy.com"];
    AFHTTPClient *httpClient = [[AFHTTPClient alloc] initWithBaseURL:base_url];
    [httpClient setParameterEncoding:AFJSONParameterEncoding];
    NSMutableURLRequest *request = [httpClient requestWithMethod:@"POST"
                                                            path:@"steps"
                                                      parameters:@{ @"step": record}];
    
    
    AFJSONRequestOperation *operation =
    [AFJSONRequestOperation JSONRequestOperationWithRequest:request
                                                    success:^(NSURLRequest *request, NSHTTPURLResponse *response, id JSON) {
                                                        NSLog(@"IP Address: %@", [JSON valueForKeyPath:@"origin"]);
                                                    }
                                                    failure:^(NSURLRequest *request, NSHTTPURLResponse *response, NSError *error, id JSON) {
                                                        NSLog(@"%@", [error userInfo]);
                                                    }];
    
    [operation start];
}

+ (void)successfulCreateRequest:(AFHTTPRequestOperation *)operation ResponseObject:(id) responseObject
{
    
}

@end
