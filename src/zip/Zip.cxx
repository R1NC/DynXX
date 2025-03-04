#include "Zip.hxx"

#include <cstring>
#include <cstdlib>

#include <algorithm>
#include <stdexcept>
#include <functional>
#include <vector>

#include <NGenXXLog.hxx>

#if defined(MSDOS) || defined(OS2) || defined(WIN32) || defined(__CYGWIN__)
#include <fcntl.h>
#include <io.h>
#define SET_BINARY_MODE(file) _setmode(_fileno(file), O_BINARY)
#else
#define SET_BINARY_MODE(file)
#endif

template <typename T>
int NGenXX::Z::ZBase<T>::windowBits()
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

template <typename T>
NGenXX::Z::ZBase<T>::ZBase(size_t bufferSize, int format) : bufferSize{bufferSize}, format{format}
{
    if (bufferSize == 0) [[unlikely]]
    {
        throw std::invalid_argument("bufferSize invalid");
    }
    if (format != NGenXXZFormatRaw && format != NGenXXZFormatZLib && format != NGenXXZFormatGZip) [[unlikely]]
    {
        throw std::invalid_argument("format invalid");
    }
    this->zs = {
        .zalloc = Z_NULL,
        .zfree = Z_NULL,
        .opaque = Z_NULL
    };
    this->inBuffer = mallocX<byte>(bufferSize);
    this->outBuffer = mallocX<byte>(bufferSize);
}

template <typename T>
size_t NGenXX::Z::ZBase<T>::input(const Bytes &bytes, bool inFinish)
{
    if (bytes.empty()) [[unlikely]]
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

template <typename T>
Bytes NGenXX::Z::ZBase<T>::processDo()
{
    std::memset(this->outBuffer, 0, this->bufferSize);
    (this->zs).avail_out = static_cast<uint>(this->bufferSize);
    (this->zs).next_out = reinterpret_cast<Bytef *>(this->outBuffer);

    // ngenxxLogPrintF(NGenXXLogLevelX::Debug, "z process before avIn:{} avOut:{}", (this->zs).avail_in, (this->zs).avail_out);
    static_cast<T *>(this)->processImp();
    // ngenxxLogPrintF(NGenXXLogLevelX::Debug, "z process after avIn:{} avOut:{}", (this->zs).avail_in, (this->zs).avail_out);

    if (this->ret != Z_OK && ret != Z_STREAM_END)
    {
        ngenxxLogPrintF(NGenXXLogLevelX::Error, "z process error:{}", this->ret);
        return {};
    }

    auto outLen = this->bufferSize - (this->zs).avail_out;
    return wrapBytes(this->outBuffer, outLen);
}

template <typename T>
bool NGenXX::Z::ZBase<T>::processFinished() const
{
    return (this->zs).avail_out != 0;
}

template <typename T>
NGenXX::Z::ZBase<T>::~ZBase()
{
    freeX(inBuffer);
    freeX(outBuffer);
}

// Explicit template instantiation
template class NGenXX::Z::ZBase<NGenXX::Z::Zip>;
template class NGenXX::Z::ZBase<NGenXX::Z::UnZip>;

NGenXX::Z::Zip::Zip(int mode, size_t bufferSize, int format) : ZBase(bufferSize, format)
{
    if (mode != NGenXXZipCompressModeDefault && mode != NGenXXZipCompressModePreferSize && mode != NGenXXZipCompressModePreferSpeed) [[unlikely]]
    {
        throw std::invalid_argument("mode invalid");
    }
    this->ret = deflateInit2(&(this->zs), mode, Z_DEFLATED, this->windowBits(), 8, Z_DEFAULT_STRATEGY);
    if (this->ret != Z_OK) [[unlikely]]
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

NGenXX::Z::UnZip::UnZip(size_t bufferSize, int format) : ZBase(bufferSize, format)
{
    this->ret = inflateInit2(&(this->zs), this->windowBits());
    if (this->ret != Z_OK) [[unlikely]]
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

template <typename T>
bool _ngenxx_z_process(size_t bufferSize,
              std::function<const Bytes()> sReadF,
              std::function<void(const Bytes &)> sWriteF,
              std::function<void()> sFlushF,
              NGenXX::Z::ZBase<T> &zb)
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

template <typename T>
bool _ngenxx_z_processCxxStream(size_t bufferSize, std::istream *inStream, std::ostream *outStream, NGenXX::Z::ZBase<T> &zb)
{
    return _ngenxx_z_process(bufferSize, 
        [bufferSize, &inStream] -> Bytes 
        {
            Bytes in;
            inStream->readsome(reinterpret_cast<char *>(in.data()), bufferSize);
            return in; 
        }, 
        [&outStream](const Bytes &bytes)
        { 
            outStream->write(const_cast<char *>(reinterpret_cast<const char *>(bytes.data())), bytes.size());
        }, 
        [&outStream]
        { 
            outStream->flush(); 
        }, 
        zb
    );
}

bool NGenXX::Z::zip(int mode, size_t bufferSize, int format, std::istream *inStream, std::ostream *outStream)
{
    Zip zip(mode, bufferSize, format);
    return _ngenxx_z_processCxxStream(bufferSize, inStream, outStream, zip);
}

bool NGenXX::Z::unzip(size_t bufferSize, int format, std::istream *inStream, std::ostream *outStream)
{
    UnZip unzip(bufferSize, format);
    return _ngenxx_z_processCxxStream(bufferSize, inStream, outStream, unzip);
}

#pragma mark C FILE

template <typename T>
bool _ngenxx_z_processCFILE(size_t bufferSize, std::FILE *inFile, std::FILE *outFile, NGenXX::Z::ZBase<T> &zb)
{
    return _ngenxx_z_process(bufferSize, 
        [bufferSize, &inFile] -> Bytes
        {
            Bytes in;
            std::fread(static_cast<void *>(in.data()), sizeof(byte), bufferSize, inFile);
            return in;
        }, 
        [&outFile](const Bytes &bytes)
        {
            std::fwrite(bytes.data(), sizeof(byte), bytes.size(), outFile);
        }, 
        [&outFile]
        { 
            std::fflush(outFile);
        }, 
        zb
    );
}

bool NGenXX::Z::zip(int mode, size_t bufferSize, int format, std::FILE *inFile, std::FILE *outFile)
{
    Zip zip(mode, bufferSize, format);
    return _ngenxx_z_processCFILE(bufferSize, inFile, outFile, zip);
}

bool NGenXX::Z::unzip(size_t bufferSize, int format, std::FILE *inFile, std::FILE *outFile)
{
    UnZip unzip(bufferSize, format);
    return _ngenxx_z_processCFILE(bufferSize, inFile, outFile, unzip);
}

#pragma mark Bytes

template <typename T>
Bytes _ngenxx_z_processBytes(size_t bufferSize, const Bytes &in, NGenXX::Z::ZBase<T> &zb)
{
    std::remove_const<decltype(bufferSize)>::type pos(0);
    Bytes outBytes;
    auto b = _ngenxx_z_process(bufferSize, 
        [bufferSize, &in, &pos] -> Bytes
        {
            auto len = std::min(bufferSize, in.size() - pos);
            Bytes bytes(in.begin() + pos, in.begin() + pos + len);
            pos += len;
            return bytes;
        }, 
        [&outBytes](const Bytes &bytes)
        {
            outBytes.insert(outBytes.end(), bytes.begin(), bytes.end());
        }, 
        [] {}, 
        zb
    );
    if (!b)
    {
        return {};
    }
    return outBytes;
}

Bytes NGenXX::Z::zip(int mode, size_t bufferSize, int format, const Bytes &bytes)
{
    Zip zip(mode, bufferSize, format);
    return _ngenxx_z_processBytes(bufferSize, bytes, zip);
}

Bytes NGenXX::Z::unzip(size_t bufferSize, int format, const Bytes &bytes)
{
    UnZip unzip(bufferSize, format);
    return _ngenxx_z_processBytes(bufferSize, bytes, unzip);
}