#ifndef NGENXX_INCLUDE_JS_HXX_
#define NGENXX_INCLUDE_JS_HXX_

#include "NGenXXTypes.h"

#include <functional>

bool ngenxxJsLoadF(const std::string &file);

bool ngenxxJsLoadS(const std::string &script, const std::string &name);

bool ngenxxJsLoadB(const Bytes bytes);

const std::string ngenxxJsCall(const std::string &func, const std::string &params);

void ngenxxJsSetMsgCallback(const std::function<const char *(const char *msg)> &callback);

#endif //  NGENXX_INCLUDE_JS_H_