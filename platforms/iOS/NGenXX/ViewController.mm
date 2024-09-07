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
    void *_db_conn;
    void *_kv_conn;
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

    _sdk = ngenxx_init(NSString2CharP(self.rootPath));
    const char *cLuaPath = NSString2CharP([NSBundle.mainBundle.resourcePath stringByAppendingPathComponent:@"biz.lua"]);
    ngenxx_L_loadF(_sdk, cLuaPath);
    
    tv.text = self.output;
    
    [self testDB];
    [self testKV];
}

- (void)dealloc {
    ngenxx_store_sqlite_close(_db_conn);
    ngenxx_store_kv_close(_kv_conn);
    ngenxx_release(_sdk);
}

- (NSString*)output {
    NSString *s = @"";

    static const char *cParams = "{\"url\":\"https://rinc.xyz\", \"params\":\"p0=1&p1=2&p2=3\", \"method\":1, \"headers\":[\"Accept-Encoding: gzip, deflate\", \"Cache-Control: no-cache\"], \"timeout\":6666}";
    const char * cRsp = ngenxx_L_call(_sdk, "lNetHttpRequest", cParams);
    if (cRsp) s = [s stringByAppendingFormat:@"%@", CharP2NSString(cRsp)];

    return s;
}

- (NSString*)rootPath {
    return NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, true).firstObject;
}

- (void)testDB {
    NSString *dbFile = [self.rootPath stringByAppendingPathComponent:@"test.db"];
    _db_conn = ngenxx_store_sqlite_open(_sdk, NSString2CharP(dbFile));
    if (_db_conn) {
        NSString *sqlPathPrepareData = [NSBundle.mainBundle.resourcePath stringByAppendingPathComponent:@"prepare_data.sql"];
        NSString *sqlPrepareData = [NSString stringWithContentsOfFile:sqlPathPrepareData encoding:NSUTF8StringEncoding error:NULL];
        bool bPrepareData = ngenxx_store_sqlite_execute(_db_conn, NSString2CharP(sqlPrepareData));
        if (!bPrepareData) return;
        
        NSString *sqlPathQuery = [NSBundle.mainBundle.resourcePath stringByAppendingPathComponent:@"query.sql"];
        NSString *sqlQuery = [NSString stringWithContentsOfFile:sqlPathQuery encoding:NSUTF8StringEncoding error:NULL];
        void *qrQuery = ngenxx_store_sqlite_query_do(_db_conn, NSString2CharP(sqlQuery));
        if (!qrQuery) return;
        while (ngenxx_store_sqlite_query_read_row(qrQuery)) {
            const char* platform = ngenxx_store_sqlite_query_read_column_text(qrQuery, "platform");
            const char* vendor = ngenxx_store_sqlite_query_read_column_text(qrQuery, "vendor");
            NSLog(@"platform:%@ vendor:%@", CharP2NSString(platform), CharP2NSString(vendor));
        }
        ngenxx_store_sqlite_query_drop(qrQuery);
    }
}

- (void)testKV {
    _kv_conn = ngenxx_store_kv_open(_sdk, "test");
    ngenxx_store_kv_write_string(_kv_conn, "s", "NGenXX");
    NSLog(@"%s", ngenxx_store_kv_read_string(_kv_conn, "s"));
    ngenxx_store_kv_write_integer(_kv_conn, "i", 1234567890);
    NSLog(@"%lld", ngenxx_store_kv_read_integer(_kv_conn, "i"));
    ngenxx_store_kv_write_float(_kv_conn, "f", 3.1415926535);
    NSLog(@"%f", ngenxx_store_kv_read_float(_kv_conn, "f"));
}

@end
