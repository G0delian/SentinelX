# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

if("/Users/izzattillakhodza/Desktop/myprojects/SentinelX/LIEF/third-party//json-3.12.0.zip" STREQUAL "")
  message(FATAL_ERROR "LOCAL can't be empty")
endif()

if(NOT EXISTS "/Users/izzattillakhodza/Desktop/myprojects/SentinelX/LIEF/third-party//json-3.12.0.zip")
  message(FATAL_ERROR "File not found: /Users/izzattillakhodza/Desktop/myprojects/SentinelX/LIEF/third-party//json-3.12.0.zip")
endif()

if("SHA256" STREQUAL "")
  message(WARNING "File cannot be verified since no URL_HASH specified")
  return()
endif()

if("6a2249e5d61c2a8351abfac218e08e9a43426dddb493950d30f3b8acfbbc648d" STREQUAL "")
  message(FATAL_ERROR "EXPECT_VALUE can't be empty")
endif()

message(VERBOSE "verifying file...
     file='/Users/izzattillakhodza/Desktop/myprojects/SentinelX/LIEF/third-party//json-3.12.0.zip'")

file("SHA256" "/Users/izzattillakhodza/Desktop/myprojects/SentinelX/LIEF/third-party//json-3.12.0.zip" actual_value)

if(NOT "${actual_value}" STREQUAL "6a2249e5d61c2a8351abfac218e08e9a43426dddb493950d30f3b8acfbbc648d")
  message(FATAL_ERROR "error: SHA256 hash of
  /Users/izzattillakhodza/Desktop/myprojects/SentinelX/LIEF/third-party//json-3.12.0.zip
does not match expected value
  expected: '6a2249e5d61c2a8351abfac218e08e9a43426dddb493950d30f3b8acfbbc648d'
    actual: '${actual_value}'
")
endif()

message(VERBOSE "verifying file... done")
