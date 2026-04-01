#pragma once

#if defined(__APPLE__)
//  #import <Foundation/Foundation.h>
#elif defined(_WIN32)
  #if !defined(WIN32_LEAN_AND_MEAN)
      #define WIN32_LEAN_AND_MEAN
  #endif
  #if !defined(NOMINMAX)
      #define NOMINMAX
  #endif
  #include <windows.h>
#elif defined(__EMSCRIPTEN__)
  #include <emscripten/emscripten.h>
#endif

#include "DynXX/C/Macro.h"
#include "DynXX/CXX/Memory.hxx"
#include "DynXX/CXX/Log.hxx"
