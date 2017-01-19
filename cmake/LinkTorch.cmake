###################
# LinkTorch.cmake #
###################

IF(WITH_TORCH)
  TARGET_LINK_LIBRARIES(${targetname} ${LUAJIT_LIBRARY} ${LUAT_LIBRARY} ${TH_LIBRARY} ${THC_LIBRARY})
ENDIF()
