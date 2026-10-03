function(dynxx_declare_observability_options)
    # Build-time instrumentation: the CMake Instrumentation API v1 records every
    # configure/generate/build/install command with its timing and can emit a Google
    # Trace File (open it in Perfetto or chrome://tracing). Every preset turns it on
    # (measured: build time within noise, and the produced object files are byte
    # identical, so nothing reaches the compiled code); -DDYNXX_ENABLE_BUILD_INSTRUMENTATION=OFF
    # switches it off for a single configure.
    # Data lands in <build>/.cmake/instrumentation/v1/data/ (trace/ holds the Google
    # Trace File produced by the postCMakeBuild hook, index/ and content/ the raw
    # snippet data).
    option(DYNXX_ENABLE_BUILD_INSTRUMENTATION "Collect CMake Instrumentation API data for this build" OFF)
endfunction()

function(dynxx_configure_observability)
    if(NOT DYNXX_ENABLE_BUILD_INSTRUMENTATION)
        return()
    endif()

    # cmake_instrumentation() only exists from CMake 4.3 (CI and the Docker images run
    # 3.28.3) and the API only covers Makefiles/Ninja/FASTBuild generators (the Xcode
    # and Visual Studio Debug presets fall outside it), so report the reason instead of
    # silently producing no data.
    if(NOT COMMAND cmake_instrumentation)
        message(STATUS "Observability: DYNXX_ENABLE_BUILD_INSTRUMENTATION needs CMake >= 4.3, ignored on ${CMAKE_VERSION}")
        return()
    endif()
    if(NOT CMAKE_GENERATOR MATCHES "(Ninja|Makefiles|FASTBuild)")
        message(STATUS "Observability: generator `${CMAKE_GENERATOR}` cannot be instrumented (Makefiles/Ninja/FASTBuild only)")
        return()
    endif()

    # A single hook after the build, so one trace file covers configure+generate+build.
    # DATA_VERSION 1 is the maximum CMake 4.3 accepts; 4.4+ upgrades to 1.1 by itself,
    # which is what adds the captureOutput/compileTrace options.
    cmake_instrumentation(
        API_VERSION 1
        DATA_VERSION 1
        HOOKS postCMakeBuild
        OPTIONS trace
    )
endfunction()
