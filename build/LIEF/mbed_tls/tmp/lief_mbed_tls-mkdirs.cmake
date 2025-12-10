# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

# If CMAKE_DISABLE_SOURCE_CHANGES is set to true and the source directory is an
# existing directory in our source tree, calling file(MAKE_DIRECTORY) on it
# would cause a fatal error, even though it would be a no-op.
if(NOT EXISTS "/Users/izzattillakhodza/Desktop/myprojects/SentinelX/build/LIEF/mbed_src")
  file(MAKE_DIRECTORY "/Users/izzattillakhodza/Desktop/myprojects/SentinelX/build/LIEF/mbed_src")
endif()
file(MAKE_DIRECTORY
  "/Users/izzattillakhodza/Desktop/myprojects/SentinelX/build/LIEF/mbed_tls/src/lief_mbed_tls-build"
  "/Users/izzattillakhodza/Desktop/myprojects/SentinelX/build/LIEF/mbed_tls"
  "/Users/izzattillakhodza/Desktop/myprojects/SentinelX/build/LIEF/mbed_tls/tmp"
  "/Users/izzattillakhodza/Desktop/myprojects/SentinelX/build/LIEF/mbed_tls/src/lief_mbed_tls-stamp"
  "/Users/izzattillakhodza/Desktop/myprojects/SentinelX/build/LIEF/mbed_tls/src"
  "/Users/izzattillakhodza/Desktop/myprojects/SentinelX/build/LIEF/mbed_tls/src/lief_mbed_tls-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "/Users/izzattillakhodza/Desktop/myprojects/SentinelX/build/LIEF/mbed_tls/src/lief_mbed_tls-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "/Users/izzattillakhodza/Desktop/myprojects/SentinelX/build/LIEF/mbed_tls/src/lief_mbed_tls-stamp${cfgdir}") # cfgdir has leading slash
endif()
