# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

if("/Users/izzattillakhodza/Desktop/myprojects/SentinelX/LIEF/third-party//mbedtls-3.6.4.zip" STREQUAL "")
  message(FATAL_ERROR "LOCAL can't be empty")
endif()

if(NOT EXISTS "/Users/izzattillakhodza/Desktop/myprojects/SentinelX/LIEF/third-party//mbedtls-3.6.4.zip")
  message(FATAL_ERROR "File not found: /Users/izzattillakhodza/Desktop/myprojects/SentinelX/LIEF/third-party//mbedtls-3.6.4.zip")
endif()

if("SHA256" STREQUAL "")
  message(WARNING "File cannot be verified since no URL_HASH specified")
  return()
endif()

if("b0bf9e18ed94a74a203e5a7c9c4ecf24187da65d04242df243128697ec7d6f57" STREQUAL "")
  message(FATAL_ERROR "EXPECT_VALUE can't be empty")
endif()

message(VERBOSE "verifying file...
     file='/Users/izzattillakhodza/Desktop/myprojects/SentinelX/LIEF/third-party//mbedtls-3.6.4.zip'")

file("SHA256" "/Users/izzattillakhodza/Desktop/myprojects/SentinelX/LIEF/third-party//mbedtls-3.6.4.zip" actual_value)

if(NOT "${actual_value}" STREQUAL "b0bf9e18ed94a74a203e5a7c9c4ecf24187da65d04242df243128697ec7d6f57")
  message(FATAL_ERROR "error: SHA256 hash of
  /Users/izzattillakhodza/Desktop/myprojects/SentinelX/LIEF/third-party//mbedtls-3.6.4.zip
does not match expected value
  expected: 'b0bf9e18ed94a74a203e5a7c9c4ecf24187da65d04242df243128697ec7d6f57'
    actual: '${actual_value}'
")
endif()

message(VERBOSE "verifying file... done")
