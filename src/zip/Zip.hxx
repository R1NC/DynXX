#ifndef NGENXX_SRC_ZIP_HXX_
#define NGENXX_SRC_ZIP_HXX_

#include <zlib.h>

#if defined(__cplusplus)

#include <NGenXXZip.h>
#include <NGenXXTypes.hxx>
#include <istream>
#include <ostream>
#include <cstdio>

namespace NGenXX
{
    namespace Z
    {
        class ZBase
        {
        private:
            byte *inBuffer;
            byte *outBuffer;
            size_t bufferSize;
            int format;
            virtual void processImp() = 0;

        protected:
            z_stream zs;
            int ret;
            bool inFinish;
            int windowBits();

        public:
            ZBase() = delete;
            ZBase(const size_t bufferSize, const int format);
            const size_t input(const Bytes &bytes, bool inFinish);
            const Bytes processDo();
            const bool processFinished();
            virtual ~ZBase();
        };

        class Zip : public ZBase
        {
        private:
            void processImp();

        public:
            Zip() = delete;
            Zip(int mode, const size_t bufferSize, const int format);
            Zip(const Zip&) = delete;
            Zip& operator=(const Zip&) = delete;
            ~Zip();
        };

        class UnZip : public ZBase
        {
        private:
            void processImp();

        public:
            UnZip() = delete;
            UnZip(const size_t bufferSize, const int format);
            UnZip(const UnZip&) = delete;
            UnZip& operator=(const UnZip&) = delete;
            ~UnZip();
        };

        bool zip(int mode, const size_t bufferSize, const int format, std::istream *inStream, std::ostream *outStream);

        bool unzip(const size_t bufferSize, const int format, std::istream *inStream, std::ostream *outStream);

        bool zip(int mode, const size_t bufferSize, const int format, std::FILE *inFile, std::FILE *outFile);

        bool unzip(const size_t bufferSize, const int format, std::FILE *inFile, std::FILE *outFile);

        const Bytes zip(int mode, const size_t bufferSize, const int format, const Bytes &bytes);

        const Bytes unzip(const size_t bufferSize, const int format, const Bytes &bytes);
    }
}

#endif

#endif // NGENXX_SRC_ZIP_HXX_