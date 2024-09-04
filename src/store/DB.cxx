#ifdef __cplusplus

#include <string>
#ifdef __APPLE__
#include <sqlite3.h>
#else
#include "../../../external/sqlite/sqlite3.h"
#endif

namespace NGenXX
{
    namespace Store
    {
        namespace DB
        {

            void *open(const char *file)
            {
                sqlite3 *db = NULL;
                sqlite3_open(file, &db);
                return db ?: NULL;
            }

            void *queryExe(void *db, const char *sql)
            {
                sqlite3_stmt *stmt;
                sqlite3_prepare_v2((sqlite3 *)db, sql, -1, &stmt, NULL);
                return stmt;
            }

            bool queryReadRow(void *query_result)
            {
                sqlite3_stmt *stmt = (sqlite3_stmt *)query_result;
                return sqlite3_step(stmt) == SQLITE_ROW;
            }

            const char *queryReadColumnText(void *query_result, const char *column)
            {
                sqlite3_stmt *stmt = (sqlite3_stmt *)query_result;
                for (int i = 0; i < sqlite3_column_count(stmt); i++)
                {
                    if (strcmp(sqlite3_column_name(stmt, i), column) == 0 && sqlite3_column_type(stmt, i) == SQLITE_TEXT)
                    {
                        return (const char *)sqlite3_column_text(stmt, i);
                    }
                }
                return NULL;
            }

            long long queryReadColumnInteger(void *query_result, const char *column)
            {
                sqlite3_stmt *stmt = (sqlite3_stmt *)query_result;
                for (int i = 0; i < sqlite3_column_count(stmt); i++)
                {
                    if (strcmp(sqlite3_column_name(stmt, i), column) == 0 && sqlite3_column_type(stmt, i) == SQLITE_INTEGER)
                    {
                        return sqlite3_column_int64(stmt, i);
                    }
                }
                return NULL;
            }

            double queryReadColumnFloat(void *query_result, const char *column)
            {
                sqlite3_stmt *stmt = (sqlite3_stmt *)query_result;
                for (int i = 0; i < sqlite3_column_count(stmt); i++)
                {
                    if (strcmp(sqlite3_column_name(stmt, i), column) == 0 && sqlite3_column_type(stmt, i) == SQLITE_FLOAT)
                    {
                        return sqlite3_column_double(stmt, i);
                    }
                }
                return NULL;
            }

            void queryDrop(void *query_result)
            {
                sqlite3_finalize((sqlite3_stmt *)query_result);
            }

            void close(void *db)
            {
                sqlite3_close((sqlite3 *)db);
            }
        }
    }
}

#endif