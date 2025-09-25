#pragma once

#include <zlib.h>

#include <istream>
#include <ostream>

#include <DynXX/CXX/Types.hxx>
#include <DynXX/CXX/Zip.hxx>

namespace DynXX::Core::Z {
    
    class ZException : public std::runtime_error {
    public:
        const int ret{Z_OK};
        explicit ZException(int ret, const std::string& msg): std::runtime_error(msg), ret(ret) {}
    };
    
    template<typename T>
    class ZBase {
    public:
        ZBase() = delete;

        explicit ZBase(size_t bufferSize, DynXXZFormatX format);

        ZBase(const ZBase &) = delete;

        ZBase &operator=(const ZBase &) = delete;

        ZBase(ZBase &&) = delete;

        ZBase &operator=(ZBase &&) = delete;

        virtual ~ZBase();

        size_t input(const Bytes &bytes, bool finish);

        Bytes processDo();

        [[nodiscard]] bool processFinished() const;

    protected:
        z_stream zs{Z_NULL, Z_NULL, Z_NULL};
        int ret{Z_OK};
        bool inFinish{false};

        int windowBits() const;

    private:
        byte *inBuffer{nullptr};
        byte *outBuffer{nullptr};
        const size_t bufferSize;
        const DynXXZFormatX format;
    };

    class Zip final : public ZBase<Zip> {
    public:
        Zip() = delete;

        /// @throws `std::invalid_argument` & `ZException`
        explicit Zip(DynXXZipCompressModeX mode, size_t bufferSize, DynXXZFormatX format) noexcept(false);

        Zip(const Zip &) = delete;

        Zip &operator=(const Zip &) = delete;

        Zip(Zip &&) = delete;

        Zip &operator=(Zip &&) = delete;

        ~Zip() override;

    private:
        friend class ZBase<Zip>;

        void processImp();
    };

    class UnZip final : public ZBase<UnZip> {
    public:
        UnZip() = delete;

        /// @throws `std::invalid_argument` & `ZException`
        explicit UnZip(size_t bufferSize, DynXXZFormatX format) noexcept(false);

        UnZip(const UnZip &) = delete;

        UnZip &operator=(const UnZip &) = delete;

        UnZip(UnZip &&) = delete;

        UnZip &operator=(UnZip &&) = delete;

        ~UnZip() override;

    private:
        friend class ZBase<UnZip>;

        void processImp();
    };

#if !defined(__EMSCRIPTEN__)

    bool zip(DynXXZipCompressModeX mode, size_t bufferSize, DynXXZFormatX format, std::istream *inStream, std::ostream *outStream);

    bool unzip(size_t bufferSize, DynXXZFormatX format, std::istream *inStream, std::ostream *outStream);

    bool zip(DynXXZipCompressModeX mode, size_t bufferSize, DynXXZFormatX format, std::FILE *inFile, std::FILE *outFile);

    bool unzip(size_t bufferSize, DynXXZFormatX format, std::FILE *inFile, std::FILE *outFile);

#endif

    Bytes zip(DynXXZipCompressModeX mode, size_t bufferSize, DynXXZFormatX format, const Bytes &bytes);

    Bytes unzip(size_t bufferSize, DynXXZFormatX format, const Bytes &bytes);
}
