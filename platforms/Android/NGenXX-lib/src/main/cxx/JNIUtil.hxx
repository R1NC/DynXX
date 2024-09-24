#include <jni.h>
#include <cstdlib>
#include <string>

jbyteArray toJByteArray(JNIEnv *env, const unsigned char *bytes, unsigned long outLen)
{
    jbyteArray jba;
    if (bytes)
    {
        jba = env->NewByteArray(outLen);
        env->SetByteArrayRegion(jba, 0, outLen, (jbyte *)bytes);
        free((void *)bytes);
    }
    else
    {
        jba = env->NewByteArray(0);
    }
    return jba;
}