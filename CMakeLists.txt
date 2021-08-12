# Copyright (c) 2021 Ably Medical

if (NOT CONFIG_SENBRD_PROTOCOL)

  cmake_minimum_required(VERSION 3.13.1)
  project(
    SenbrdProtocol
    VERSION 0.1
    LANGUAGES C CXX
  )

  # set C/C++ standards
  set(CMAKE_C_STANDARD 99)
  set(CMAKE_C_STANDARD_REQUIRED true)
  set(CMAKE_CXX_STANDARD 11)
  set(CMAKE_CXX_STANDARD_REQUIRED true)
  add_compile_options(-Wall -Wextra -Werror)

  # set kconfig variables, can be overriden on commandline
  add_compile_definitions(CONFIG_BIG_ENDIAN=y)
  add_compile_definitions(CONFIG_PROTOCOL_MAX_PIEZO_CHANNELS=4)
  add_compile_definitions(CONFIG_PROTOCOL_MAX_PIEZO_SAMPLE_SETS_PER_MSG=10)

  # create actual library
  add_library(SenbrdProtocol src/message.c)
  target_include_directories(SenbrdProtocol PUBLIC include/)

else()

  add_library(SenbrdProtocol INTERFACE)
  target_link_libraries(SenbrdProtocol INTERFACE zephyr_interface)
  target_include_directories(SenbrdProtocol INTERFACE include)

  zephyr_library()
  zephyr_library_sources(src/message.c)
  zephyr_library_link_libraries(SenbrdProtocol)
  zephyr_include_directories(include)

endif()
