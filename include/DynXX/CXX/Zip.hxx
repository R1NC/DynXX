#pragma once

#include "Types.hxx"
#include "../C/Zip.h"
#include <cstdio>

constexpr auto DynXXZDefaultBufferSize = 16 * 1024UZ;

enum class DynXXZipCompressModeX : int8_t {
    Default = -1,
    PreferSpeed = 1,
    PreferSize = 9,
};

enum class DynXXZFormatX : uint8_t {
    ZLib = 0,
    GZip = 1,
    Raw = 2,
};

DynXXZipHandle dynxxZZipInit(DynXXZipCompressModeX mode = DynXXZipCompressModeX::Default,
                     size_t bufferSize = DynXXZDefaultBufferSize,
                     DynXXZFormatX format = DynXXZFormatX::ZLib);

[[nodiscard]] size_t dynxxZZipInput(DynXXZipHandle zip, BytesView inBytes, bool inFinish);

Bytes dynxxZZipProcessDo(DynXXZipHandle zip);

bool dynxxZZipProcessFinished(DynXXZipHandle zip);

void dynxxZZipRelease(DynXXZipHandle zip);

DynXXUnZipHandle dynxxZUnzipInit(size_t bufferSize = DynXXZDefaultBufferSize,
                       DynXXZFormatX format = DynXXZFormatX::ZLib);

[[nodiscard]] size_t dynxxZUnzipInput(DynXXUnZipHandle unzip, BytesView inBytes, bool inFinish);

Bytes dynxxZUnzipProcessDo(DynXXUnZipHandle unzip);

bool dynxxZUnzipProcessFinished(DynXXUnZipHandle unzip);

void dynxxZUnzipRelease(DynXXUnZipHandle unzip);

[[nodiscard]] bool dynxxZCFileZip(std::FILE *cFILEIn, std::FILE *cFILEOut,
                     DynXXZipCompressModeX mode = DynXXZipCompressModeX::Default,
                     size_t bufferSize = DynXXZDefaultBufferSize,
                     DynXXZFormatX format = DynXXZFormatX::ZLib);

[[nodiscard]] bool dynxxZCFileUnzip(std::FILE *cFILEIn, std::FILE *cFILEOut,
                       size_t bufferSize = DynXXZDefaultBufferSize,
                       DynXXZFormatX format = DynXXZFormatX::ZLib);

[[nodiscard]] bool dynxxZCxxStreamZip(std::istream *cxxStreamIn, std::ostream *cxxStreamOut,
                         DynXXZipCompressModeX mode = DynXXZipCompressModeX::Default,
                         size_t bufferSize = DynXXZDefaultBufferSize,
                         DynXXZFormatX format = DynXXZFormatX::ZLib);

[[nodiscard]] bool dynxxZCxxStreamUnzip(std::istream *cxxStreamIn, std::ostream *cxxStreamOut,
                           size_t bufferSize = DynXXZDefaultBufferSize,
                           DynXXZFormatX format = DynXXZFormatX::ZLib);

Bytes dynxxZBytesZip(BytesView inBytes,
                      DynXXZipCompressModeX mode = DynXXZipCompressModeX::Default,
                      size_t bufferSize = DynXXZDefaultBufferSize,
                      DynXXZFormatX format = DynXXZFormatX::ZLib);

Bytes dynxxZBytesUnzip(BytesView inBytes,
                        size_t bufferSize = DynXXZDefaultBufferSize,
                        DynXXZFormatX format = DynXXZFormatX::ZLib);
