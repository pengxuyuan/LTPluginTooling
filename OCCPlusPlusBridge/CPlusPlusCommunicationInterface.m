//
//  CPlusPlusCommunicationInterface.m
//  OCCPlusPlusDemo
//
//  Created by Pengxuyuan on 2020/10/15.
//

#import "CPlusPlusCommunicationInterface.h"

void HandleCPlusPlusEvent(void *target, enum CPlusPlusCallOCFunctionActionType actionType, void *params)
{
//    [(__bridge id)target dosomthing:params];
    [(__bridge id)target handleCPlusPlusEvent:actionType params:params];
}

@interface CPlusPlusCommunicationInterface()

@end

@implementation CPlusPlusCommunicationInterface

+ (instancetype)sharedInstance {
    static dispatch_once_t onceToken;
    static CPlusPlusCommunicationInterface *instance;
    dispatch_once(&onceToken, ^{
        instance = [CPlusPlusCommunicationInterface new];
    });
    return instance;
}

- (instancetype)init
{
    self = [super init];
    if (self) {
        _call = HandleCPlusPlusEvent;
    }
    return self;
}

- (void)dealloc {
    NSLog(@"__FUNCTION__");
}


- (void)handleCPlusPlusEvent:(enum CPlusPlusCallOCFunctionActionType)actionType params:(void *)params {
    if (actionType == ActionTypeExampleMethod) {
        NSLog(@"C++ 调用 OC 方法成功 ActionTypeExampleMethod");
    } else if (actionType == ActionTypeExampleMethod1) {
        NSLog(@"C++ 调用 OC 方法成功 ActionTypeExampleMethod1");
    } else if (actionType == ActionTypeExampleMethod2) {
        NSLog(@"C++ 调用 OC 方法成功 ActionTypeExampleMethod2");
    }
}

@end
