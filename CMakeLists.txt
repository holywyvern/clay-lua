CMAKE_MINIMUM_REQUIRED(VERSION 3.10)
PROJECT("love-clay" C CXX)

SET(CMAKE_INSTALL_PREFIX, "${PROJECT_SOURCE_DIR}")


SET(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake")
FIND_PACKAGE(LuaJIT REQUIRED)

SET(LIB_NAME "clay")

ADD_LIBRARY(
	"${LIB_NAME}"
	MODULE
	src/clay.h
	src/string_cache.c
	src/lua_clay.c
)

set_property(TARGET "${LIB_NAME}" PROPERTY C_STANDARD 99)

TARGET_INCLUDE_DIRECTORIES(
	"${LIB_NAME}"
	PUBLIC
	src
	"${LUA_INCLUDE_DIR}"
)

TARGET_LINK_LIBRARIES(
	"${LIB_NAME}"
	${LUA_LIBRARIES}
)

IF(MSVC)
  TARGET_COMPILE_DEFINITIONS(${LIB_NAME} PRIVATE LUA_BUILD_AS_DLL)
	TARGET_LINK_LIBRARIES("${LIB_NAME}" ole32.lib uuid.lib shell32.lib)
endif(MSVC)

SET_TARGET_PROPERTIES("${LIB_NAME}" PROPERTIES PREFIX "")

IF(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
  SET(CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}" CACHE PATH "..." FORCE)
ENDIF(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)

INSTALL(TARGETS "${LIB_NAME}" DESTINATION .)