#include "Zip.hxx"
#include "../../include/NGenXXLog.h"
#include "../log/Log.hxx"
#include <algorithm>

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
        .opaque = Z_NULL};
    this->inBuffer = (byte *)malloc(sizeof(byte) * bufferSize + 1);
    this->outBuffer = (byte *)malloc(sizeof(byte) * bufferSize + 1);
}

const size NGenXX::Z::ZBase::input(const byte *in, const size inLen, bool inFinish)
{
    if (in == NULL || inLen <= 0)
        return 0;
    size dataLen = std::min(inLen, this->bufferSize);

    memset(this->inBuffer, 0, this->bufferSize);
    memcpy(this->inBuffer, in, dataLen);

    (this->zs).avail_in = dataLen;
    (this->zs).next_in = (Bytef *)(this->inBuffer);
    this->inFinish = inFinish;
    return dataLen;
}

const byte *NGenXX::Z::ZBase::processDo(size *outLen)
{
    if (outLen == NULL) return NULL;
    memset(this->outBuffer, 0, this->bufferSize);
    (this->zs).avail_out = this->bufferSize;
    (this->zs).next_out = (Bytef *)(this->outBuffer);

    this->processImp();
    if (this->ret != Z_OK && ret != Z_STREAM_END)
    {
        Log::print(NGenXXLogLevelError, "inflate error:" + std::to_string(this->ret));
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
    deflateEnd(&(this->zs));
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
        Log::print(NGenXXLogLevelError, "deflateInit error:" + std::to_string(this->ret));
        throw std::runtime_error("deflateInit failed");
    }
}

void NGenXX::Z::Zip::processImp()
{
    this->ret = deflate(&(this->zs), this->inFinish ? Z_FINISH : Z_NO_FLUSH);
}

NGenXX::Z::Zip::~Zip()
{
}

NGenXX::Z::UnZip::UnZip(const size bufferSize) : ZBase(bufferSize)
{
    this->ret = inflateInit(&(this->zs));
    if (this->ret != Z_OK)
    {
        Log::print(NGenXXLogLevelError, "inflateInit error:" + std::to_string(this->ret));
        throw std::runtime_error("inflateInit failed");
    }
}

void NGenXX::Z::UnZip::processImp()
{
    this->ret = inflate(&(this->zs), Z_NO_FLUSH);
}

NGenXX::Z::UnZip::~UnZip()
{
}