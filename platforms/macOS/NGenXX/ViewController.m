//
//  ViewController.m
//  SdkEngineDemo
//
//  Created by Rinc on 2024/8/23.
//

#import "ViewController.h"
#import "NGenXXApple.h"

@interface ViewController () 

@property (nonatomic, strong) NGenXXApple *na;

@end

@implementation ViewController

- (void)viewDidLoad {
    [super viewDidLoad];
    
    _na = [NGenXXApple new];
    [_na testDeviceInfo];
    [_na testDB];
    [_na testKV];
    [_na testHttpL];
    [_na testHttpJ];
    [_na testCrypto];
    [_na testJsonDecoder];
    [_na testZip];
}

- (void)setRepresentedObject:(id)representedObject {
    [super setRepresentedObject:representedObject];
}

@end
