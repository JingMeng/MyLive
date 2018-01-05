
cmake_minimum_required(VERSION 3.4.1)

SET(distribution_DIR ../../../../libs)

add_library( # Sets the name of the library.
             mylive

             # Sets the library as a shared library.
             SHARED

             # Provides a relative path to your source file(s).
             src/main/cpp/mylive.c )


find_library( # Sets the name of the path variable.
              log-lib

              # Specifies the name of the NDK library that
              # you want CMake to locate.
              log )

add_library( libx264
             STATIC
             IMPORTED )

set_target_properties( libx264
                      PROPERTIES IMPORTED_LOCATION
                      ${distribution_DIR}/armeabi/libx264.a )

add_library(libfaac
            STATIC
            IMPORTED)

set_target_properties( libfaac
                        PROPERTIES IMPORTED_LOCATION
                        ${dirtribution_DIR}/armeabi/libfaac.a)

add_library(librtmp
            STATIC
            IMPORTED)

set_target_properties( librtmp
                        PROPERTIES IMPORTED_LOCATION
                        ${dirtribution_DIR}/armeabi/librtmp.a)


include_directories(src/main/cpp/x264/include)
include_directories(src/main/cpp/rtmp/include)
include_directories(src/main/cpp/faac/include)

target_link_libraries( # Specifies the target library.
                       mylive
                       libx264

                       # Links the target library to the log library
                       # included in the NDK.
                       ${log-lib} )