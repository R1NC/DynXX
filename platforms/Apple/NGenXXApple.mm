#import "NGenXXApple.h"

#import "NGenXX.h"
#include <fstream>

#define NSString2CharP(nsstr) [nsstr cStringUsingEncoding:NSUTF8StringEncoding]
#define CharP2NSString(cp) [NSString stringWithCString:cp encoding:NSUTF8StringEncoding]
#define STDStr2NSStr(stdStr) [NSString stringWithCString:stdStr.c_str() encoding:NSUTF8StringEncoding]

static const char *cParamsJson = "{\"url\":\"https://rinc.xyz\", \"params\":\"p0=1&p1=2&p2=3\", \"method\":0, \"header_v\":[\"Cache-Control: no-cache\"], \"header_c\": 1, \"timeout\":66666}";

@interface NGenXXApple () {
    void *_db_conn;
    void *_kv_conn;
}
@end

@implementation NGenXXApple

- (instancetype)init {
    if (self = [super init]) {
        if (!ngenxx_init(NSString2CharP(self.root))) {
            NSLog(@"!!! SDK INIT FAILED !!!");
        }
    }
    return self;
}

- (NSString*)root {
    return NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, true).firstObject;
}

- (void)dealloc {
    ngenxx_store_sqlite_close(_db_conn);
    ngenxx_store_kv_close(_kv_conn);
    ngenxx_release();
}

- (void)testHttpL {
    //const char * cRsp = ngenxx_net_http_request("https://rinc.xyz", "", 0, NULL, 0, 5555);
    const char *cLuaPath = NSString2CharP([NSBundle.mainBundle.resourcePath stringByAppendingPathComponent:@"biz.lua"]);
    if (!ngenxx_L_loadF(cLuaPath)) {
        NSLog(@"!!! LOAD LUA FAILED !!!");
    }
    const char * cRsp = ngenxx_L_call("lNetHttpRequest", cParamsJson);
    NSLog(@"%s", cRsp);
}

- (void)testHttpJ {
    NSString *jsbPath0 = [NSBundle.mainBundle.resourcePath stringByAppendingPathComponent:@"NGenXX.qjsb"];
    NSString *jsbPath1 = [NSBundle.mainBundle.resourcePath stringByAppendingPathComponent:@"biz.qjsb"];
    NSData *jsbData0 = [NSData dataWithContentsOfFile:jsbPath0];
    NSData *jsbData1 = [NSData dataWithContentsOfFile:jsbPath1];
    auto bytes0 = reinterpret_cast<unsigned char *>(const_cast<void *>(jsbData0.bytes));
    auto len0 = static_cast<unsigned int>(jsbData0.length);
    auto bytes1 = reinterpret_cast<unsigned char *>(const_cast<void *>(jsbData1.bytes));
    auto len1 = static_cast<unsigned int>(jsbData1.length);
    if (ngenxx_J_loadB(bytes0, len0) && ngenxx_J_loadB(bytes1, len1)) {
        auto res = ngenxx_J_call("jTestNetHttpReq", "https://rinc.xyz");
        NSLog(@"%s", res);
    }
}

- (void)testDB {
    _db_conn = ngenxx_store_sqlite_open("test.db");
    if (!_db_conn) return;
    
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
        long long i = ngenxx_store_sqlite_query_read_column_integer(qrQuery, "i");
        double f = ngenxx_store_sqlite_query_read_column_float(qrQuery, "f");
        NSLog(@"platform:%@ i:%lld f:%f", CharP2NSString(platform), i, f);
    }
    ngenxx_store_sqlite_query_drop(qrQuery);
}

- (void)testKV {
    _kv_conn = ngenxx_store_kv_open("test");
    if (!_kv_conn) return;
    
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

- (void)testCrypto {
    NSString* inStr = @"0123456789qwertyuiopasdfghjklzxcvbnmQWERTYUIOPASDFGHJKLZXCVBNM<>()[]{}|,;:'`~!@#$%^&*-=_+/";
    NSData *inData = [inStr dataUsingEncoding:NSUTF8StringEncoding];
    byte *inBytes = (unsigned char *) inData.bytes;
    size inLen = inData.length;
    NSString * keyStr = @"MNBVCXZLKJHGFDSA";
    NSData *keyData = [keyStr dataUsingEncoding:NSUTF8StringEncoding];
    byte *keyBytes = (unsigned char *) keyData.bytes;
    size keyLen = keyData.length;
    
    size aesEncodedLen;
    const byte *aesEncodedBytes = ngenxx_crypto_aes_encrypt(inBytes, inLen, keyBytes, keyLen, &aesEncodedLen);
    if (aesEncodedBytes && aesEncodedLen > 0) {
        size aesDecodedLen;
        const byte *aesDecodedBytes = ngenxx_crypto_aes_decrypt(aesEncodedBytes, aesEncodedLen, keyBytes, keyLen, &aesDecodedLen);
        if (aesDecodedBytes && aesDecodedLen > 0);
    }
    
    size ivLen = 12;
    byte ivBytes[ivLen];
    ngenxx_crypto_rand(ivLen, ivBytes);
    size aesgcmTagBits = 15 * 8;
    
    size aesgcmEncodedLen;
    const byte *aesgcmEncodedBytes = ngenxx_crypto_aes_gcm_encrypt(inBytes, inLen, keyBytes, keyLen, ivBytes, ivLen, NULL, 0, aesgcmTagBits, &aesgcmEncodedLen);
    if (aesgcmEncodedBytes && aesgcmEncodedLen > 0) {
        size aesgcmDecodedLen;
        const byte *aesgcmDecodedBytes = ngenxx_crypto_aes_gcm_decrypt(aesgcmEncodedBytes, aesgcmEncodedLen, keyBytes, keyLen, ivBytes, ivLen, NULL, 0, aesgcmTagBits, &aesgcmDecodedLen);
        if (aesgcmDecodedBytes && aesgcmDecodedLen > 0);
    }
}

- (void)testJsonDecoder {
    void *jsonDecoder = ngenxx_json_decoder_init(cParamsJson);
    if (jsonDecoder) {
        void *urlNode = ngenxx_json_decoder_read_node(jsonDecoder, NULL, "url");
        if (urlNode) {
            const char *url = ngenxx_json_decoder_read_string(jsonDecoder, urlNode);
            NSLog(@"url:%s", url);
        }
        void *headerCNode = ngenxx_json_decoder_read_node(jsonDecoder, NULL, "header_c");
        if (headerCNode) {
            double headerC = ngenxx_json_decoder_read_number(jsonDecoder, headerCNode);
            NSLog(@"header_c:%f", headerC);
        }
        void *headerVNode = ngenxx_json_decoder_read_node(jsonDecoder, NULL, "header_v");
        if (headerVNode) {
            void *headerNode = ngenxx_json_decoder_read_child(jsonDecoder, headerVNode);
            while (headerNode) {
                const char *header = ngenxx_json_decoder_read_string(jsonDecoder, headerNode);
                NSLog(@"header:%s", header);
                headerNode = ngenxx_json_decoder_read_next(jsonDecoder, headerNode);
            }
        }
        ngenxx_json_decoder_release(jsonDecoder);
    }
}

- (void)testZip {
    static const size kZBufferSize = 1024;
    NSString *sqlPath = [NSBundle.mainBundle.resourcePath stringByAppendingPathComponent:@"prepare_data.sql"];
    std::ifstream zipIS(NSString2CharP(sqlPath), std::ios::in);
    NSString *zipPath = [self.root stringByAppendingPathComponent:@"xxx.zip"];
    std::ofstream zipOS(NSString2CharP(zipPath), std::ios::out);
    BOOL zipRes = ngenxx_z_cxxstream_zip(NGenXXZipCompressModeDefault, kZBufferSize, NGenXXZFormatGZip, (void *)&zipIS, (void *)&zipOS);
    if (zipRes) {
        NSString *txtPath = [self.root stringByAppendingPathComponent:@"xxx.txt"];
        std::ifstream unzipIS(NSString2CharP(zipPath), std::ios::in);
        std::ofstream unzipOS(NSString2CharP(txtPath), std::ios::out);
        BOOL unzipRes = ngenxx_z_cxxstream_unzip(kZBufferSize, NGenXXZFormatGZip, (void *)&unzipIS, (void *)&unzipOS);
        if (unzipRes);
    }
}

@end
