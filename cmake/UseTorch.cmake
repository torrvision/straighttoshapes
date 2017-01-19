##################
# UseTorch.cmake #
##################

OPTION(WITH_TORCH "Build with Torch support?" OFF)

IF(WITH_TORCH)
  IF(MSVC_IDE)
    FIND_PATH(LUA_INCLUDE_DIR lua.h HINTS C:/Torch/include/lua)
    FIND_PATH(LUABRIDGE_INCLUDE_DIR LuaBridge.h HINTS $ENV{HOMEPATH}/Downloads/LuaBridge/Source/LuaBridge)
    FIND_LIBRARY(LUAJIT_LIBRARY libluajit.lib HINTS C:/Torch)
	FIND_LIBRARY(LUAT_LIBRARY luaT.lib HINTS C:/Torch)
	FIND_LIBRARY(TH_LIBRARY TH.lib HINTS C:/Torch)
	FIND_LIBRARY(THC_LIBRARY THC.lib HINTS C:/Torch)
  ENDIF()

  IF("${CMAKE_SYSTEM}" MATCHES "Linux")
    FIND_PATH(LUA_INCLUDE_DIR lua.h HINTS ~/software/torch/install/include)
    FIND_LIBRARY(LUAJIT_LIBRARY libluajit.so HINTS ~/software/torch/install/lib)
    FIND_LIBRARY(LUAT_LIBRARY libluaT.so HINTS ~/software/torch/install/lib)
    FIND_LIBRARY(TH_LIBRARY libTH.so HINTS ~/software/torch/install/lib)
    FIND_LIBRARY(THC_LIBRARY libTHC.so HINTS ~/software/torch/install/lib)

    FIND_PATH(LUABRIDGE_INCLUDE_DIR LuaBridge.h HINTS "${PROJECT_SOURCE_DIR}/libraries/LuaBridge-master/Source/LuaBridge")
  ENDIF()

  INCLUDE_DIRECTORIES(${LUA_INCLUDE_DIR})
  INCLUDE_DIRECTORIES(${LUABRIDGE_INCLUDE_DIR})
  ADD_DEFINITIONS(-DWITH_TORCH)
ENDIF()
