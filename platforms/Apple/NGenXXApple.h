#import <Foundation/Foundation.h>

@interface NGenXXApple : NSObject

- (instancetype)init;

- (void)testHttpL;

- (void)testDB;

- (void)testKV;

- (void)testDeviceInfo;

- (void)testCrypto;

- (void)testJsonDecoder;

- (BOOL)zipWithInputStream:(NSInputStream*)inStream outputStream:(NSOutputStream*)outStream;
- (BOOL)unzipWithInputStream:(NSInputStream*)inStream outputStream:(NSOutputStream*)outStream;
- (void)testZip;

@end

