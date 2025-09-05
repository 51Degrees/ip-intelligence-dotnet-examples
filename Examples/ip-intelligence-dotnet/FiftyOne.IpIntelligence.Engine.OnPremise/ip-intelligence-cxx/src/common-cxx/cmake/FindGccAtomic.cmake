set(ATOMIC_SEARCH_PATH
  $ENV{HOME}/local/lib64
  $ENV{HOME}/local/lib
  /usr/local/lib64
  /usr/local/lib
  /opt/local/lib64
  /opt/local/lib
  /usr/lib64
  /usr/lib
  /lib64
  /lib
  ${MINGW64_BIN_PATH}
)
if (32bit)
set(ATOMIC_SEARCH_PATH
  ${ATOMIC_SEARCH_PATH}
  $ENV{HOME}/local/lib32
  /usr/local/lib32
  /opt/local/lib32
  /usr/lib32
  /lib32
  ${MINGW32_LIB_PATH}
)
endif()

FIND_LIBRARY(GCCLIBATOMIC_LIBRARY NAMES atomic libatomic.so atomic.so.1 libatomic.so.1
  HINTS
  ${CMAKE_LIBRARY_PATH}
  ${ATOMIC_SEARCH_PATH}
)

IF (GCCLIBATOMIC_LIBRARY)
    SET(GCCLIBATOMIC_FOUND TRUE)
    MESSAGE(STATUS "Found GCC's libatomic.so: lib=${GCCLIBATOMIC_LIBRARY}")
ELSE ()
    IF (MINGW)
      # This code is checking for location of Mingw-w64 in particular
      get_filename_component(MINGW64_BIN_PATH ${CMAKE_C_COMPILER} DIRECTORY)
      set(MINGW64_PATH ${MINGW64_BIN_PATH}/..)
      set(MINGW32_LIB_PATH ${MINGW64_PATH}/x86_64-w64-mingw32/lib)
      set(ATOMIC_MINGW_SEARCH_PATH ${MINGW64_BIN_PATH})
      IF (32bit)
        set (ATOMIC_MINGW_SEARCH_PATH
          ${ATOMIC_MINGW_SEARCH_PATH}
          ${MINGW32_LIB_PATH}
        )
      ENDIF ()
      MESSAGE(STATUS "Mingw-w64 Path: ${MINGW64_PATH}")

      # Use find file here as FIND_LIBRARY does not work well with Windows
      # library naming format.
      FIND_FILE(GCCLIBATOMIC_LIBRARY libatomic-1.dll
        HINTS
        ${CMAKE_LIBRARY_PATH}
        ${ATOMIC_MINGW_SEARCH_PATH}
      )
      
      IF (GCCLIBATOMIC_LIBRARY)
        SET(GCCLIBATOMIC_FOUND TRUE)
        MESSAGE(STATUS "Found GCC's libatomic-1.dll: lib=${GCCLIBATOMIC_LIBRARY}")
      ELSE ()
        SET(GCCLIBATOMIC_FOUND FALSE)
        MESSAGE(STATUS "GCC's libatomic was not found with Mingw-w64. Please verify your Mingw-w64 installation.")
      ENDIF ()
    ELSE ()
      SET(GCCLIBATOMIC_FOUND FALSE)
      MESSAGE(STATUS "GCC's libatomic was not found.")
      MESSAGE(STATUS "Either upgrade to GCC 5+, or install libatomic with 'sudo apt-get install libatomic1'")
    ENDIF ()
ENDIF ()
