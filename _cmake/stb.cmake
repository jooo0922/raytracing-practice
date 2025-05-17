cmake_minimum_required(VERSION 3.18)

include(FetchContent)
Set(FETCHCONTENT_QUIET FALSE)

FetchContent_Declare(stb_image
  GIT_REPOSITORY https://github.com/nothings/stb.git
  GIT_PROGRESS TRUE
  GIT_TAG f4a71b13373436a2866c5d68f8f80ac6f0bc1ffe)

FetchContent_GetProperties(stb_image)

if(NOT stb_image_POPULATED)
  FetchContent_MakeAvailable(stb_image)
endif()

set(stb_INCLUDE ${stb_image_SOURCE_DIR})

message(STATUS "STB Should Be Downloaded")
