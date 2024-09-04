#ifndef NGENXX_STORE_DB_HXX_
#define NGENXX_STORE_DB_HXX_

namespace NGenXX
{
    namespace Store
    {
        namespace DB
        {
            /**
             *
             */
            void *open(const char *file);

            /**
             *
             */
            void *queryExe(void *db, const char *sql);

            /**
             *
             */
            bool queryReadRow(void *query_result);

            /**
             *
             */
            const char *queryReadColumnText(void *query_result, const char *column);

            /**
             *
             */
            long long queryReadColumnInteger(void *query_result, const char *column);

            /**
             *
             */
            double queryReadColumnFloat(void *query_result, const char *column);

            /**
             *
             */
            void queryDrop(void *query_result);

            /**
             *
             */
            void close(void *handle);
        }
    }
}

#endif // NGENXX_STORE_DB_HXX_