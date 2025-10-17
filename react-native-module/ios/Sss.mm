#import "Sss.h"
#import <React-callinvoker/ReactCommon/CallInvoker.h>
#import <React/RCTBridge+Private.h>
#import <jsi/jsi.h>

@interface RCTBridge (RCTTurboModule)
- (std::shared_ptr<facebook::react::CallInvoker>)jsCallInvoker;
@end

using namespace facebook;

@implementation Sss

@synthesize bridge = _bridge;

RCT_EXPORT_MODULE()

- (void)invalidate {
  _bridge = nil;
}

- (void)setBridge:(RCTBridge *)bridge {
  _bridge = bridge;
}

RCT_EXPORT_BLOCKING_SYNCHRONOUS_METHOD(install)
{
    RCTBridge* bridge = [RCTBridge currentBridge];
    RCTCxxBridge *cxxBridge = (RCTCxxBridge *)_bridge;
    
    if (!cxxBridge.runtime) {
        return @(false);
    }

    auto callInvoker = [bridge jsCallInvoker];
    
    facebook::jsi::Runtime *jsRuntime =
            (facebook::jsi::Runtime *)cxxBridge.runtime;
    
    Sss::installOpenCV(*jsRuntime, callInvoker);
    
    return @(true);
}

- (std::shared_ptr<facebook::react::TurboModule>)getTurboModule:
    (const facebook::react::ObjCTurboModule::InitParams &)params
{
    RCTCxxBridge *cxxBridge = (RCTCxxBridge *)_bridge;
      auto callInvoker = cxxBridge.jsCallInvoker;
      facebook::jsi::Runtime *jsRuntime =
          (facebook::jsi::Runtime *)cxxBridge.runtime;
    
    Sss::installOpenCV(*jsRuntime, callInvoker);
    
    return std::make_shared<facebook::react::NativeFastOpencvSpecJSI>(params);
}

- (void)install:(RCTPromiseResolveBlock)resolve reject:(RCTPromiseRejectBlock)reject { 
    
}

/*
- (NSNumber *)multiply:(double)a b:(double)b {
    NSNumber *result = @(Sss::multiply(a, b));

    return result;
}

- (std::shared_ptr<facebook::react::TurboModule>)getTurboModule:
    (const facebook::react::ObjCTurboModule::InitParams &)params
{
    return std::make_shared<facebook::react::NativeSssSpecJSI>(params);
}
*/


@end
