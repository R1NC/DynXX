#include "Zip.hxx"

#include <algorithm>
#include <stdexcept>
#include <functional>

#include <DynXX/CXX/Log.hxx>
#include <DynXX/CXX/Memory.hxx>

#if defined(MSDOS) || defined(OS2) || defined(WIN32) || defined(__CYGWIN__)
#include <fcntl.h>
#include <io.h>
#define SET_BINARY_MODE(file) _setmode(_fileno(file), O_BINARY)
#else
#define SET_BINARY_MODE(file)
#endif

namespace {
    using enum DynXXLogLevelX;
    using enum DynXXZipCompressModeX;
    using enum DynXXZFormatX;
    using namespace DynXX::Core::Z;
    
    template<typename T>
    concept ReadBytesFuncT = requires(T f) {
        { f() } -> std::convertible_to<Bytes>;
    };

    template<typename T>
    concept WriteBytesFuncT = requires(T f) {
        { f(Bytes{}) } -> std::convertible_to<void>;
    };

    template<typename T>
    concept FlushFuncT = requires(T f) {
        { f() } -> std::convertible_to<void>;
    };

    template <typename T, ReadBytesFuncT RFT, WriteBytesFuncT WFT, FlushFuncT FFT = std::function<void()>>
    bool process(ZBase<T> &zb, size_t bufferSize, 
              RFT &&readF, WFT &&writeF, FFT &&flushF = [](){})
    {
        auto inputFinished = false;
        do
        {
            auto in = std::forward<RFT>(readF)();
            inputFinished = in.size() < bufferSize;
            if (auto ret = zb.input(in, inputFinished); ret == 0L) [[unlikely]]
            {
                return false;
            }

            auto processFinished = false;
            do
            {
                auto outData = zb.processDo();
                if (outData.empty()) [[unlikely]]
                {
                    return false;
                }
                processFinished = zb.processFinished();

                std::forward<WFT>(writeF)(outData);
            } while (!processFinished);
        } while (!inputFinished);
        std::forward<FFT>(flushF)();

        return true;
    }

#if !defined(__EMSCRIPTEN__)

    template <typename T>
    bool processCxxStream(size_t bufferSize, std::istream *inStream, std::ostream *outStream, ZBase<T> &zb)
    {
        return process(zb, bufferSize,
            [bufferSize, inStream]
            {
                Bytes in;
                inStream->readsome(reinterpret_cast<char *>(in.data()), static_cast<std::streamsize>(bufferSize));
                return in;
            },
            [outStream](const Bytes &bytes)
            {
                outStream->write(reinterpret_cast<const char *>(bytes.data()), static_cast<std::streamsize>(bytes.size()));
            },
            [outStream]
            {
                outStream->flush();
            }
        );
    }

    template <typename T>
    bool processCFILE(size_t bufferSize, std::FILE *inFile, std::FILE *outFile, ZBase<T> &zb)
    {
        return process(zb, bufferSize,
            [bufferSize, inFile]
            {
                Bytes in;
                const auto ret = std::fread(in.data(), sizeof(byte), bufferSize, inFile);
                dynxxLogPrintF(Debug, "Z read ret: {}", ret);
                return in;
            },
            [outFile](const Bytes &bytes)
            {
                const auto ret = std::fwrite(bytes.data(), sizeof(byte), bytes.size(), outFile);
                dynxxLogPrintF(Debug, "Z write ret: {}", ret);
            },
            [outFile]
            {
                const auto ret = std::fflush(outFile);
                dynxxLogPrintF(Debug, "Z fflush ret: {}", ret); 
            }
        );
    }

#endif

    template <typename T>
    Bytes processBytes(size_t bufferSize, BytesView inBytesView, ZBase<T> &zb)
    {
        if (inBytesView.empty()) [[unlikely]]
        {
            return {};
        }
        int64_t pos(0);
        Bytes outBytes;
        if (!process(zb, bufferSize,
            [bufferSize, inBytes = Bytes(inBytesView.begin(), inBytesView.end()), &pos]
            {
                const auto len = static_cast<int64_t>(std::min<size_t>(bufferSize, inBytes.size() - pos));
                Bytes bytes(inBytes.data() + pos, inBytes.data() + pos + len);
                pos += len;
                return bytes;
            },
            [&outBytes](const Bytes &bytes)
            {
                outBytes.insert(outBytes.end(), bytes.begin(), bytes.end());
            }
        ))
        {
            return {};
        }
        return outBytes;
    }
}

namespace DynXX::Core::Z {

template <typename T>
int ZBase<T>::windowBits() const
{
    constexpr auto HEX = 16;
    if (this->format == GZip)
    {
        return HEX | MAX_WBITS;
    }
    if (this->format == Raw)
    {
        return -MAX_WBITS;
    }
    return MAX_WBITS;
}

template <typename T>
ZBase<T>::ZBase(size_t bufferSize, DynXXZFormatX format) : bufferSize{bufferSize}, format{format}
{
    if (bufferSize == 0) [[unlikely]]
    {
        throw std::invalid_argument("bufferSize invalid");
    }
    if (format != Raw && format != ZLib && format != GZip) [[unlikely]]
    {
        throw std::invalid_argument("format invalid");
    }
    this->inBuffer.resize(bufferSize, 0);
    this->outBuffer.resize(bufferSize, 0);
}

template <typename T>
size_t ZBase<T>::input(BytesView bytes, bool finish)
{
    if (bytes.empty()) [[unlikely]]
    {
        return 0;
    }
    const auto dataLen = std::min<size_t>(bytes.size(), this->bufferSize);

    this->inBuffer.resize(dataLen, 0);
    copyRange(bytes, this->inBuffer, dataLen);

    this->zs.avail_in = static_cast<unsigned int>(dataLen);
    this->zs.next_in = this->inBuffer.data();
    this->inFinish = finish;
    return dataLen;
}

template <typename T>
Bytes ZBase<T>::processDo()
{
    this->outBuffer.resize(this->bufferSize, 0);
    this->zs.avail_out = static_cast<unsigned int>(this->bufferSize);
    this->zs.next_out = this->outBuffer.data();

    static_cast<T *>(this)->processImp();

    if (this->ret != Z_OK && this->ret != Z_STREAM_END) [[unlikely]]
    {
        dynxxLogPrintF(Error, "z process error:{}", this->ret);
        return {};
    }

    const auto outLen = this->bufferSize - (this->zs).avail_out;
    return makeBytes(this->outBuffer.data(), outLen);
}

template <typename T>
bool ZBase<T>::processFinished() const
{
    return this->zs.avail_out != 0;
}

// Explicit template instantiation
template class ZBase<Zip>;
template class ZBase<UnZip>;

Zip::Zip(DynXXZipCompressModeX mode, size_t bufferSize, DynXXZFormatX format) : ZBase(bufferSize, format)
{
    if (mode != Default && mode != PreferSize && mode != PreferSpeed) [[unlikely]]
    {
        throw std::invalid_argument("mode invalid");
    }
    this->ret = deflateInit2(&(this->zs), underlying(mode), Z_DEFLATED, this->windowBits(), 8, Z_DEFAULT_STRATEGY);
    if (this->ret != Z_OK) [[unlikely]]
    {
        dynxxLogPrintF(Error, "deflateInit error:{}", this->ret);
        throw ZException(this->ret, "deflateInit failed");
    }
}

void Zip::processImp()
{
    this->ret = deflate(&(this->zs), this->inFinish ? Z_FINISH : Z_NO_FLUSH);
}

Zip::~Zip()
{
    deflateEnd(&(this->zs));
}

UnZip::UnZip(size_t bufferSize, DynXXZFormatX format) : ZBase(bufferSize, format)
{
    this->ret = inflateInit2(&this->zs, this->windowBits());
    if (this->ret != Z_OK) [[unlikely]]
    {
        dynxxLogPrintF(Error, "inflateInit error:{}", this->ret);
        throw ZException(this->ret, "inflateInit failed");
    }
}

void UnZip::processImp()
{
    this->ret = inflate(&(this->zs), Z_NO_FLUSH);
}

UnZip::~UnZip()
{
    inflateEnd(&this->zs);
}

#if !defined(__EMSCRIPTEN__)

// Cxx stream

bool zip(DynXXZipCompressModeX mode, size_t bufferSize, DynXXZFormatX format, std::istream *inStream, std::ostream *outStream)
{
    Zip zip(mode, bufferSize, format);
    return processCxxStream(bufferSize, inStream, outStream, zip);
}

bool unzip(size_t bufferSize, DynXXZFormatX format, std::istream *inStream, std::ostream *outStream)
{
    UnZip unzip(bufferSize, format);
    return processCxxStream(bufferSize, inStream, outStream, unzip);
}

// C FILE

bool zip(DynXXZipCompressModeX mode, size_t bufferSize, DynXXZFormatX format, std::FILE *inFile, std::FILE *outFile)
{
    Zip zip(mode, bufferSize, format);
    return processCFILE(bufferSize, inFile, outFile, zip);
}

bool unzip(size_t bufferSize, DynXXZFormatX format, std::FILE *inFile, std::FILE *outFile)
{
    UnZip unzip(bufferSize, format);
    return processCFILE(bufferSize, inFile, outFile, unzip);
}

#endif

// Bytes

Bytes zip(DynXXZipCompressModeX mode, size_t bufferSize, DynXXZFormatX format, BytesView bytes)
{
    Zip zip(mode, bufferSize, format);
    return processBytes(bufferSize, bytes, zip);
}

Bytes unzip(size_t bufferSize, DynXXZFormatX format, BytesView bytes)
{
    UnZip unzip(bufferSize, format);
    return processBytes(bufferSize, bytes, unzip);
}

} // namespace DynXX::Core::Z
