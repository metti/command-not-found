###### PROJECT DEFINITIONS ######

PROJECT (CNF)

STRING(TOLOWER ${PROJECT_NAME} BINARY_NAME)
STRING(TOLOWER command-not-found PACKAGE_NAME)


###### PREREQUISITES ######

CMAKE_MINIMUM_REQUIRED (VERSION 3.6.0)

LIST(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/")

SET(CMAKE_EXPORT_COMPILE_COMMANDS ON)

INCLUDE(GetGitRevisionDescription)

OPTION(STATIC_LINKS "Static Executables" OFF)

IF(STATIC_LINKS)
    MESSAGE(STATUS "Statically linking boost libraries")
    SET(Boost_USE_STATIC_LIBS ON)
ENDIF(STATIC_LINKS)

FIND_PACKAGE(Boost 1.44
                   COMPONENTS
                        filesystem
                        system
                        locale
                   REQUIRED)

INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIRS})

FIND_PACKAGE(LibTdb 1.2 REQUIRED)
LIST(APPEND EXTRA_LIBRARIES ${LibTdb_LIBRARY})

FIND_PACKAGE(LibArchive 3 REQUIRED)
INCLUDE_DIRECTORIES(${LibArchive_INCLUDE_DIRS})
LIST(APPEND EXTRA_LIBRARIES ${LibArchive_LIBRARIES})

###### PROJECT CONFIGURATION ######

### Names ###
SET (PROGRAM_NAME ${BINARY_NAME})
SET (PROGRAM_LONG_NAME "Command not found")
SET (PROGRAM_AUTHOR "Matthias Maennich")
SET (PROGRAM_MAINTAINER "Matthias Maennich")
SET (PROGRAM_MAINTAINER_MAIL "arch@maennich.net")

### Version Information ###
SET (VERSION_MAJOR 0)
SET (VERSION_MINOR 6)
SET (VERSION_PATCH 2)
#SET (VERSION_TAG "-beta")
SET (VERSION_SHORT "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}${VERSION_TAG}")

GET_GIT_HEAD_REVISION(GIT_REFSPEC GIT_HASH GIT_HASH_SHORT)

IF ("${CMAKE_BUILD_TYPE}" MATCHES "^Debug$")
    SET (DEBUG 1)
    MESSAGE (STATUS "Compiling with debug symbols")
ENDIF()

### Application Configuration ###

SET (DATABASE_PATH ${CMAKE_INSTALL_PREFIX}/var/lib/${BINARY_NAME})
SET (LC_MESSAGE_PATH ${CMAKE_INSTALL_PREFIX}/usr/share)

### Prepare Config ###
CONFIGURE_FILE (
    "${PROJECT_SOURCE_DIR}/config.cpp.in"
    "${PROJECT_BINARY_DIR}/config.cpp"
)

CONFIGURE_FILE (
    "${PROJECT_SOURCE_DIR}/config.h.in"
    "${PROJECT_BINARY_DIR}/config.h"
)

CONFIGURE_FILE (
    "${PROJECT_SOURCE_DIR}/cnf-sync.in"
    "${PROJECT_BINARY_DIR}/cnf-sync"
)

CONFIGURE_FILE (
    "${PROJECT_SOURCE_DIR}/cnf.service.in"
    "${PROJECT_BINARY_DIR}/cnf.service"
)

CONFIGURE_FILE (
    "${PROJECT_SOURCE_DIR}/cnf.timer.in"
    "${PROJECT_BINARY_DIR}/cnf.timer"
)

### Add binary dir to include path ###

INCLUDE_DIRECTORIES ("${PROJECT_BINARY_DIR}" "${PROJECT_SOURCE_DIR}")


###### EXECUTABLES and LIBRARIES######

SET (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -pedantic -Werror -std=c++14")

OPTION(ENABLE_CLANG_TIDY OFF)
IF(ENABLE_CLANG_TIDY)
    FIND_PROGRAM(CLANG_TIDY_EXE clang-tidy)
    IF(CLANG_TIDY_EXE)
        MESSAGE(STATUS "Adding clang-tidy diagnostics")
        SET(CMAKE_CXX_CLANG_TIDY "${CLANG_TIDY_EXE}")
    ENDIF()
ELSE()
    UNSET(CMAKE_CXX_CLANG_TIDY)
ENDIF()

### CNF Client ###

SET (CNF_SRCS    db.cpp
                 db_tdb.cpp
                 package.cpp
                 similar.cpp
                 ${PROJECT_BINARY_DIR}/config.cpp
)

ADD_LIBRARY(${BINARY_NAME} SHARED ${CNF_SRCS})

TARGET_LINK_LIBRARIES(${BINARY_NAME} ${Boost_LIBRARIES} ${EXTRA_LIBRARIES})

SET_TARGET_PROPERTIES(${BINARY_NAME}
                        PROPERTIES
                            SOVERSION ${VERSION_MAJOR}
                            VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}
                     )

ADD_EXECUTABLE (${BINARY_NAME}-lookup lookup.cpp)

TARGET_LINK_LIBRARIES (${BINARY_NAME}-lookup ${BINARY_NAME})


ADD_EXECUTABLE (${BINARY_NAME}-populate populate.cpp)

TARGET_LINK_LIBRARIES (${BINARY_NAME}-populate ${BINARY_NAME})

IF (NOT "${CMAKE_BUILD_TYPE}" MATCHES "^Debug$")

    ADD_CUSTOM_COMMAND(TARGET ${BINARY_NAME}-populate
                       POST_BUILD
                       COMMAND ${CMAKE_OBJCOPY} --only-keep-debug ${BINARY_NAME}-populate ${BINARY_NAME}-populate.debug
                       COMMAND ${CMAKE_OBJCOPY} --add-gnu-debuglink=${BINARY_NAME}-populate.debug ${BINARY_NAME}-populate
                       COMMAND ${CMAKE_OBJCOPY} --strip-debug --strip-unneeded ${BINARY_NAME}-populate
                       COMMENT "Splitting symbols from ${BINARY_NAME}-populate"
                       )
    ADD_CUSTOM_COMMAND(TARGET ${BINARY_NAME}
                       POST_BUILD
                       COMMAND ${CMAKE_OBJCOPY} --only-keep-debug lib${BINARY_NAME}.so lib${BINARY_NAME}.so.debug
                       COMMAND ${CMAKE_OBJCOPY} --add-gnu-debuglink=lib${BINARY_NAME}.so.debug lib${BINARY_NAME}.so
                       COMMAND ${CMAKE_OBJCOPY} --strip-debug --strip-unneeded lib${BINARY_NAME}.so
                       COMMENT "Splitting symbols from lib${BINARY_NAME}.so"
                       )
    ADD_CUSTOM_COMMAND(TARGET ${BINARY_NAME}-lookup
                       POST_BUILD
                       COMMAND ${CMAKE_OBJCOPY} --only-keep-debug ${BINARY_NAME}-lookup ${BINARY_NAME}-lookup.debug
                       COMMAND ${CMAKE_OBJCOPY} --add-gnu-debuglink=${BINARY_NAME}-lookup.debug ${BINARY_NAME}-lookup
                       COMMAND ${CMAKE_OBJCOPY} --strip-debug --strip-unneeded ${BINARY_NAME}-lookup
                       COMMENT "Splitting symbols from ${BINARY_NAME}-lookup"
                       )
ENDIF()

###### TESTS #####
OPTION(WITH_TESTS "Build the test suite" ON)
IF(WITH_TESTS)
    ENABLE_TESTING()
    FIND_PACKAGE(Catch2 REQUIRED)
    INCLUDE(Catch)

    ADD_CUSTOM_TARGET(check)
    ADD_CUSTOM_COMMAND(TARGET check COMMAND ${CMAKE_CTEST_COMMAND}
                                            --output-on-failure
                                             -j${CPU_CORES})
    ADD_CUSTOM_TARGET(tests)

    ADD_LIBRARY(test_main OBJECT test_main.cpp)
    TARGET_LINK_LIBRARIES(test_main PUBLIC Catch2::Catch2)

    FOREACH(test_name similar)
        STRING(REPLACE "/" "-" test_bin_name ${test_name})
        SET(test_bin_name test-${test_bin_name})
        ADD_EXECUTABLE(${test_bin_name} ${test_name}.t.cpp)
        TARGET_LINK_LIBRARIES(${test_bin_name} PRIVATE ${BINARY_NAME} test_main)
        ADD_DEPENDENCIES(check ${test_bin_name})
        ADD_DEPENDENCIES(tests ${test_bin_name})
        catch_discover_tests(${test_bin_name})
    ENDFOREACH()
ENDIF()

###### INSTALLATION ######

### Binaries
INSTALL (TARGETS ${BINARY_NAME}-lookup DESTINATION usr/bin)
INSTALL (TARGETS ${BINARY_NAME}-populate DESTINATION usr/bin)
INSTALL (TARGETS ${BINARY_NAME} DESTINATION usr/lib)

INSTALL (DIRECTORY DESTINATION var/lib/${BINARY_NAME})

INSTALL (FILES cnf.sh
            PERMISSIONS OWNER_WRITE
                        OWNER_EXECUTE
                        GROUP_EXECUTE
                        WORLD_EXECUTE
                        OWNER_READ
                        GROUP_READ
                        WORLD_READ
            DESTINATION etc/profile.d)

INSTALL (FILES cnf.fish
            PERMISSIONS OWNER_WRITE
                        OWNER_READ
                        GROUP_READ
                        WORLD_READ
						RENAME __fish_command_not_found_handler.fish
            DESTINATION usr/share/fish/functions)

INSTALL (FILES ${PROJECT_BINARY_DIR}/${BINARY_NAME}-sync
            PERMISSIONS OWNER_WRITE
                        OWNER_EXECUTE
                        GROUP_EXECUTE
                        WORLD_EXECUTE
                        OWNER_READ
                        GROUP_READ
                        WORLD_READ

            DESTINATION usr/bin)

INSTALL (FILES ${PROJECT_BINARY_DIR}/${BINARY_NAME}.service
            PERMISSIONS OWNER_WRITE
                        OWNER_READ
                        GROUP_READ
                        WORLD_READ

            DESTINATION usr/lib/systemd/system)

INSTALL (FILES ${PROJECT_BINARY_DIR}/${BINARY_NAME}.timer
            PERMISSIONS OWNER_WRITE
                        OWNER_READ
                        GROUP_READ
                        WORLD_READ

            DESTINATION usr/lib/systemd/system)


###### I18N FILES ######

SET(locales
    zh_CN
    )

FIND_PROGRAM(MSGFMT msgfmt)

IF(MSGFMT)
    FILE(MAKE_DIRECTORY "${PROJECT_BINARY_DIR}/i18n")

    FOREACH(_locale ${locales})
        SET(_po "${PROJECT_SOURCE_DIR}/i18n/${_locale}.po")
        SET(_mo "${PROJECT_BINARY_DIR}/i18n/${_locale}.mo")
        ADD_CUSTOM_COMMAND(
            OUTPUT ${_mo}
            COMMAND ${MSGFMT} -o ${_mo} ${_po}
            DEPENDS ${_po}
            )
        INSTALL(
            FILES ${_mo}
            DESTINATION ${LC_MESSAGE_PATH}/${_locale}/LC_MESSAGES/
            RENAME ${PROGRAM_NAME}.mo
            )
        SET(_mo_files ${_mo_files} ${_mo})
    ENDFOREACH()

    ADD_CUSTOM_TARGET(i18n ALL DEPENDS ${_mo_files})
ENDIF()


###### PACKAGING ######

ADD_CUSTOM_TARGET(dist)

### Basic Configuration ###
SET(CPACK_PACKAGE_NAME ${PROJECT_NAME})
SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY ${PROGRAM_LONG_NAME})
SET(CPACK_PACKAGE_VENDOR ${PROGRAM_AUTHOR})
SET(CPACK_PACKAGE_EXECUTABLES ${BINARY_NAME} ${PROJECT_NAME})

SET(CPACK_PACKAGE_VERSION_MAJOR ${VERSION_MAJOR})
SET(CPACK_PACKAGE_VERSION_MINOR ${VERSION_MINOR})
SET(CPACK_PACKAGE_VERSION_PATCH ${VERSION_PATCH})

### Source packages ###
STRING(TOLOWER
         "${PACKAGE_NAME}-${VERSION_SHORT}-src"
         CPACK_SOURCE_PACKAGE_FILE_NAME)

SET(CPACK_SOURCE_GENERATOR "TGZ")
ADD_CUSTOM_TARGET(source COMMAND ${CMAKE_MAKE_PROGRAM} package_source)


### Binary Packages ###
STRING(TOLOWER
         "${PACKAGE_NAME}-${VERSION_SHORT}-${CMAKE_SYSTEM_PROCESSOR}"
         CPACK_PACKAGE_FILE_NAME)

SET(CPACK_GENERATOR "STGZ")

FIND_PROGRAM(MAKEPKG_EXECUTABLE makepkg)

IF (MAKEPKG_EXECUTABLE)

    CONFIGURE_FILE (
        "${PROJECT_SOURCE_DIR}/archlinux/install.in"
        "${PROJECT_BINARY_DIR}/archlinux/install"
        COPYONLY
    )

    CONFIGURE_FILE (
        "${PROJECT_SOURCE_DIR}/archlinux/PKGBUILD.in"
        "${PROJECT_BINARY_DIR}/archlinux/PKGBUILD"
    )

    CONFIGURE_FILE (
        "${PROJECT_SOURCE_DIR}/archlinux/install.in"
        "${PROJECT_BINARY_DIR}/archlinux-aur/install"
        COPYONLY
    )

    EXECUTE_PROCESS(COMMAND ${CMAKE_COMMAND} -E md5sum "${PROJECT_BINARY_DIR}/archlinux-aur/install"
                    OUTPUT_VARIABLE ARCH_INSTALL_MD5
                   )

    STRING(REPLACE "${PROJECT_BINARY_DIR}/archlinux-aur/install" "" ARCH_INSTALL_MD5 ${ARCH_INSTALL_MD5})
    STRING(REPLACE "\n" "" ARCH_INSTALL_MD5 ${ARCH_INSTALL_MD5})
    STRING(REPLACE " " "" ARCH_INSTALL_MD5 ${ARCH_INSTALL_MD5})

    CONFIGURE_FILE (
        "${PROJECT_SOURCE_DIR}/archlinux/PKGBUILD-AUR.in"
        "${PROJECT_BINARY_DIR}/archlinux-aur/PKGBUILD"
    )

    ADD_CUSTOM_TARGET(archpackage
                          cp ../${CPACK_SOURCE_PACKAGE_FILE_NAME}.tar.gz .
                        COMMAND
                           makepkg --skipinteg --force
                        WORKING_DIRECTORY
                           archlinux
                      )
    ADD_DEPENDENCIES(archpackage source)

    ADD_CUSTOM_TARGET(archsrcpackage
                        COMMAND
                           makepkg --source -f
                        WORKING_DIRECTORY
                           archlinux-aur
                      )

ENDIF(MAKEPKG_EXECUTABLE)

INCLUDE(CPack)

