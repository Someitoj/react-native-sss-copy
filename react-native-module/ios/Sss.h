#ifdef __cplusplus
#import "react-native-sss.h"
#endif



#undef YES
#undef NO
#include <opencv2/opencv.hpp>
using namespace cv;
#if __has_feature(objc_bool)
#define YES __objc_yes
#define NO  __objc_no
#else
#define YES ((BOOL)1)
#define NO  ((BOOL)0)
#endif
#endif

#import "generated/RNSssSpec/RNSssSpec.h"
//#ifdef RCT_NEW_ARCH_ENABLED
//#import "RNFastOpencvSpec.h"

@interface Sss : NSObject <NativeSssSpec>

//#else
//#import <React/RCTBridgeModule.h>

@end
