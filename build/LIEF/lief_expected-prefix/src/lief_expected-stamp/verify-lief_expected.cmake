# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

if("/Users/izzattillakhodza/Desktop/myprojects/SentinelX/LIEF/third-party//expected-1.2.0.zip" STREQUAL "")
  message(FATAL_ERROR "LOCAL can't be empty")
endif()

if(NOT EXISTS "/Users/izzattillakhodza/Desktop/myprojects/SentinelX/LIEF/third-party//expected-1.2.0.zip")
  message(FATAL_ERROR "File not found: /Users/izzattillakhodza/Desktop/myprojects/SentinelX/LIEF/third-party//expected-1.2.0.zip")
endif()

if("SHA256" STREQUAL "")
  message(WARNING "File cannot be verified since no URL_HASH specified")
  return()
endif()

if("a3031c888eb4f891d6df8d695c9b939c8b0ff815f9daafb378d6ab06d2a40937" STREQUAL "")
  message(FATAL_ERROR "EXPECT_VALUE can't be empty")
endif()

message(VERBOSE "verifying file...
     file='/Users/izzattillakhodza/Desktop/myprojects/SentinelX/LIEF/third-party//expected-1.2.0.zip'")

file("SHA256" "/Users/izzattillakhodza/Desktop/myprojects/SentinelX/LIEF/third-party//expected-1.2.0.zip" actual_value)

if(NOT "${actual_value}" STREQUAL "a3031c888eb4f891d6df8d695c9b939c8b0ff815f9daafb378d6ab06d2a40937")
  message(FATAL_ERROR "error: SHA256 hash of
  /Users/izzattillakhodza/Desktop/myprojects/SentinelX/LIEF/third-party//expected-1.2.0.zip
does not match expected value
  expected: 'a3031c888eb4f891d6df8d695c9b939c8b0ff815f9daafb378d6ab06d2a40937'
    actual: '${actual_value}'
")
endif()

message(VERBOSE "verifying file... done")
