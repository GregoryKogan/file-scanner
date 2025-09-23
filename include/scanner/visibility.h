#ifndef SCANNER_VISIBILITY_H_
#define SCANNER_VISIBILITY_H_

// Generic helper definitions for shared library support
#if defined _WIN32 || defined __CYGWIN__
#define SCANNER_HELPER_DLL_IMPORT __declspec(dllimport)
#define SCANNER_HELPER_DLL_EXPORT __declspec(dllexport)
#define SCANNER_HELPER_DLL_LOCAL
#else
#if __GNUC__ >= 4
#define SCANNER_HELPER_DLL_IMPORT __attribute__((visibility("default")))
#define SCANNER_HELPER_DLL_EXPORT __attribute__((visibility("default")))
#define SCANNER_HELPER_DLL_LOCAL __attribute__((visibility("hidden")))
#else
#define SCANNER_HELPER_DLL_IMPORT
#define SCANNER_HELPER_DLL_EXPORT
#define SCANNER_HELPER_DLL_LOCAL
#endif
#endif

// Now we use the generic helper definitions above to define SCANNER_API
// SCANNER_LIB_EXPORT is defined by CMake when we are building the shared
// library
#ifdef SCANNER_LIB_EXPORT
#define SCANNER_API SCANNER_HELPER_DLL_EXPORT
#else
#define SCANNER_API SCANNER_HELPER_DLL_IMPORT
#endif  // SCANNER_LIB_EXPORT

#define SCANNER_LOCAL SCANNER_HELPER_DLL_LOCAL

#endif  // SCANNER_VISIBILITY_H_