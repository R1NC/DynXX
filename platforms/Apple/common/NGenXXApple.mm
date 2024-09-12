#import "NGenXXApple.h"

#import "NGenXX.h"

#define NSString2CharP(nsstr) [nsstr cStringUsingEncoding:NSUTF8StringEncoding]
#define CharP2NSString(cp) [NSString stringWithCString:cp encoding:NSUTF8StringEncoding]
#define STDStr2NSStr(stdStr) [NSString stringWithCString:stdStr.c_str() encoding:NSUTF8StringEncoding]

@interface NGenXXApple () {
    void *_sdk;
    void *_db_conn;
    void *_kv_conn;
}
@end

@implementation NGenXXApple

- (instancetype)init {
    if (self = [super init]) {
        _sdk = ngenxx_init(NSString2CharP(self.root));
        const char *cLuaPath = NSString2CharP([NSBundle.mainBundle.resourcePath stringByAppendingPathComponent:@"biz.lua"]);
        ngenxx_L_loadF(_sdk, cLuaPath);
    }
    return self;
}

- (NSString*)root {
    return NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, true).firstObject;
}

- (void)dealloc {
    ngenxx_store_sqlite_close(_db_conn);
    ngenxx_store_kv_close(_kv_conn);
    ngenxx_release(_sdk);
}

- (void)testHttpL {
    static const char *cParams = "{\"url\":\"https://rinc.xyz\", \"params\":\"p0=1&p1=2&p2=3\", \"method\":0, \"headers_v\":[\"Cache-Control: no-cache\"], \"headers_c\": 1, \"timeout\":6666}";
    const char * cRsp = ngenxx_L_call(_sdk, "lNetHttpRequest", cParams);
    NSLog(@"%s", cRsp);
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

- (void)testKV {
    _kv_conn = ngenxx_store_kv_open(_sdk, "test");
    ngenxx_store_kv_write_string(_kv_conn, "s", "NGenXX");
    NSLog(@"%s", ngenxx_store_kv_read_string(_kv_conn, "s"));
    ngenxx_store_kv_write_integer(_kv_conn, "i", 1234567890);
    NSLog(@"%lld", ngenxx_store_kv_read_integer(_kv_conn, "i"));
    ngenxx_store_kv_write_float(_kv_conn, "f", 3.1415926535);
    NSLog(@"%f", ngenxx_store_kv_read_float(_kv_conn, "f"));
}

- (void)testDeviceInfo {
    int deviceType = ngenxx_device_type();
    const char *deviceName = ngenxx_device_name();
    const char *osv = ngenxx_device_os_version();
    int arch = ngenxx_device_cpu_arch();
    NSLog(@"deviceType:%d deviceName:%s OS:%s arch:%d", deviceType, deviceName, osv, arch);
}

@end
