PROJECT(canbus)
SET(PROJECT_VERSION 1.1)

CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
INCLUDE(FindPkgConfig)
INCLUDE(CheckIncludeFiles)

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
MACRO(CMAKE_USE_FULL_RPATH install_rpath)
    SET(CMAKE_SKIP_BUILD_RPATH  FALSE)
    SET(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE) 
    SET(CMAKE_INSTALL_RPATH ${install_rpath})
    SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
ENDMACRO(CMAKE_USE_FULL_RPATH)
CMAKE_USE_FULL_RPATH("${CMAKE_INSTALL_PREFIX}/lib")

pkg_check_modules(base_TYPES REQUIRED "base-types")
INCLUDE_DIRECTORIES(${base_TYPES_INCLUDE_DIRS})
LINK_DIRECTORIES(${base_TYPES_LIBRARY_DIRS})

pkg_check_modules(IO REQUIRED "iodrivers_base")
INCLUDE_DIRECTORIES(${IO_INCLUDE_DIRS})
LINK_DIRECTORIES(${IO_LIBRARY_DIRS})

include_directories(BEFORE ${CMAKE_SOURCE_DIR})

check_include_files("sys/socket.h;linux/can.h" HAVE_CAN_H)

if(HAVE_CAN_H)
  message(STATUS "kernel support socket-can, building support for it")
  SET(canSocketSource canbus-socket.cc)
  add_definitions(-DHAVE_CAN_H)
else()
  message(STATUS "kernel does not support socket-can")
endif()

ADD_LIBRARY(canbus SHARED canbus.cc canbus-hico.cc ${canSocketSource})
TARGET_LINK_LIBRARIES(canbus ${base_TYPES_LIBRARIES} ${IO_LIBRARIES})

ADD_EXECUTABLE(canbus_test canbus_test.cc)
TARGET_LINK_LIBRARIES(canbus_test canbus)

add_executable(hico_tool    tools/hcantool.c)
add_executable(canbus_reset tools/canbus_reset.cc)
target_link_libraries(canbus_reset canbus)

INSTALL(TARGETS canbus hico_tool canbus_reset
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib)
install(FILES tools/hico_monitor DESTINATION bin
    PERMISSIONS OWNER_WRITE OWNER_READ GROUP_READ WORLD_READ OWNER_EXECUTE GROUP_EXECUTE WORLD_EXECUTE)
INSTALL(FILES canbus.hh canmessage.hh DESTINATION include)

CONFIGURE_FILE(Doxyfile.in Doxyfile @ONLY)
ADD_CUSTOM_TARGET(doc doxygen Doxyfile)

CONFIGURE_FILE(canbus.pc.in canbus.pc @ONLY)
INSTALL(FILES ${CMAKE_BINARY_DIR}/canbus.pc DESTINATION lib/pkgconfig)

