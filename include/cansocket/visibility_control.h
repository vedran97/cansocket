#ifndef CANSOCKET__VISIBILITY_CONTROL_H_
#define CANSOCKET__VISIBILITY_CONTROL_H_

// This logic was borrowed (then namespaced) from the examples on the gcc wiki:
//     https://gcc.gnu.org/wiki/Visibility

#if defined _WIN32 || defined __CYGWIN__
  #ifdef __GNUC__
    #define CANSOCKET_EXPORT __attribute__ ((dllexport))
    #define CANSOCKET_IMPORT __attribute__ ((dllimport))
  #else
    #define CANSOCKET_EXPORT __declspec(dllexport)
    #define CANSOCKET_IMPORT __declspec(dllimport)
  #endif
  #ifdef CANSOCKET_BUILDING_LIBRARY
    #define CANSOCKET_PUBLIC CANSOCKET_EXPORT
  #else
    #define CANSOCKET_PUBLIC CANSOCKET_IMPORT
  #endif
  #define CANSOCKET_PUBLIC_TYPE CANSOCKET_PUBLIC
  #define CANSOCKET_LOCAL
#else
  #define CANSOCKET_EXPORT __attribute__ ((visibility("default")))
  #define CANSOCKET_IMPORT
  #if __GNUC__ >= 4
    #define CANSOCKET_PUBLIC __attribute__ ((visibility("default")))
    #define CANSOCKET_LOCAL  __attribute__ ((visibility("hidden")))
  #else
    #define CANSOCKET_PUBLIC
    #define CANSOCKET_LOCAL
  #endif
  #define CANSOCKET_PUBLIC_TYPE
#endif

#endif  // CANSOCKET__VISIBILITY_CONTROL_H_
