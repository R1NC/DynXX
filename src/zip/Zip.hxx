#ifndef NGENXX_ZIP_HXX_
#define NGENXX_ZIP_HXX_

#ifdef __EMSCRIPTEN__
#include "../../../external/zlib/zlib.h"
#else
#include <zlib.h>
#endif

#ifdef __cplusplus

#include "../../include/NGenXXZip.h"
#include "../NGenXX-Types.hxx"
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
            size bufferSize;
            virtual void processImp() = 0;

        protected:
            z_stream zs;
            int ret;
            bool inFinish;

        public:
            ZBase() = delete;
            ZBase(const size bufferSize);
            const size input(const Bytes bytes, bool inFinish);
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
            Zip(int mode, const size bufferSize);
            ~Zip();
        };

        class UnZip : public ZBase
        {
        private:
            void processImp();

        public:
            UnZip() = delete;
            UnZip(const size bufferSize);
            ~UnZip();
        };

        bool zip(int mode, const size bufferSize, std::istream *inStream, std::ostream *outStream);

        bool unzip(const size bufferSize, std::istream *inStream, std::ostream *outStream);

        bool zip(int mode, const size bufferSize, std::FILE *inFile, std::FILE *outFile);

        bool unzip(const size bufferSize, std::FILE *inFile, std::FILE *outFile);
    }
}

#endif

#endif // NGENXX_ZIP_HXX_