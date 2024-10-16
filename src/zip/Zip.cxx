#include "Zip.hxx"
#include "../../include/NGenXXLog.h"
#include "../log/Log.hxx"
#include "../NGenXX-Types.hxx"
#include <algorithm>
#include <stdexcept>
#include <cstring>
#include <functional>

#if defined(MSDOS) || defined(OS2) || defined(WIN32) || defined(__CYGWIN__)
#include <fcntl.h>
#include <io.h>
#define SET_BINARY_MODE(file) _setmode(_fileno(file), O_BINARY)
#else
#define SET_BINARY_MODE(file)
#endif

NGenXX::Z::ZBase::ZBase(const size bufferSize) : bufferSize{bufferSize}
{
    if (bufferSize <= 0) {
        throw std::invalid_argument("bufferSize invalid");
    }
    this->zs = {
        .zalloc = Z_NULL,
        .zfree = Z_NULL,
        .opaque = Z_NULL
    };
    this->inBuffer = (byte *)malloc(sizeof(byte) * bufferSize + 1);
    this->outBuffer = (byte *)malloc(sizeof(byte) * bufferSize + 1);
}

const size NGenXX::Z::ZBase::input(const Bytes bytes, bool inFinish)
{
    auto [in, inLen] = bytes;
    if (in == NULL || inLen <= 0)
        return 0;
    size dataLen = std::min(inLen, this->bufferSize);

    std::memset(this->inBuffer, 0, this->bufferSize);
    memcpy(this->inBuffer, in, dataLen);

    (this->zs).avail_in = dataLen;
    (this->zs).next_in = (Bytef *)(this->inBuffer);
    this->inFinish = inFinish;
    return dataLen;
}

const byte *NGenXX::Z::ZBase::processDo(size *outLen)
{
    if (outLen == NULL) return NULL;
    std::memset(this->outBuffer, 0, this->bufferSize);
    (this->zs).avail_out = this->bufferSize;
    (this->zs).next_out = (Bytef *)(this->outBuffer);

    this->processImp();
    if (this->ret != Z_OK && ret != Z_STREAM_END)
    {
        NGenXX::Log::print(NGenXXLogLevelError, "z process error:" + std::to_string(this->ret));
        *outLen = 0;
        return NULL;
    }

    *outLen = this->bufferSize - (this->zs).avail_out;
    return this->outBuffer;
}

const bool NGenXX::Z::ZBase::processFinished()
{
    return (this->zs).avail_out != 0;
}

NGenXX::Z::ZBase::~ZBase()
{
    free((void *)inBuffer);
    free((void *)outBuffer);
}

NGenXX::Z::Zip::Zip(int mode, const size bufferSize) : ZBase(bufferSize)
{
    if (mode != NGenXXZipCompressModeDefault && mode != NGenXXZipCompressModePreferSize && mode != NGenXXZipCompressModePreferSpeed) {
        throw std::invalid_argument("mode invalid");
    }
    this->ret = deflateInit(&(this->zs), mode);
    if (this->ret != Z_OK)
    {
        NGenXX::Log::print(NGenXXLogLevelError, "deflateInit error:" + std::to_string(this->ret));
        throw std::runtime_error("deflateInit failed");
    }
}

void NGenXX::Z::Zip::processImp()
{
    this->ret = deflate(&(this->zs), this->inFinish ? Z_FINISH : Z_NO_FLUSH);
}

NGenXX::Z::Zip::~Zip()
{
    deflateEnd(&(this->zs));
}

NGenXX::Z::UnZip::UnZip(const size bufferSize) : ZBase(bufferSize)
{
    this->ret = inflateInit(&(this->zs));
    if (this->ret != Z_OK)
    {
        NGenXX::Log::print(NGenXXLogLevelError, "inflateInit error:" + std::to_string(this->ret));
        throw std::runtime_error("inflateInit failed");
    }
}

void NGenXX::Z::UnZip::processImp()
{
    this->ret = inflate(&(this->zs), Z_NO_FLUSH);
}

NGenXX::Z::UnZip::~UnZip()
{
    inflateEnd(&(this->zs));
}

#pragma mark Stream

bool zProcess(const size bufferSize,
              std::function<size(NGenXX::Bytes)> sReadF,
              std::function<void(NGenXX::Bytes)> sWriteF,
              std::function<void()> sFlushF,
              NGenXX::Z::ZBase &zb)
{
    byte inBuffer[bufferSize];

    bool inputFinished;
    do
    {
        size inLen = sReadF({inBuffer, bufferSize});
        inputFinished = inLen < bufferSize;
        int ret = zb.input({inBuffer, inLen}, inputFinished);
        if (ret == 0L)
        {
            return false;
        }

        bool processFinished;
        do
        {
            size outLen;
            const byte *outData = zb.processDo(&outLen);
            if (outData == NULL)
            {
                return false;
            }
            NGenXX::Bytes outBytes = {outData, outLen};
            processFinished = zb.processFinished();

            sWriteF(outBytes);
        } while (!processFinished);
    } while (!inputFinished);
    sFlushF();

    return true;
}

#pragma mark Cxx stream

bool zProcessCxxStream(const size bufferSize, std::istream *inStream, std::ostream *outStream, NGenXX::Z::ZBase &zb)
{
    return zProcess(bufferSize, 
        [&](NGenXX::Bytes bytes) -> size {
            return inStream->readsome((char *)bytes.first, bytes.second);
        },
        [&](NGenXX::Bytes bytes) -> void {
            outStream->write((char *)(bytes.first), bytes.second);
        },
        [&]() -> void {
            outStream->flush();
        },
        zb
    );
}

bool NGenXX::Z::zip(int mode, const size bufferSize, std::istream *inStream, std::ostream *outStream)
{
    auto zip = Zip(mode, bufferSize);
    return zProcessCxxStream(bufferSize, inStream, outStream, zip);
}

bool NGenXX::Z::unzip(const size bufferSize, std::istream *inStream, std::ostream *outStream)
{
    auto unzip = UnZip(bufferSize);
    return zProcessCxxStream(bufferSize, inStream, outStream, unzip);
}

#pragma mark C FILE

bool zProcessCFILE(const size bufferSize, std::FILE *inFile, std::FILE *outFile, NGenXX::Z::ZBase &zb)
{
    return zProcess(bufferSize, 
        [&](NGenXX::Bytes bytes) -> size {
            return std::fread((void *)bytes.first, sizeof(byte), bytes.second, inFile);
        },
        [&](NGenXX::Bytes bytes) -> void {
            std::fwrite(bytes.first, sizeof(byte), bytes.second, outFile);
        },
        [&]() -> void {
            std::fflush(outFile);
        },
        zb
    );
}

bool NGenXX::Z::zip(int mode, const size bufferSize, std::FILE *inFile, std::FILE *outFile)
{
    auto zip = Zip(mode, bufferSize);
    return zProcessCFILE(bufferSize, inFile, outFile, zip);
}

bool NGenXX::Z::unzip(const size bufferSize, std::FILE *inFile, std::FILE *outFile)
{
    auto unzip = UnZip(bufferSize);
    return zProcessCFILE(bufferSize, inFile, outFile, unzip);
}