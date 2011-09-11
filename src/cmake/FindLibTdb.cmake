# Finds LibTdb library
#
#  LibTdb_INCLUDE_DIR - where to find pthread.h
#  LibTdb_LIBRARIES   - List of libraries when using pthread.
#  LibTdb_FOUND       - True if LibTdb found.


if (LibTdb_INCLUDE_DIR)
  # Already in cache, be silent
  set(LibTdb_FIND_QUIETLY TRUE)
endif (LibTdb_INCLUDE_DIR)

find_path(LibTdb_INCLUDE_DIR tdb.h
  /opt/local/include
  /usr/local/include
  /usr/include
)

set(LibTdb_NAMES tdb)
find_library(LibTdb_LIBRARY
  NAMES ${LibTdb_NAMES}
  PATHS /usr/lib /usr/local/lib /opt/local/lib
)

if (LibTdb_INCLUDE_DIR AND LibTdb_LIBRARY)
   set(LibTdb_FOUND TRUE)
   set( LibTdb_LIBRARIES ${LibTdb_LIBRARY} )
else (LibTdb_INCLUDE_DIR AND LibTdb_LIBRARY)
   set(LibTdb_FOUND FALSE)
   set(LibTdb_LIBRARIES)
endif (LibTdb_INCLUDE_DIR AND LibTdb_LIBRARY)

if (LibTdb_FOUND)
   if (NOT LibTdb_FIND_QUIETLY)
      message(STATUS "Found TDB Library: ${LibTdb_LIBRARY}")
   endif (NOT LibTdb_FIND_QUIETLY)
else (LibTdb_FOUND)
   if (LibTdb_FIND_REQUIRED)
      message(STATUS "Looked for LibTdb libraries named ${LibTdb_NAMES}.")
      message(FATAL_ERROR "Could NOT find LibTdb library")
   endif (LibTdb_FIND_REQUIRED)
endif (LibTdb_FOUND)

mark_as_advanced(
  LibTdb_LIBRARY
  LibTdb_INCLUDE_DIR
)
