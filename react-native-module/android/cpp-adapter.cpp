#include <jni.h>
#include <jsi/jsi.h>
#include "react-native-sss.h"
#include <ReactCommon/CallInvokerHolder.h>

extern "C"
JNIEXPORT void JNICALL
Java_com_sss_SssModule_nativeInstall(JNIEnv *env, jobject thiz, jlong jsi_runtime_ref,
                                                   jobject js_call_invoker_holder) {
    auto jsiRuntime{ reinterpret_cast<jsi::Runtime*>(jsi_runtime_ref) };
    auto jsCallInvoker{ jni::alias_ref<react::CallInvokerHolder::javaobject>{reinterpret_cast<react::CallInvokerHolder::javaobject>(js_call_invoker_holder) }->cthis()->getCallInvoker() };

    Sss::installOpenCV(*jsiRuntime, jsCallInvoker);
};
