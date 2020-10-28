//
//  OCObjectRegisteredObserverHelper.m
//  OCCPlusPlusDemo
//
//  Created by Pengxuyuan on 2020/10/15.
//

#import "OCObjectRegisteredObserverCenter.h"
#import "CPlusPlusCommunicationInterface.h"
#import "OCCPlusPlusBridge.h"

@interface OCObjectRegisteredObserverCenter()

@property (nonatomic, strong) NSMutableArray *observerArray;

@end

@implementation OCObjectRegisteredObserverCenter

+ (void)load {
    NSLog(@"OCObjectRegisteredObserverCenter 开始注册：将 CPlusPlusCommunicationInterface 注册成接受 C++ 消息对象");
    CPlusPlusCommunicationInterface *interface = [CPlusPlusCommunicationInterface sharedInstance];
    void *targetOCObject = (__bridge void*)interface;
    
    GlobleOCTargetObserverCPlusPlus = targetOCObject;
    GlobleTargetCallFunction = interface.call;
}

+ (instancetype)sharedInstance {
    static dispatch_once_t onceToken;
    static OCObjectRegisteredObserverCenter *instance;
    dispatch_once(&onceToken, ^{
        instance = [OCObjectRegisteredObserverCenter new];
    });
    return instance;
}

@end
