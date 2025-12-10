# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

# If CMAKE_DISABLE_SOURCE_CHANGES is set to true and the source directory is an
# existing directory in our source tree, calling file(MAKE_DIRECTORY) on it
# would cause a fatal error, even though it would be a no-op.
if(NOT EXISTS "/Users/izzattillakhodza/Desktop/myprojects/SentinelX/build/LIEF/lief_libjson-prefix/src/lief_libjson")
  file(MAKE_DIRECTORY "/Users/izzattillakhodza/Desktop/myprojects/SentinelX/build/LIEF/lief_libjson-prefix/src/lief_libjson")
endif()
file(MAKE_DIRECTORY
  "/Users/izzattillakhodza/Desktop/myprojects/SentinelX/build/LIEF/lief_libjson-prefix/src/lief_libjson-build"
  "/Users/izzattillakhodza/Desktop/myprojects/SentinelX/build/LIEF/lief_libjson-prefix"
  "/Users/izzattillakhodza/Desktop/myprojects/SentinelX/build/LIEF/lief_libjson-prefix/tmp"
  "/Users/izzattillakhodza/Desktop/myprojects/SentinelX/build/LIEF/lief_libjson-prefix/src/lief_libjson-stamp"
  "/Users/izzattillakhodza/Desktop/myprojects/SentinelX/build/LIEF/lief_libjson-prefix/src"
  "/Users/izzattillakhodza/Desktop/myprojects/SentinelX/build/LIEF/lief_libjson-prefix/src/lief_libjson-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "/Users/izzattillakhodza/Desktop/myprojects/SentinelX/build/LIEF/lief_libjson-prefix/src/lief_libjson-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "/Users/izzattillakhodza/Desktop/myprojects/SentinelX/build/LIEF/lief_libjson-prefix/src/lief_libjson-stamp${cfgdir}") # cfgdir has leading slash
endif()
