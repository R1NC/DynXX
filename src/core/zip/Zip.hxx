#ifndef DYNXX_SRC_CORE_ZIP_HXX_
#define DYNXX_SRC_CORE_ZIP_HXX_

#include <zlib.h>

#if defined(__cplusplus)

#include <istream>
#include <ostream>

#include <DynXX/CXX/Types.hxx>

namespace DynXX::Core::Z {
    template<typename T>
    class ZBase {
    public:
        ZBase() = delete;

        explicit ZBase(size_t bufferSize, int format);

        ZBase(const ZBase &) = delete;

        ZBase &operator=(const ZBase &) = delete;

        ZBase(ZBase &&) = delete;

        ZBase &operator=(ZBase &&) = delete;

        virtual ~ZBase();

        size_t input(const Bytes &bytes, bool inFinish);

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
        const int format;
    };

    class Zip final : public ZBase<Zip> {
    public:
        Zip() = delete;

        explicit Zip(int mode, size_t bufferSize, int format) noexcept(false);

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

        explicit UnZip(size_t bufferSize, int format) noexcept(false);

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

    bool zip(int mode, size_t bufferSize, int format, std::istream *inStream, std::ostream *outStream);

    bool unzip(size_t bufferSize, int format, std::istream *inStream, std::ostream *outStream);

    bool zip(int mode, size_t bufferSize, int format, std::FILE *inFile, std::FILE *outFile);

    bool unzip(size_t bufferSize, int format, std::FILE *inFile, std::FILE *outFile);

#endif

    Bytes zip(int mode, size_t bufferSize, int format, const Bytes &bytes);

    Bytes unzip(size_t bufferSize, int format, const Bytes &bytes);
}

#endif

#endif // DYNXX_SRC_CORE_ZIP_HXX_
