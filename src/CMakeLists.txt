include(CheckIncludeFiles)
check_include_files("sys/socket.h;linux/can.h" HAVE_CAN_H)
if(HAVE_CAN_H)
  message(STATUS "kernel support socket-can, building support for it")
  list(APPEND CAN_SOCKET_SOURCES DriverSocket.cpp)
  list(APPEND CAN_SOCKET_HEADERS DriverSocket.hpp)
  add_definitions(-DHAVE_CAN_H)
else()
  message(STATUS "kernel does not support socket-can")
endif()

rock_library(canbus
    SOURCES Driver.cpp 
        Driver2Web.cpp DriverHico.cpp DriverHicoPCI.cpp DriverNetGateway.cpp
        DriverSocket.cpp DriverEasySYNC.cpp ${CAN_SOCKET_SOURCES}
    HEADERS Driver.hpp Message.hpp
        Driver2Web.hpp DriverHico.hpp DriverHicoPCI.hpp DriverNetGateway.hpp
        DriverSocket.hpp DriverEasySYNC.hpp ${CAN_SOCKET_HEADERS}
    DEPS_PKGCONFIG base-types base-logging iodrivers_base)

rock_executable(canbus-easysync
    SOURCES tools/MainEasySYNC.cpp
    DEPS canbus)
rock_executable(canbus-monitor
    SOURCES tools/MainMonitor.cpp
    DEPS canbus)
rock_executable(hico_tool tools/hcantool.c)
rock_executable(canbus-reset tools/MainReset.cpp
    DEPS canbus)
rock_executable(canbus-send tools/MainSend.cpp
    DEPS canbus)

install(FILES tools/hico_monitor DESTINATION bin
    PERMISSIONS OWNER_WRITE OWNER_READ GROUP_READ WORLD_READ OWNER_EXECUTE GROUP_EXECUTE WORLD_EXECUTE)

install(FILES backwards/canmessage.hh backwards/canbus.hh
    DESTINATION include)
