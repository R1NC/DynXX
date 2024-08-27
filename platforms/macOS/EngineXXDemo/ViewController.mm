//
//  ViewController.m
//  SdkEngineDemo
//
//  Created by Rinc on 2024/8/23.
//

#import "ViewController.h"
#import "EngineXX.h"

#define NSString2CharP(nsstr) [nsstr cStringUsingEncoding:NSUTF8StringEncoding]
#define CharP2NSString(cp) [NSString stringWithCString:cp encoding:NSUTF8StringEncoding]
#define STDStr2NSStr(stdStr) [NSString stringWithCString:stdStr.c_str() encoding:NSUTF8StringEncoding]

@implementation ViewController

- (void)viewDidLoad {
    [super viewDidLoad];

    NSString *s = @"";
    void* lstate = enginexx_L_create();
    const char *cLuaPath = NSString2CharP([NSBundle.mainBundle.resourcePath stringByAppendingPathComponent:@"biz.lua"]);
    int ret = enginexx_L_loadF(lstate, cLuaPath);
    if (ret == 0) {
        const char * cV = enginexx_L_call(lstate, "lTestGetVersion", NULL);
        if (cV) s = [s stringByAppendingFormat:@"%@\n\n", CharP2NSString(cV)];
        
        static const char *cParams = "{\"url\":\"https://rinc.xyz\", \"params\":\"\"}";
        const char * cRsp = enginexx_L_call(lstate, "lTestHttpReq", cParams);
        if (cRsp) s = [s stringByAppendingFormat:@"%@", CharP2NSString(cRsp)];
    }
    enginexx_L_destroy(lstate);
    
    NSLog(@"%@", s);
}


- (void)setRepresentedObject:(id)representedObject {
    [super setRepresentedObject:representedObject];
}


@end
