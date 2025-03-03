cmake_minimum_required(VERSION 3.30)

project(uniopen)

if(NOT MINGW)
  message(FATAL_ERROR "Only MinGW is supported")
endif()

cmake_minimum_required(VERSION 3.30)

project(uniopen)

if(NOT MINGW)
  message(FATAL_ERROR "Only MinGW is supported")
endif()

file(READ "${CMAKE_SOURCE_DIR}/VERSION" FILE_VERSION)

set(PATCH_DIR "${CMAKE_SOURCE_DIR}/patch")
set(OUTPUT_DIR "${CMAKE_SOURCE_DIR}/out")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${OUTPUT_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${OUTPUT_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_DIR})
set(LIBMAGIC_SOURCE_FILES
  file/src/apprentice.c
  file/src/apptype.c
  file/src/ascmagic.c
  file/src/asctime_r.c
  file/src/asprintf.c
  file/src/buffer.c
  file/src/cdf_time.c
  file/src/cdf.c
  file/src/compress.c
  file/src/ctime_r.c
  file/src/der.c
  file/src/dprintf.c
  file/src/encoding.c
  file/src/fmtcheck.c
  file/src/fsmagic.c
  file/src/funcs.c
  file/src/getline.c
  file/src/getopt_long.c
  file/src/gmtime_r.c
  file/src/is_csv.c
  file/src/is_json.c
  file/src/is_simh.c
  file/src/is_tar.c
  file/src/localtime_r.c
  file/src/magic.c
  file/src/pread.c
  file/src/print.c
  file/src/readcdf.c
  file/src/readelf.c
  file/src/seccomp.c
  file/src/softmagic.c
  file/src/strcasestr.c
  file/src/strlcat.c
  file/src/strlcpy.c
  file/src/vasprintf.c
)
set(LIBMAGIC_LIBS shlwapi pcre2-posix)
set(LIBMAGIC_DEFINITIONS -DHAVE_CONFIG_H -DVERSION="${FILE_VERSION}")
set(UNIOPEN_SOURCE_FILES
  src/uniopen.c
  src/uniopen.rc
  src/messages.c
)

# Replace the version in the magic.h.in and write it to magic.h
file(READ file/src/magic.h.in MAGIC_H_CONTENT)
string(REPLACE "." "" FILE_VERSION_WITHOUT_DOT "${FILE_VERSION}")
string(REPLACE "X.YY" ${FILE_VERSION_WITHOUT_DOT} MAGIC_H_CONTENT_NEW "${MAGIC_H_CONTENT}")
file(WRITE file/src/magic.h "${MAGIC_H_CONTENT_NEW}")

include_directories("${CMAKE_SOURCE_DIR}/include" pcre2/src file/src src)

set(PCRE2_BUILD_PCRE2GREP OFF CACHE BOOL "Build pcre2grep" FORCE)
set(PCRE2_BUILD_TESTS OFF CACHE BOOL "Build pcre2grep tests" FORCE)
add_subdirectory(pcre2 EXCLUDE_FROM_ALL)

add_library(magicstatic STATIC ${LIBMAGIC_SOURCE_FILES})
target_compile_definitions(magicstatic PUBLIC ${LIBMAGIC_DEFINITIONS})

add_executable(uniopen WIN32 ${UNIOPEN_SOURCE_FILES})
add_dependencies(uniopen pcre2-posix magicstatic)
target_include_directories(uniopen PRIVATE "${CMAKE_SOURCE_DIR}")
target_link_libraries(uniopen magicstatic ${LIBMAGIC_LIBS})
set_target_properties(uniopen PROPERTIES LINK_FLAGS "-municode")

execute_process(
  WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/file"
  COMMAND git apply "${PATCH_DIR}/cdf_h.patch"
  COMMAND git apply "${PATCH_DIR}/apprentice_c.patch"
)
