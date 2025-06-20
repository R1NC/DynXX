#ifndef NGENXX_INCLUDE_JS_HXX_
#define NGENXX_INCLUDE_JS_HXX_

#include "NGenXXTypes.hxx"

#include <functional>

bool ngenxxJsLoadF(const std::string &file, bool isModule);

bool ngenxxJsLoadS(const std::string &script, const std::string &name, bool isModule);

bool ngenxxJsLoadB(const Bytes &bytes, bool isModule);

std::optional<std::string> ngenxxJsCall(const std::string &func, const std::string &params, bool await);

void ngenxxJsSetMsgCallback(const std::function<const char *(const char *msg)> &callback);

#endif // NGENXX_INCLUDE_JS_H_
