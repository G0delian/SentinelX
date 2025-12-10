# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

if("/Users/izzattillakhodza/Desktop/myprojects/SentinelX/LIEF/third-party//spdlog-1.15.3.zip" STREQUAL "")
  message(FATAL_ERROR "LOCAL can't be empty")
endif()

if(NOT EXISTS "/Users/izzattillakhodza/Desktop/myprojects/SentinelX/LIEF/third-party//spdlog-1.15.3.zip")
  message(FATAL_ERROR "File not found: /Users/izzattillakhodza/Desktop/myprojects/SentinelX/LIEF/third-party//spdlog-1.15.3.zip")
endif()

if("SHA256" STREQUAL "")
  message(WARNING "File cannot be verified since no URL_HASH specified")
  return()
endif()

if("b74274c32c8be5dba70b7006c1d41b7d3e5ff0dff8390c8b6390c1189424e094" STREQUAL "")
  message(FATAL_ERROR "EXPECT_VALUE can't be empty")
endif()

message(VERBOSE "verifying file...
     file='/Users/izzattillakhodza/Desktop/myprojects/SentinelX/LIEF/third-party//spdlog-1.15.3.zip'")

file("SHA256" "/Users/izzattillakhodza/Desktop/myprojects/SentinelX/LIEF/third-party//spdlog-1.15.3.zip" actual_value)

if(NOT "${actual_value}" STREQUAL "b74274c32c8be5dba70b7006c1d41b7d3e5ff0dff8390c8b6390c1189424e094")
  message(FATAL_ERROR "error: SHA256 hash of
  /Users/izzattillakhodza/Desktop/myprojects/SentinelX/LIEF/third-party//spdlog-1.15.3.zip
does not match expected value
  expected: 'b74274c32c8be5dba70b7006c1d41b7d3e5ff0dff8390c8b6390c1189424e094'
    actual: '${actual_value}'
")
endif()

message(VERBOSE "verifying file... done")
