# Install script for directory: D:/code/C++/HealthApp/DB/mariadb/include

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "C:/Program Files (x86)/mariadb")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "RelWithDebInfo")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "FALSE")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xDevelopmentx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/mariadb" TYPE FILE FILES
    "D:/code/C++/HealthApp/DB/mariadb/include/mariadb_com.h"
    "D:/code/C++/HealthApp/DB/mariadb/include/mysql.h"
    "D:/code/C++/HealthApp/DB/mariadb/include/mariadb_stmt.h"
    "D:/code/C++/HealthApp/DB/mariadb/include/ma_pvio.h"
    "D:/code/C++/HealthApp/DB/mariadb/include/ma_tls.h"
    "D:/code/C++/HealthApp/DB/libmariadb/codeblock/include/mariadb_version.h"
    "D:/code/C++/HealthApp/DB/mariadb/include/ma_list.h"
    "D:/code/C++/HealthApp/DB/mariadb/include/errmsg.h"
    "D:/code/C++/HealthApp/DB/mariadb/include/mariadb_dyncol.h"
    "D:/code/C++/HealthApp/DB/mariadb/include/mariadb_ctype.h"
    "D:/code/C++/HealthApp/DB/mariadb/include/mariadb_rpl.h"
    "D:/code/C++/HealthApp/DB/mariadb/include/mysqld_error.h"
    )
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xDevelopmentx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/mariadb/mysql" TYPE FILE FILES
    "D:/code/C++/HealthApp/DB/mariadb/include/mysql/client_plugin.h"
    "D:/code/C++/HealthApp/DB/mariadb/include/mysql/plugin_auth_common.h"
    "D:/code/C++/HealthApp/DB/mariadb/include/mysql/plugin_auth.h"
    )
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xDevelopmentx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/mariadb/mariadb" TYPE FILE FILES "D:/code/C++/HealthApp/DB/mariadb/include/mariadb/ma_io.h")
endif()

