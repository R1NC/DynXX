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

@interface ViewController () {
    void *_sdk;
    void *_db_conn;
}
@end

@implementation ViewController

- (void)viewDidLoad {
    [super viewDidLoad];

    _sdk = ngenxx_init(NSString2CharP(self.root));
    
    [self testLua];
    [self testDB];
}

- (void)dealloc {
    ngenxx_store_sqlite_close(_db_conn);
    ngenxx_release(_sdk);
}

- (NSString*)root {
    return NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, true).firstObject;
}

- (void)testLua {
    NSString *s = @"";

    const char *cLuaPath = NSString2CharP([NSBundle.mainBundle.resourcePath stringByAppendingPathComponent:@"biz.lua"]);
    ngenxx_L_loadF(_sdk, cLuaPath);
    static const char *cParams = "{\"url\":\"https://rinc.xyz\", \"params\":\"p0=1&p1=2&p2=3\", \"method\":0, \"headers_v\":[\"Cache-Control: no-cache\"], \"headers_v\":1, \"timeout\":6666}";
    const char * cRsp = ngenxx_L_call(_sdk, "lNetHttpRequest", cParams);
    if (cRsp) s = [s stringByAppendingFormat:@"%@", CharP2NSString(cRsp)];

    NSLog(@"%@", s);
}

- (void)testDB {
    NSString *dbFile = [self.root stringByAppendingPathComponent:@"test.db"];
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


- (void)setRepresentedObject:(id)representedObject {
    [super setRepresentedObject:representedObject];
}

@end
