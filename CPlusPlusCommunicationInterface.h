//
//  CPlusPlusCommunicationInterface.h
//  OCCPlusPlusDemo
//
//  Created by Pengxuyuan on 2020/10/15.
//

#import <Foundation/Foundation.h>
#import "OCCPlusPlusBridge.h"

/// 处理 C++ 调用 OC 方法的中转类，接口文件
@interface CPlusPlusCommunicationInterface : NSObject

+ (instancetype)sharedInstance;

@property CPlusPlusCallOCFunction call;

//- (int)dosomthing:(void *)param;//这里必须定义，否则在MyObjectDoSomethingWith会报错 unknown method
- (void)handleCPlusPlusEvent:(enum CPlusPlusCallOCFunctionActionType)actionType params:(void *)params;

@end
