//
//  ViewController.mm
//  NGenXX
//
//  Created by Rinc on 2024/8/21.
//

#import "ViewController.h"
#import "NGenXX.h"

#define NSString2CharP(nsstr) [nsstr cStringUsingEncoding:NSUTF8StringEncoding]
#define CharP2NSString(cp) [NSString stringWithCString:cp encoding:NSUTF8StringEncoding]
#define STDStr2NSStr(stdStr) [NSString stringWithCString:stdStr.c_str() encoding:NSUTF8StringEncoding]

@interface ViewController () {
    void *_handle;
}
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

    _handle = ngenxx_init();
    const char *cLuaPath = NSString2CharP([NSBundle.mainBundle.resourcePath stringByAppendingPathComponent:@"biz.lua"]);
    ngenxx_L_loadF(_handle, cLuaPath);
    
    tv.text = self.output;
}

- (void)dealloc {
    ngenxx_release(_handle);
}

- (NSString*)output {
    NSString *s = @"";

    static const char *cParams = "{\"url\":\"https://rinc.xyz\", \"params\":\"p0=1&p1=2&p2=3\", \"method\":1, \"headers\":[\"Accept-Encoding: gzip, deflate\", \"Cache-Control: no-cache\"], \"timeout\":6666}";
    const char * cRsp = ngenxx_L_call(_handle, "lNetHttpReq", cParams);
    if (cRsp) s = [s stringByAppendingFormat:@"%@", CharP2NSString(cRsp)];

    return s;
}

@end
