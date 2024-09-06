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
    void *_sdk;
    void *_conn;
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

    _sdk = ngenxx_init();
    const char *cLuaPath = NSString2CharP([NSBundle.mainBundle.resourcePath stringByAppendingPathComponent:@"biz.lua"]);
    ngenxx_L_loadF(_sdk, cLuaPath);
    
    tv.text = self.output;
}

- (void)dealloc {
    ngenxx_release(_sdk);
    ngenxx_store_sqlite_close(_conn);
}

- (NSString*)output {
    NSString *s = @"";

    static const char *cParams = "{\"url\":\"https://rinc.xyz\", \"params\":\"p0=1&p1=2&p2=3\", \"method\":1, \"headers\":[\"Accept-Encoding: gzip, deflate\", \"Cache-Control: no-cache\"], \"timeout\":6666}";
    const char * cRsp = ngenxx_L_call(_sdk, "lNetHttpRequest", cParams);
    if (cRsp) s = [s stringByAppendingFormat:@"%@", CharP2NSString(cRsp)];
    
    [self testDB];

    return s;
}

- (void)testDB {
    NSString *dbDir = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, true).firstObject;
    NSString *dbFile = [dbDir stringByAppendingPathComponent:@"xxx.db"];
    _conn = ngenxx_store_sqlite_open(_sdk, NSString2CharP(dbFile));
    if (_conn) {
        NSString *sqlPathPrepareTable = [NSBundle.mainBundle.resourcePath stringByAppendingPathComponent:@"prepare_table.sql"];
        NSString *sqlPrepareTable = [NSString stringWithContentsOfFile:sqlPathPrepareTable encoding:NSUTF8StringEncoding error:NULL];
        void *qrPrepareTable = ngenxx_store_sqlite_query_exe(_conn, NSString2CharP(sqlPrepareTable));
        if (!qrPrepareTable) return;
        ngenxx_store_sqlite_query_drop(qrPrepareTable);
        
        NSString *sqlPathPrepareData = [NSBundle.mainBundle.resourcePath stringByAppendingPathComponent:@"prepare_data.sql"];
        NSString *sqlPrepareData = [NSString stringWithContentsOfFile:sqlPathPrepareData encoding:NSUTF8StringEncoding error:NULL];
        void *qrPrepareData = ngenxx_store_sqlite_query_exe(_conn, NSString2CharP(sqlPrepareData));
        if (!qrPrepareData) return;
        ngenxx_store_sqlite_query_drop(qrPrepareData);
        
        NSString *sqlPathQuery = [NSBundle.mainBundle.resourcePath stringByAppendingPathComponent:@"query.sql"];
        NSString *sqlQuery = [NSString stringWithContentsOfFile:sqlPathQuery encoding:NSUTF8StringEncoding error:NULL];
        void *qrQuery = ngenxx_store_sqlite_query_exe(_conn, NSString2CharP(sqlQuery));
        if (!qrQuery) return;
        if (qrQuery) {
            while (ngenxx_store_sqlite_query_read_row(qrQuery)) {
                const char* platform = ngenxx_store_sqlite_query_read_column_text(qrQuery, "platform");
                const char* vendor = ngenxx_store_sqlite_query_read_column_text(qrQuery, "vendor");
                NSLog(@"platform:%@ vendor:%@", CharP2NSString(platform), CharP2NSString(vendor));
            }
        }
        ngenxx_store_sqlite_query_drop(qrQuery);
    }
}

@end
