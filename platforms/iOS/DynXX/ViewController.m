//
//  ViewController.mm
//  DynXX
//
//  Created by Rinc on 2024/8/21.
//

#import "ViewController.h"
#import "DynXXApple.h"

@interface ViewController ()

@property (nonatomic, strong) DynXXApple *na;

@end

@implementation ViewController

- (void)viewDidLoad {
    [super viewDidLoad];
    
    _na = [DynXXApple new];
    [_na testDeviceInfo];
    [_na testDB];
    [_na testKV];
    [_na testHttpL];
    [_na testHttpJ];
    [_na testCrypto];
    [_na testJsonDecoder];
    [_na testZip];
}

@end
