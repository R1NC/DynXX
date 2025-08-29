#import "DynXXApple.h"

#import <DynXX/CXX/DynXX.hxx>
#include <fstream>

#define NSString2CharP(nsstr) [nsstr cStringUsingEncoding:NSUTF8StringEncoding]
#define CharP2NSString(cp) [NSString stringWithCString:cp encoding:NSUTF8StringEncoding]
#define STDStr2NSStr(stdStr) [NSString stringWithCString:stdStr.c_str() encoding:NSUTF8StringEncoding]

static const auto cParamsJson = "{\"url\":\"https://rinc.xyz\", \"params\":\"p0=1&p1=2&p2=3\", \"method\":0, \"header_v\":[\"Cache-Control: no-cache\"], \"timeout\":66666}";

@interface DynXXApple () {
    DynXXSQLiteConnHandle _db_conn;
    DynXXKVConnHandle _kv_conn;
}
@end

@implementation DynXXApple

- (instancetype)init {
    if (self = [super init]) {
        dynxx_log_set_level(DynXXLogLevelDebug);
        if (!dynxx_init(NSString2CharP(self.root))) {
            NSLog(@"!!! SDK INIT FAILED !!!");
        }
    }
    return self;
}

- (NSString*)root {
    return NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, true).firstObject;
}

- (void)dealloc {
   dynxx_sqlite_close(_db_conn);
   dynxx_kv_close(_kv_conn);
   dynxx_release();
}

- (void)testHttpL {
    dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), ^{
        const char *cLuaPath0 = NSString2CharP([NSBundle.mainBundle.resourcePath stringByAppendingPathComponent:@"json.lua"]);
        const char *cLuaPath1 = NSString2CharP([NSBundle.mainBundle.resourcePath stringByAppendingPathComponent:@"DynXX.lua"]);
        const char *cLuaPath2 = NSString2CharP([NSBundle.mainBundle.resourcePath stringByAppendingPathComponent:@"biz.lua"]);
        if (!dynxx_lua_loadF(cLuaPath0)
            || !dynxx_lua_loadF(cLuaPath1)
            || !dynxx_lua_loadF(cLuaPath2)) {
            NSLog(@"!!! LOAD LUA FAILED !!!");
            return;
        }
        auto cRsp = dynxx_lua_call("TestTimer", "http://rinc.xyz");
        NSLog(@"%s", cRsp);
    });
}

- (void)testHttpJ {
    dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), ^{
        /*NSString *jsbPath0 = [NSBundle.mainBundle.resourcePath stringByAppendingPathComponent:@"DynXX.qjsb"];
        NSString *jsbPath1 = [NSBundle.mainBundle.resourcePath stringByAppendingPathComponent:@"biz.qjsb"];
        NSData *jsbData0 = [NSData dataWithContentsOfFile:jsbPath0];
        NSData *jsbData1 = [NSData dataWithContentsOfFile:jsbPath1];
        auto bytes0 = reinterpret_cast<unsigned char *>(const_cast<void *>(jsbData0.bytes));
        auto len0 = static_cast<unsigned int>(jsbData0.length);
        auto bytes1 = reinterpret_cast<unsigned char *>(const_cast<void *>(jsbData1.bytes));
        auto len1 = static_cast<unsigned int>(jsbData1.length);
        if (dynxx_js_loadB(bytes0, len0) && dynxx_js_loadB(bytes1, len1)) {*/
        NSString *jsPath0 = [NSBundle.mainBundle.resourcePath stringByAppendingPathComponent:@"DynXX.js"];
        NSString *jsPath1 = [NSBundle.mainBundle.resourcePath stringByAppendingPathComponent:@"biz.js"];
        if (dynxx_js_loadF(NSString2CharP(jsPath0), false) && dynxx_js_loadF(NSString2CharP(jsPath1), false)) {
            auto res = dynxx_js_call("jTestStoreSQLite", "https://rinc.xyz", false);
            NSLog(@"%s", res);
        }
    });
}

- (void)testDB {
    _db_conn = dynxx_sqlite_open("test.db");
    if (!_db_conn) return;
    
    NSString *sqlPathPrepareData = [NSBundle.mainBundle.resourcePath stringByAppendingPathComponent:@"prepare_data.sql"];
    NSString *sqlPrepareData = [NSString stringWithContentsOfFile:sqlPathPrepareData encoding:NSUTF8StringEncoding error:nil];
    auto bPrepareData = dynxx_sqlite_execute(_db_conn, NSString2CharP(sqlPrepareData));
    if (!bPrepareData) return;
    
    NSString *sqlPathQuery = [NSBundle.mainBundle.resourcePath stringByAppendingPathComponent:@"query.sql"];
    NSString *sqlQuery = [NSString stringWithContentsOfFile:sqlPathQuery encoding:NSUTF8StringEncoding error:nil];
    auto qrQuery = dynxx_sqlite_query_do(_db_conn, NSString2CharP(sqlQuery));
    if (!qrQuery) return;
    
    while (dynxx_sqlite_query_read_row(qrQuery)) {
        auto platform = dynxx_sqlite_query_read_column_text(qrQuery, "platform");
        auto i = dynxx_sqlite_query_read_column_integer(qrQuery, "i");
        auto f = dynxx_sqlite_query_read_column_float(qrQuery, "f");
        NSLog(@"platform:%@ i:%lld f:%f", CharP2NSString(platform), i, f);
    }
    dynxx_sqlite_query_drop(qrQuery);
}

- (void)testKV {
    _kv_conn = dynxx_kv_open("test");
    if (!_kv_conn) return;
    
    dynxx_kv_write_string(_kv_conn, "s", "DynXX");
    NSLog(@"%s", dynxx_kv_read_string(_kv_conn, "s"));
    dynxx_kv_write_integer(_kv_conn, "i", 1234567890);
    NSLog(@"%lld", dynxx_kv_read_integer(_kv_conn, "i"));
    dynxx_kv_write_float(_kv_conn, "f", 3.1415926535);
    NSLog(@"%f", dynxx_kv_read_float(_kv_conn, "f"));
}

- (void)testDeviceInfo {
    auto deviceType = dynxx_device_type();
    auto deviceName = dynxx_device_name();
    auto osv = dynxx_device_os_version();
    auto arch = dynxx_device_cpu_arch();
    NSLog(@"deviceType:%d deviceName:%s OS:%s arch:%d", deviceType, deviceName, osv, arch);
}

- (void)testCrypto {
    NSString* inStr = @"0123456789qwertyuiopasdfghjklzxcvbnmQWERTYUIOPASDFGHJKLZXCVBNM<>()[]{}|,;:'`~!@#$%^&*-=_+/";
    NSData *inData = [inStr dataUsingEncoding:NSUTF8StringEncoding];
    auto inBytes = reinterpret_cast<const unsigned char *>(inData.bytes);
    auto inLen = inData.length;
    NSString * keyStr = @"MNBVCXZLKJHGFDSA";
    NSData *keyData = [keyStr dataUsingEncoding:NSUTF8StringEncoding];
    auto keyBytes = (unsigned char *) keyData.bytes;
    auto keyLen = keyData.length;
    
    size_t aesEncodedLen;
    auto aesEncodedBytes = dynxx_crypto_aes_encrypt(inBytes, inLen, keyBytes, keyLen, &aesEncodedLen);
    if (aesEncodedBytes && aesEncodedLen > 0) {
        auto aes_encoded_hex = dynxx_coding_hex_bytes2str(aesEncodedBytes, aesEncodedLen);
        NSLog(@"AES Encoded:%s", aes_encoded_hex);
        size_t aesDecodedLen;
        auto aesDecodedBytes = dynxx_crypto_aes_decrypt(aesEncodedBytes, aesEncodedLen, keyBytes, keyLen, &aesDecodedLen);
        if (aesDecodedBytes && aesDecodedLen > 0) {
            auto aes_decoded = dynxx_coding_bytes2str(aesDecodedBytes, aesDecodedLen);
            NSLog(@"AES Decoded:%s", aes_decoded);
        }
    }
    
    size_t ivLen = 12;
    const auto ivBytes = dynxx_crypto_rand(ivLen);
    size_t aesgcmTagBits = 15 * 8;
    
    size_t aesgcmEncodedLen;
    auto aesgcmEncodedBytes = dynxx_crypto_aes_gcm_encrypt(inBytes, inLen, keyBytes, keyLen, ivBytes, ivLen, nullptr, 0, aesgcmTagBits, &aesgcmEncodedLen);
    if (aesgcmEncodedBytes && aesgcmEncodedLen > 0) {
        auto aes_gcm_encoded_hex = dynxx_coding_hex_bytes2str(aesgcmEncodedBytes, aesgcmEncodedLen);
        NSLog(@"AES GCM Encoded:%s", aes_gcm_encoded_hex);
        size_t aesgcmDecodedLen;
        auto aesgcmDecodedBytes = dynxx_crypto_aes_gcm_decrypt(aesgcmEncodedBytes, aesgcmEncodedLen, keyBytes, keyLen, ivBytes, ivLen, nullptr, 0, aesgcmTagBits, &aesgcmDecodedLen);
        if (aesgcmDecodedBytes && aesgcmDecodedLen > 0) {
            auto aes_decoded = dynxx_coding_bytes2str(aesgcmDecodedBytes, aesgcmDecodedLen);
            NSLog(@"AES GCM Decoded:%s", aes_decoded);
        }
    }
}

- (void)testJsonDecoder {
    auto jsonDecoder = dynxx_json_decoder_init(cParamsJson);
    if (jsonDecoder) {
        auto urlNode = dynxx_json_decoder_read_node(jsonDecoder, nullptr, "url");
        if (urlNode) {
            auto url = dynxx_json_decoder_read_string(jsonDecoder, urlNode);
            NSLog(@"url:%s", url);
        }
        auto methodNode = dynxx_json_decoder_read_node(jsonDecoder, nullptr, "method");
        if (methodNode) {
            auto method = dynxx_json_decoder_read_number(jsonDecoder, methodNode);
            NSLog(@"method:%f", method);
        }
        auto headerVNode = dynxx_json_decoder_read_node(jsonDecoder, nullptr, "header_v");
        if (headerVNode) {
            auto headerNode = dynxx_json_decoder_read_child(jsonDecoder, headerVNode);
            while (headerNode) {
                auto header = dynxx_json_decoder_read_string(jsonDecoder, headerNode);
                NSLog(@"header:%s", header);
                headerNode = dynxx_json_decoder_read_next(jsonDecoder, headerNode);
            }
        }
       dynxx_json_decoder_release(jsonDecoder);
    }
}

- (void)testZip {
    static const size_t kZBufferSize = 1024;
    static const auto format = DynXXZFormatX::GZip;
    static const auto mode = DynXXZipCompressModeX::Default;
    NSString *sqlPath = [NSBundle.mainBundle.resourcePath stringByAppendingPathComponent:@"prepare_data.sql"];
    std::ifstream zipIS(NSString2CharP(sqlPath), std::ios::in);
    NSString *zipPath = [self.root stringByAppendingPathComponent:@"xxx.zip"];
    std::ofstream zipOS(NSString2CharP(zipPath), std::ios::out);
    auto zipRes = dynxxZCxxStreamZip(mode, kZBufferSize, format, &zipIS, &zipOS);
    if (zipRes) {
        NSString *txtPath = [self.root stringByAppendingPathComponent:@"xxx.txt"];
        std::ifstream unzipIS(NSString2CharP(zipPath), std::ios::in);
        std::ofstream unzipOS(NSString2CharP(txtPath), std::ios::out);
        auto unzipRes = dynxxZCxxStreamUnzip(kZBufferSize, format, &unzipIS, &unzipOS);
        if (unzipRes);
    }
}

@end
