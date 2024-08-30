//
//  ViewController.m
//  SdkEngineDemo
//
//  Created by Rinc on 2024/8/23.
//

#import "ViewController.h"
#import "NGenXX.h"

#define NSString2CharP(nsstr) [nsstr cStringUsingEncoding:NSUTF8StringEncoding]
#define CharP2NSString(cp) [NSString stringWithCString:cp encoding:NSUTF8StringEncoding]
#define STDStr2NSStr(stdStr) [NSString stringWithCString:stdStr.c_str() encoding:NSUTF8StringEncoding]

@implementation ViewController

- (void)viewDidLoad {
    [super viewDidLoad];

    NSString *s = @"";
    void* lstate = ngenxx_L_create();
    const char *cLuaPath = NSString2CharP([NSBundle.mainBundle.resourcePath stringByAppendingPathComponent:@"biz.lua"]);
    int ret = ngenxx_L_loadF(lstate, cLuaPath);
    if (ret == 0) {
        static const char *cParams = "{\"url\":\"https://rinc.xyz\", \"params\":\"\"}";
        const char * cRsp = ngenxx_L_call(lstate, "lNetHttpReq", cParams);
        if (cRsp) s = [s stringByAppendingFormat:@"%@", CharP2NSString(cRsp)];
    }
    ngenxx_L_destroy(lstate);
    
    NSLog(@"%@", s);
}


- (void)setRepresentedObject:(id)representedObject {
    [super setRepresentedObject:representedObject];
}


@end
