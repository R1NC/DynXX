#ifndef NGENXX_JNI_UTIL_HXX_
#define NGENXX_JNI_UTIL_HXX_

#include <jni.h>
#include <cstdlib>
#include <string>

jbyteArray moveToJByteArray(JNIEnv *env, const unsigned char *bytes, unsigned long outLen, bool needFree)
{
    jbyteArray jba;
    if (bytes && outLen > 0)
    {
        jba = env->NewByteArray(outLen);
        env->SetByteArrayRegion(jba, 0, outLen, (jbyte *)bytes);
        if (needFree) free((void *)bytes);
    }
    else
    {
        jba = env->NewByteArray(0);
    }
    return jba;
}

#endif // NGENXX_JNI_UTIL_HXX_