# Finds LibGdbm library
#
#  LibGdbm_INCLUDE_DIR - where to find pthread.h
#  LibGdbm_LIBRARIES   - List of libraries when using pthread.
#  LibGdbm_FOUND       - True if LibGdbm found.


if (LibGdbm_INCLUDE_DIR)
  # Already in cache, be silent
  set(LibGdbm_FIND_QUIETLY TRUE)
endif (LibGdbm_INCLUDE_DIR)

find_path(LibGdbm_INCLUDE_DIR gdbm.h
  /opt/local/include
  /usr/local/include
  /usr/include
)

set(LibGdbm_NAMES gdbm)
find_library(LibGdbm_LIBRARY
  NAMES ${LibGdbm_NAMES}
  PATHS /usr/lib /usr/local/lib /opt/local/lib
)

if (LibGdbm_INCLUDE_DIR AND LibGdbm_LIBRARY)
   set(LibGdbm_FOUND TRUE)
   set( LibGdbm_LIBRARIES ${LibGdbm_LIBRARY} )
else (LibGdbm_INCLUDE_DIR AND LibGdbm_LIBRARY)
   set(LibGdbm_FOUND FALSE)
   set(LibGdbm_LIBRARIES)
endif (LibGdbm_INCLUDE_DIR AND LibGdbm_LIBRARY)

if (LibGdbm_FOUND)
   if (NOT LibGdbm_FIND_QUIETLY)
      message(STATUS "Found GDB Library: ${LibGdbm_LIBRARY}")
   endif (NOT LibGdbm_FIND_QUIETLY)
else (LibGdbm_FOUND)
   if (LibGdbm_FIND_REQUIRED)
      message(STATUS "Looked for LibGdbm libraries named ${LibGdbm_NAMES}.")
      message(FATAL_ERROR "Could NOT find LibGdbm library")
   endif (LibGdbm_FIND_REQUIRED)
endif (LibGdbm_FOUND)

mark_as_advanced(
  LibGdbm_LIBRARY
  LibGdbm_INCLUDE_DIR
)
