#ifndef NGENXX_ZIP_H_
#define NGENXX_ZIP_H_

#include "NGenXXTypes.h"

#ifdef __cplusplus
extern "C"
{
#endif

    enum NGenXXZipCompressMode
    {
        NGenXXZipCompressModeDefault = -1,
        NGenXXZipCompressModePreferSpeed = 1,
        NGenXXZipCompressModePreferSize = 9,
    };

    void *ngenxx_z_zip_init(const int mode, const size bufferSize);

    const size ngenxx_z_zip_input(const void *zip, const byte *in, const size inLen, const bool inFinish);

    const byte *ngenxx_z_zip_process_do(const void *zip, size *outLen);

    const bool ngenxx_z_zip_process_finished(const void *zip);

    void ngenxx_z_zip_release(const void *zip);

    void *ngenxx_z_unzip_init(const size bufferSize);

    const size ngenxx_z_unzip_input(const void *unzip, const byte *in, const size inLen, const bool inFinish);

    const byte *ngenxx_z_unzip_process_do(const void *unzip, size *outLen);

    const bool ngenxx_z_unzip_process_finished(const void *unzip);

    void ngenxx_z_unzip_release(const void *unzip);

#ifdef __cplusplus
}
#endif

#endif // NGENXX_ZIP_H_