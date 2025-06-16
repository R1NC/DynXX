#ifndef NGENXX_INCLUDE_HXX_
#define NGENXX_INCLUDE_HXX_

#include "NGenXXNetHttp.hxx"
#include "NGenXXStoreKV.hxx"
#include "NGenXXStoreSQLite.hxx"
#include "NGenXXLua.hxx"
#include "NGenXXJS.hxx"
#include "NGenXXLog.hxx"
#include "NGenXXJson.hxx"
#include "NGenXXCoding.hxx"
#include "NGenXXCrypto.hxx"
#include "NGenXXZip.hxx"
#include "NGenXXDevice.hxx"

std::string ngenxxGetVersion();

bool ngenxxInit(const std::string &root);

std::string ngenxxRootPath();

void ngenxxRelease();

#endif // NGENXX_INCLUDE_HXX_
