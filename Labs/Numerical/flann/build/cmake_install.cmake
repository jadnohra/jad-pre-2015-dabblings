# Install script for directory: Z:/Personal/Lab/Physics/lamby/flann

# Set the install prefix
IF(NOT DEFINED CMAKE_INSTALL_PREFIX)
  SET(CMAKE_INSTALL_PREFIX "C:/Program Files (x86)/flann")
ENDIF(NOT DEFINED CMAKE_INSTALL_PREFIX)
STRING(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
IF(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  IF(BUILD_TYPE)
    STRING(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  ELSE(BUILD_TYPE)
    SET(CMAKE_INSTALL_CONFIG_NAME "Release")
  ENDIF(BUILD_TYPE)
  MESSAGE(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
ENDIF(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)

# Set the component getting installed.
IF(NOT CMAKE_INSTALL_COMPONENT)
  IF(COMPONENT)
    MESSAGE(STATUS "Install component: \"${COMPONENT}\"")
    SET(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  ELSE(COMPONENT)
    SET(CMAKE_INSTALL_COMPONENT)
  ENDIF(COMPONENT)
ENDIF(NOT CMAKE_INSTALL_COMPONENT)

IF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")
  FILE(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/bin" TYPE PROGRAM FILES
    "C:/Program Files (x86)/Microsoft Visual Studio 9.0/VC/redist/x86/Microsoft.VC90.CRT/Microsoft.VC90.CRT.manifest"
    "C:/Program Files (x86)/Microsoft Visual Studio 9.0/VC/redist/x86/Microsoft.VC90.CRT/msvcm90.dll"
    "C:/Program Files (x86)/Microsoft Visual Studio 9.0/VC/redist/x86/Microsoft.VC90.CRT/msvcp90.dll"
    "C:/Program Files (x86)/Microsoft Visual Studio 9.0/VC/redist/x86/Microsoft.VC90.CRT/msvcr90.dll"
    )
ENDIF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")

IF(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for each subdirectory.
  INCLUDE("Z:/Personal/Lab/Physics/lamby/flann/build/cmake/cmake_install.cmake")
  INCLUDE("Z:/Personal/Lab/Physics/lamby/flann/build/src/cmake_install.cmake")
  INCLUDE("Z:/Personal/Lab/Physics/lamby/flann/build/examples/cmake_install.cmake")
  INCLUDE("Z:/Personal/Lab/Physics/lamby/flann/build/test/cmake_install.cmake")
  INCLUDE("Z:/Personal/Lab/Physics/lamby/flann/build/doc/cmake_install.cmake")

ENDIF(NOT CMAKE_INSTALL_LOCAL_ONLY)

IF(CMAKE_INSTALL_COMPONENT)
  SET(CMAKE_INSTALL_MANIFEST "install_manifest_${CMAKE_INSTALL_COMPONENT}.txt")
ELSE(CMAKE_INSTALL_COMPONENT)
  SET(CMAKE_INSTALL_MANIFEST "install_manifest.txt")
ENDIF(CMAKE_INSTALL_COMPONENT)

FILE(WRITE "Z:/Personal/Lab/Physics/lamby/flann/build/${CMAKE_INSTALL_MANIFEST}" "")
FOREACH(file ${CMAKE_INSTALL_MANIFEST_FILES})
  FILE(APPEND "Z:/Personal/Lab/Physics/lamby/flann/build/${CMAKE_INSTALL_MANIFEST}" "${file}\n")
ENDFOREACH(file)
