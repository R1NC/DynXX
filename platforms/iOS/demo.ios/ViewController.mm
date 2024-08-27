//
//  ViewController.mm
//  demo.ios
//
//  Created by Rinc on 2024/8/21.
//

#import "ViewController.h"
#import "EngineXX.h"

#define NSString2CharP(nsstr) [nsstr cStringUsingEncoding:NSUTF8StringEncoding]
#define CharP2NSString(cp) [NSString stringWithCString:cp encoding:NSUTF8StringEncoding]
#define STDStr2NSStr(stdStr) [NSString stringWithCString:stdStr.c_str() encoding:NSUTF8StringEncoding]

@interface ViewController ()

@end

@implementation ViewController

- (void)viewDidLoad {
    [super viewDidLoad];
    
    CGFloat topPadding = 0;
    CGFloat bottomPadding = 0;
    for (UIScene *scene in UIApplication.sharedApplication.connectedScenes) {
        if ([scene isKindOfClass:UIWindowScene.class]) {
            UIWindowScene *winScene = (UIWindowScene*)scene;
            id<UIWindowSceneDelegate> winSceneDelegate = (id<UIWindowSceneDelegate>)winScene.delegate;
            topPadding = winSceneDelegate.window.safeAreaInsets.top;
            bottomPadding = winScene.keyWindow.safeAreaInsets.bottom;
        }
    }
    
    UITextView *tv = [[UITextView alloc] initWithFrame:CGRectMake(0, topPadding, self.view.frame.size.width, self.view.frame.size.height - topPadding - bottomPadding)];
    tv.font = [UIFont systemFontOfSize:16.f];
    tv.editable = NO;
    [self.view addSubview:tv];
    
    tv.text = self.output;
}

- (NSString*)output {
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
    return s;
}

@end
