#import "NGenXXApple.h"

#import "NGenXX.h"

#define NSString2CharP(nsstr) [nsstr cStringUsingEncoding:NSUTF8StringEncoding]
#define CharP2NSString(cp) [NSString stringWithCString:cp encoding:NSUTF8StringEncoding]
#define STDStr2NSStr(stdStr) [NSString stringWithCString:stdStr.c_str() encoding:NSUTF8StringEncoding]

struct Bytes {
    const byte *data;
    const size len;
};
typedef struct Bytes Bytes;

static const char *cParamsJson = "{\"url\":\"https://rinc.xyz\", \"params\":\"p0=1&p1=2&p2=3\", \"method\":0, \"headers_v\":[\"Cache-Control: no-cache\"], \"headers_c\": 1, \"timeout\":6666}";

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
        void *headersCNode = ngenxx_json_decoder_read_node(jsonDecoder, NULL, "headers_c");
        if (headersCNode) {
            double headersC = ngenxx_json_decoder_read_number(jsonDecoder, headersCNode);
            NSLog(@"headers_c:%f", headersC);
        }
        void *headersVNode = ngenxx_json_decoder_read_node(jsonDecoder, NULL, "headers_v");
        if (headersVNode) {
            void *headerNode = ngenxx_json_decoder_read_child(jsonDecoder, headersVNode);
            while (headerNode) {
                const char *header = ngenxx_json_decoder_read_string(jsonDecoder, headerNode);
                NSLog(@"header:%s", header);
                headerNode = ngenxx_json_decoder_read_next(jsonDecoder, headerNode);
            }
        }
        ngenxx_json_decoder_release(jsonDecoder);
    }
}

#pragma mark Zip

static const size kZBufferSize = 1024;

- (void)testZip {
    NSInputStream *zipInS = [[NSInputStream alloc] initWithFileAtPath:[NSBundle.mainBundle.resourcePath stringByAppendingPathComponent:@"prepare_data.sql"]];
    //[zipInS scheduleInRunLoop:[NSRunLoop currentRunLoop] forMode:NSDefaultRunLoopMode];
    [zipInS open];
    NSOutputStream *zipOutS = [[NSOutputStream alloc] initToFileAtPath:[self.root stringByAppendingPathComponent:@"xx.zip"] append:NO];
    //[zipOutS scheduleInRunLoop:[NSRunLoop currentRunLoop] forMode:NSDefaultRunLoopMode];
    [zipOutS open];
    //[[NSRunLoop currentRunLoop] run];
    [self zipWithInputStream:zipInS outputStream:zipOutS];
    //[zipInS removeFromRunLoop:[NSRunLoop currentRunLoop] forMode:NSDefaultRunLoopMode];
    [zipInS close];
    //[zipOutS removeFromRunLoop:[NSRunLoop currentRunLoop] forMode:NSDefaultRunLoopMode];
    [zipOutS close];
    
    NSInputStream *unzipInS = [[NSInputStream alloc] initWithFileAtPath:[self.root stringByAppendingPathComponent:@"xx.zip"]];
    //[unzipInS scheduleInRunLoop:[NSRunLoop currentRunLoop] forMode:NSDefaultRunLoopMode];
    [unzipInS open];
    NSOutputStream *unzipOutS = [[NSOutputStream alloc] initToFileAtPath:[self.root stringByAppendingPathComponent:@"xx.txt"] append:NO];
    //[unzipOutS scheduleInRunLoop:[NSRunLoop currentRunLoop] forMode:NSDefaultRunLoopMode];
    [unzipOutS open];
    [self unzipWithInputStream:unzipInS outputStream:unzipOutS];
    //[unzipInS removeFromRunLoop:[NSRunLoop currentRunLoop] forMode:NSDefaultRunLoopMode];
    [unzipInS close];
    //[unzipOutS removeFromRunLoop:[NSRunLoop currentRunLoop] forMode:NSDefaultRunLoopMode];
    [unzipOutS close];
}

- (BOOL)zipWithInputStream:(NSInputStream*)inStream outputStream:(NSOutputStream*)outStream {
    void *zip = ngenxx_z_zip_init(-1, kZBufferSize);
    BOOL res = [self zProcessWithBufferSize:kZBufferSize inputStream:inStream outputStream:outStream
                                 inputF:^size(byte *inBuffer, size inLen, BOOL inputFinished) {
        return ngenxx_z_zip_input(zip, inBuffer, inLen, inputFinished);
    } processDoF:^const Bytes{
        size outLen;
        const byte *outBytes = ngenxx_z_zip_process_do(zip, &outLen);
        return {.data = outBytes, .len = outLen};
    } processFinishedF:^BOOL{
        return ngenxx_z_zip_process_finished(zip);
    }];
    ngenxx_z_zip_release(zip);
    return res;
}

- (BOOL)unzipWithInputStream:(NSInputStream*)inStream outputStream:(NSOutputStream*)outStream {
    void *unzip = ngenxx_z_unzip_init(kZBufferSize);
    BOOL res = [self zProcessWithBufferSize:kZBufferSize inputStream:inStream outputStream:outStream
                                 inputF:^size(byte *inBuffer, size inLen, BOOL inputFinished) {
        return ngenxx_z_unzip_input(unzip, inBuffer, inLen, inputFinished);
    } processDoF:^const Bytes{
        size outLen;
        const byte *outBytes = ngenxx_z_unzip_process_do(unzip, &outLen);
        return {.data = outBytes, .len = outLen};
    } processFinishedF:^BOOL{
        return ngenxx_z_unzip_process_finished(unzip);
    }];
    ngenxx_z_unzip_release(unzip);
    return res;
}

- (BOOL)zProcessWithBufferSize:(size)bufferSize
                   inputStream:(NSInputStream*)inStream
                  outputStream:(NSOutputStream*)outStream
                        inputF:(size(^)(byte *inBuffer, size inLen, BOOL inputFinished))inputF
                    processDoF:(const Bytes(^)())processDoF
              processFinishedF:(BOOL(^)())processFinishedF {
    byte inBuffer[bufferSize];

    BOOL inputFinished;
    do {
        size inLen = (size)[inStream read:inBuffer maxLength:bufferSize];
        inputFinished = inLen < bufferSize;
        int ret = inputF(inBuffer, inLen, inputFinished);
        if (ret == 0L) {
            return NO;
        }

        BOOL processFinished;
        do {
            const Bytes outBytes = processDoF();
            if (outBytes.data == NULL) {
                return NO;
            }
            processFinished = processFinishedF();

            [outStream write:outBytes.data maxLength:outBytes.len];
        } while(!processFinished);
    } while (!inputFinished);

    return YES;
}

@end
