# Finds LibPthread library
#
#  LibPthread_INCLUDE_DIR - where to find pthread.h
#  LibPthread_LIBRARIES   - List of libraries when using pthread.
#  LibPthread_FOUND       - True if LibPthread found.


if (LibPthread_INCLUDE_DIR)
  # Already in cache, be silent
  set(LibPthread_FIND_QUIETLY TRUE)
endif (LibPthread_INCLUDE_DIR)

find_path(LibPthread_INCLUDE_DIR pthread.h
  /opt/local/include
  /usr/local/include
  /usr/include
)

set(LibPthread_NAMES pthread)
find_library(LibPthread_LIBRARY
  NAMES ${LibPthread_NAMES}
  PATHS /usr/lib /usr/local/lib /opt/local/lib
)

if (LibPthread_INCLUDE_DIR AND LibPthread_LIBRARY)
   set(LibPthread_FOUND TRUE)
   set( LibPthread_LIBRARIES ${LibPthread_LIBRARY} )
else (LibPthread_INCLUDE_DIR AND LibPthread_LIBRARY)
   set(LibPthread_FOUND FALSE)
   set(LibPthread_LIBRARIES)
endif (LibPthread_INCLUDE_DIR AND LibPthread_LIBRARY)

if (LibPthread_FOUND)
   if (NOT LibPthread_FIND_QUIETLY)
      message(STATUS "Found pthread Library: ${LibPthread_LIBRARY}")
   endif (NOT LibPthread_FIND_QUIETLY)
else (LibPthread_FOUND)
   if (LibPthread_FIND_REQUIRED)
      message(STATUS "Looked for LibPthread libraries named ${LibPthread_NAMES}.")
      message(FATAL_ERROR "Could NOT find LibPthread library")
   endif (LibPthread_FIND_REQUIRED)
endif (LibPthread_FOUND)

mark_as_advanced(
  LibPthread_LIBRARY
  LibPthread_INCLUDE_DIR
)
