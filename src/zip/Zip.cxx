#include "Zip.hxx"

#include <algorithm>
#include <stdexcept>
#include <cstring>
#include <cstdlib>
#include <functional>
#include <vector>
#include <type_traits>

#include <NGenXXLog.hxx>

#if defined(MSDOS) || defined(OS2) || defined(WIN32) || defined(__CYGWIN__)
#include <fcntl.h>
#include <io.h>
#define SET_BINARY_MODE(file) _setmode(_fileno(file), O_BINARY)
#else
#define SET_BINARY_MODE(file)
#endif

int NGenXX::Z::ZBase::windowBits()
{
    if (this->format == NGenXXZFormatGZip)
    {
        return 16 | MAX_WBITS;
    }
    if (this->format == NGenXXZFormatRaw)
    {
        return -MAX_WBITS;
    }
    return MAX_WBITS;
}

NGenXX::Z::ZBase::ZBase(const size_t bufferSize, const int format) : bufferSize{bufferSize}, format{format}
{
    if (bufferSize == 0)
    {
        throw std::invalid_argument("bufferSize invalid");
    }
    if (format != NGenXXZFormatRaw && format != NGenXXZFormatZLib && format != NGenXXZFormatGZip)
    {
        throw std::invalid_argument("format invalid");
    }
    this->zs = {
        .zalloc = Z_NULL,
        .zfree = Z_NULL,
        .opaque = Z_NULL
    };
    this->inBuffer = reinterpret_cast<byte *>(std::malloc(sizeof(byte) * bufferSize + 1));
    std::memset(this->inBuffer, 0, bufferSize + 1);
    this->outBuffer = reinterpret_cast<byte *>(std::malloc(sizeof(byte) * bufferSize + 1));
    std::memset(this->outBuffer, 0, bufferSize + 1);
}

size_t NGenXX::Z::ZBase::input(const Bytes &bytes, bool inFinish)
{
    if (bytes.empty())
    {
        return 0;
    }
    auto dataLen = std::min(bytes.size(), this->bufferSize);

    std::memset(this->inBuffer, 0, this->bufferSize);
    std::memcpy(this->inBuffer, bytes.data(), dataLen);

    (this->zs).avail_in = static_cast<uint>(dataLen);
    (this->zs).next_in = reinterpret_cast<Bytef *>(this->inBuffer);
    this->inFinish = inFinish;
    return dataLen;
}

const Bytes NGenXX::Z::ZBase::processDo()
{
    std::memset(this->outBuffer, 0, this->bufferSize);
    (this->zs).avail_out = static_cast<uint>(this->bufferSize);
    (this->zs).next_out = reinterpret_cast<Bytef *>(this->outBuffer);

    // ngenxxLogPrintF(NGenXXLogLevelX::Debug, "z process before avIn:{} avOut:{}", (this->zs).avail_in, (this->zs).avail_out);
    this->processImp();
    // ngenxxLogPrintF(NGenXXLogLevelX::Debug, "z process after avIn:{} avOut:{}", (this->zs).avail_in, (this->zs).avail_out);

    if (this->ret != Z_OK && ret != Z_STREAM_END)
    {
        ngenxxLogPrintF(NGenXXLogLevelX::Error, "z process error:{}", this->ret);
        return BytesEmpty;
    }

    auto outLen = this->bufferSize - (this->zs).avail_out;
    return wrapBytes(this->outBuffer, outLen);
}

bool NGenXX::Z::ZBase::processFinished()
{
    return (this->zs).avail_out != 0;
}

NGenXX::Z::ZBase::~ZBase()
{
    std::free(inBuffer);
    std::free(outBuffer);
}

NGenXX::Z::Zip::Zip(int mode, const size_t bufferSize, const int format) : ZBase(bufferSize, format)
{
    if (mode != NGenXXZipCompressModeDefault && mode != NGenXXZipCompressModePreferSize && mode != NGenXXZipCompressModePreferSpeed)
    {
        throw std::invalid_argument("mode invalid");
    }
    this->ret = deflateInit2(&(this->zs), mode, Z_DEFLATED, this->windowBits(), 8, Z_DEFAULT_STRATEGY);
    if (this->ret != Z_OK)
    {
        ngenxxLogPrintF(NGenXXLogLevelX::Error, "deflateInit error:{}", this->ret);
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

NGenXX::Z::UnZip::UnZip(const size_t bufferSize, const int format) : ZBase(bufferSize, format)
{
    this->ret = inflateInit2(&(this->zs), this->windowBits());
    if (this->ret != Z_OK)
    {
        ngenxxLogPrintF(NGenXXLogLevelX::Error, "inflateInit error:{}", this->ret);
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

bool zProcess(const size_t bufferSize,
              std::function<const Bytes()> &sReadF,
              std::function<void(const Bytes &)> &sWriteF,
              std::function<void()> &sFlushF,
              NGenXX::Z::ZBase &zb)
{
    auto inputFinished = false;
    do
    {
        auto in = sReadF();
        inputFinished = in.size() < bufferSize;
        auto ret = zb.input(in, inputFinished);
        if (ret == 0L)
        {
            return false;
        }

        auto processFinished = false;
        do
        {
            auto outData = zb.processDo();
            if (outData.empty())
            {
                return false;
            }
            processFinished = zb.processFinished();

            sWriteF(outData);
        } while (!processFinished);
    } while (!inputFinished);
    sFlushF();

    return true;
}

#pragma mark Cxx stream

bool zProcessCxxStream(const size_t bufferSize, std::istream *inStream, std::ostream *outStream, NGenXX::Z::ZBase &zb)
{
    return zProcess(bufferSize, 
        [bufferSize, &inStream]() -> const Bytes 
        {
            Bytes in;
            inStream->readsome(reinterpret_cast<char *>(in.data()), bufferSize);
            return in; 
        }, 
        [&outStream](const Bytes &bytes) -> void
        { 
            outStream->write(reinterpret_cast<char *>(std::decay_t<byte *>(bytes.data())), bytes.size());
        }, 
        [&outStream]() -> void
        { 
            outStream->flush(); 
        }, 
        zb
    );
}

bool NGenXX::Z::zip(int mode, const size_t bufferSize, const int format, std::istream *inStream, std::ostream *outStream)
{
    Zip zip(mode, bufferSize, format);
    return zProcessCxxStream(bufferSize, inStream, outStream, zip);
}

bool NGenXX::Z::unzip(const size_t bufferSize, const int format, std::istream *inStream, std::ostream *outStream)
{
    UnZip unzip(bufferSize, format);
    return zProcessCxxStream(bufferSize, inStream, outStream, unzip);
}

#pragma mark C FILE

bool zProcessCFILE(const size_t bufferSize, std::FILE *inFile, std::FILE *outFile, NGenXX::Z::ZBase &zb)
{
    return zProcess(bufferSize, 
        [bufferSize, &inFile]() -> const Bytes
        {
            Bytes in;
            std::fread(static_cast<void *>(in.data()), sizeof(byte), bufferSize, inFile);
            return in;
        }, 
        [&outFile](const Bytes &bytes) -> void
        {
            std::fwrite(bytes.data(), sizeof(byte), bytes.size(), outFile);
        }, 
        [&outFile]() -> void
        { 
            std::fflush(outFile);
        }, 
        zb
    );
}

bool NGenXX::Z::zip(int mode, const size_t bufferSize, const int format, std::FILE *inFile, std::FILE *outFile)
{
    Zip zip(mode, bufferSize, format);
    return zProcessCFILE(bufferSize, inFile, outFile, zip);
}

bool NGenXX::Z::unzip(const size_t bufferSize, const int format, std::FILE *inFile, std::FILE *outFile)
{
    UnZip unzip(bufferSize, format);
    return zProcessCFILE(bufferSize, inFile, outFile, unzip);
}

#pragma mark Bytes

const Bytes zProcessBytes(const size_t bufferSize, const Bytes &in, NGenXX::Z::ZBase &zb)
{
    size_t pos = 0;
    Bytes outBytes;
    auto b = zProcess(bufferSize, 
        [bufferSize, &in, &pos]() -> const Bytes
        {
            auto len = std::min(bufferSize, in.size() - pos);
            Bytes bytes(in.begin() + pos, in.begin() + pos + len);
            pos += len;
            return bytes;
        }, 
        [&outBytes](const Bytes &bytes) -> void
        {
            outBytes.insert(outBytes.end(), bytes.begin(), bytes.end());
        }, 
        []() -> void {}, 
        zb
    );
    if (!b)
    {
        return BytesEmpty;
    }
    return outBytes;
}

const Bytes NGenXX::Z::zip(int mode, const size_t bufferSize, const int format, const Bytes &bytes)
{
    Zip zip(mode, bufferSize, format);
    return zProcessBytes(bufferSize, bytes, zip);
}

const Bytes NGenXX::Z::unzip(const size_t bufferSize, const int format, const Bytes &bytes)
{
    UnZip unzip(bufferSize, format);
    return zProcessBytes(bufferSize, bytes, unzip);
}