# Add include folders to the library and targets that consume it the SYSTEM
# keyword suppresses warnings for users of the library
if(IRIS_STANDALONE_PROJECT)
  target_include_directories(Iris INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
  )
else()
  target_include_directories(Iris SYSTEM INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
  )
endif()
