# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

if("/Users/izzattillakhodza/Desktop/myprojects/SentinelX/LIEF/third-party//utfcpp-4.0.6.zip" STREQUAL "")
  message(FATAL_ERROR "LOCAL can't be empty")
endif()

if(NOT EXISTS "/Users/izzattillakhodza/Desktop/myprojects/SentinelX/LIEF/third-party//utfcpp-4.0.6.zip")
  message(FATAL_ERROR "File not found: /Users/izzattillakhodza/Desktop/myprojects/SentinelX/LIEF/third-party//utfcpp-4.0.6.zip")
endif()

if("SHA256" STREQUAL "")
  message(WARNING "File cannot be verified since no URL_HASH specified")
  return()
endif()

if("9633e55568da90a98dcc221245370279191754bf298804b772b3f2d1aaec3136" STREQUAL "")
  message(FATAL_ERROR "EXPECT_VALUE can't be empty")
endif()

message(VERBOSE "verifying file...
     file='/Users/izzattillakhodza/Desktop/myprojects/SentinelX/LIEF/third-party//utfcpp-4.0.6.zip'")

file("SHA256" "/Users/izzattillakhodza/Desktop/myprojects/SentinelX/LIEF/third-party//utfcpp-4.0.6.zip" actual_value)

if(NOT "${actual_value}" STREQUAL "9633e55568da90a98dcc221245370279191754bf298804b772b3f2d1aaec3136")
  message(FATAL_ERROR "error: SHA256 hash of
  /Users/izzattillakhodza/Desktop/myprojects/SentinelX/LIEF/third-party//utfcpp-4.0.6.zip
does not match expected value
  expected: '9633e55568da90a98dcc221245370279191754bf298804b772b3f2d1aaec3136'
    actual: '${actual_value}'
")
endif()

message(VERBOSE "verifying file... done")
