#ifndef NGENXX_INCLUDE_Z_HXX_
#define NGENXX_INCLUDE_Z_HXX_

#include "NGenXXTypes.hxx"

enum class NGenXXZipCompressModeX : int
{
    Default = -1,
    PreferSpeed = 1,
    PreferSize = 9,
};

enum class NGenXXZFormatX : int
{
    ZLib = 0,
    GZip = 1,
    Raw = 2,
};

void *const ngenxxZZipInit(const NGenXXZipCompressModeX mode, const size_t bufferSize, const NGenXXZFormatX format);

const size_t ngenxxZZipInput(void *const zip, const Bytes inBytes, const bool inFinish);

const Bytes ngenxxZZipProcessDo(void *const zip);

const bool ngenxxZZipProcessFinished(void *const zip);

void ngenxxZZipRelease(void *const zip);

void *const ngenxxZUnzipInit(const size_t bufferSize, const NGenXXZFormatX format);

const size_t ngenxxZUnzipInput(void *const unzip, const Bytes inBytes, const bool inFinish);

const Bytes ngenxxZUnzipProcessDo(void *const unzip);

const bool ngenxxZUnzipProcessFinished(void *const unzip);

void ngenxxZUnzipRelease(void *const unzip);

bool ngenxxZCFileZip(const NGenXXZipCompressModeX mode, const size_t bufferSize, const NGenXXZFormatX format, std::FILE *cFILEIn, std::FILE *cFILEOut);

bool ngenxxZCFileUnzip(const size_t bufferSize, const NGenXXZFormatX format, std::FILE *cFILEIn, std::FILE *cFILEOut);

bool ngenxxZCxxStreamZip(const NGenXXZipCompressModeX mode, const size_t bufferSize, const NGenXXZFormatX format, std::istream *cxxStreamIn, std::ostream *cxxStreamOut);

bool ngenxxZCxxStreamUnzip(const size_t bufferSize, const NGenXXZFormatX format, std::istream *cxxStreamIn, std::ostream *cxxStreamOut);

const Bytes ngenxxZBytesZip(const NGenXXZipCompressModeX mode, const size_t bufferSize, const NGenXXZFormatX format, const Bytes inBytes);

const Bytes ngenxxZBytesUnzip(const size_t bufferSize, const NGenXXZFormatX format, const Bytes inBytes);

#endif //  NGENXX_INCLUDE_Z_HXX_