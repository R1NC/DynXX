#ifndef NGENXX_ZIP_HXX_
#define NGENXX_ZIP_HXX_

#include <zlib.h>

#ifdef __cplusplus

#include "../../include/NGenXXZip.h"

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
            const size input(const byte *in, const size inLen, bool inFinish);
            const byte *processDo(size *outLen);
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
    }
}

#endif

#endif // NGENXX_ZIP_HXX_