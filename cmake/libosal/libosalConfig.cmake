# libosalConfig.cmake - pkg-config wrapper for libosal

find_package(PkgConfig REQUIRED)
pkg_check_modules(PC_LIBOSAL REQUIRED libosal)

find_library(LIBOSAL_LIB NAMES osal)
if(NOT LIBOSAL_LIB)
  message(FATAL_ERROR "Could not find libosal shared library")
endif()

# Legacy variable for older CMake usage
set(libosal_LIBS ${PC_LIBOSAL_LIBRARIES} CACHE INTERNAL "Link flags for libosal")

add_library(libosal::libosal SHARED IMPORTED)
set_target_properties(libosal::libosal PROPERTIES
    IMPORTED_LOCATION "${LIBOSAL_LIB}"
    INTERFACE_INCLUDE_DIRECTORIES "${PC_LIBOSAL_INCLUDEDIR}"
)

separate_arguments(PC_LIBOSAL_LIBS UNIX_COMMAND "${PC_LIBOSAL_LIBRARIES}")

set(LINK_DIRS "")
set(LINK_LIBS "")

foreach(lib ${PC_LIBOSAL_LIBS})
  if(lib MATCHES "^-l(.+)$")
    list(APPEND LINK_LIBS "${CMAKE_MATCH_1}")
  elseif(lib MATCHES "^-L(.+)$")
    list(APPEND LINK_DIRS "${CMAKE_MATCH_1}")
  else()
    list(APPEND LINK_LIBS "${lib}")
  endif()
endforeach()

if(LINK_DIRS)
  target_link_directories(libosal::libosal INTERFACE ${LINK_DIRS})
endif()

target_link_libraries(libosal::libosal INTERFACE ${LINK_LIBS})
