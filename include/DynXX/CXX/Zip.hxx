#ifndef DYNXX_INCLUDE_Z_HXX_
#define DYNXX_INCLUDE_Z_HXX_

#include "Types.hxx"

constexpr size_t DynXXZDefaultBufferSize = 16 * 1024;

enum class DynXXZipCompressModeX : int {
    Default = -1,
    PreferSpeed = 1,
    PreferSize = 9,
};

enum class DynXXZFormatX : int {
    ZLib = 0,
    GZip = 1,
    Raw = 2,
};

void *dynxxZZipInit(const DynXXZipCompressModeX mode = DynXXZipCompressModeX::Default,
                     size_t bufferSize = DynXXZDefaultBufferSize,
                     const DynXXZFormatX format = DynXXZFormatX::ZLib);

size_t dynxxZZipInput(void *const zip, const Bytes &inBytes, bool inFinish);

Bytes dynxxZZipProcessDo(void *const zip);

bool dynxxZZipProcessFinished(void *const zip);

void dynxxZZipRelease(void *const zip);

void *dynxxZUnzipInit(size_t bufferSize = DynXXZDefaultBufferSize,
                       const DynXXZFormatX format = DynXXZFormatX::ZLib);

size_t dynxxZUnzipInput(void *const unzip, const Bytes &inBytes, bool inFinish);

Bytes dynxxZUnzipProcessDo(void *const unzip);

bool dynxxZUnzipProcessFinished(void *const unzip);

void dynxxZUnzipRelease(void *const unzip);

bool dynxxZCFileZip(std::FILE *cFILEIn, std::FILE *cFILEOut,
                     const DynXXZipCompressModeX mode = DynXXZipCompressModeX::Default,
                     size_t bufferSize = DynXXZDefaultBufferSize,
                     const DynXXZFormatX format = DynXXZFormatX::ZLib);

bool dynxxZCFileUnzip(std::FILE *cFILEIn, std::FILE *cFILEOut,
                       size_t bufferSize = DynXXZDefaultBufferSize,
                       const DynXXZFormatX format = DynXXZFormatX::ZLib);

bool dynxxZCxxStreamZip(std::istream *cxxStreamIn, std::ostream *cxxStreamOut,
                         const DynXXZipCompressModeX mode = DynXXZipCompressModeX::Default,
                         size_t bufferSize = DynXXZDefaultBufferSize,
                         const DynXXZFormatX format = DynXXZFormatX::ZLib);

bool dynxxZCxxStreamUnzip(std::istream *cxxStreamIn, std::ostream *cxxStreamOut,
                           size_t bufferSize = DynXXZDefaultBufferSize,
                           const DynXXZFormatX format = DynXXZFormatX::ZLib);

Bytes dynxxZBytesZip(const Bytes &inBytes,
                      const DynXXZipCompressModeX mode = DynXXZipCompressModeX::Default,
                      size_t bufferSize = DynXXZDefaultBufferSize,
                      const DynXXZFormatX format = DynXXZFormatX::ZLib);

Bytes dynxxZBytesUnzip(const Bytes &inBytes,
                        size_t bufferSize = DynXXZDefaultBufferSize,
                        const DynXXZFormatX format = DynXXZFormatX::ZLib);

#endif // DYNXX_INCLUDE_Z_HXX_
