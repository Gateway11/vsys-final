
# - Find the blis include file and library
#
#  BLIS_FOUND - system has blis
#  BLIS_INCLUDE_DIRS - the zlib include directory
#  BLIS_LIBRARIES - The libraries needed to use zlib

if(APPLE AND NOT IOS)
	set(BLIS_HINTS "/usr")
endif()
if(BLIS_HINTS)
	set(BLIS_LIBRARIES_HINTS "${BLIS_HINTS}/lib")
endif()

find_path(BLIS_INCLUDE_DIRS
	NAMES blis/blis.h
	HINTS "${BLIS_HINTS}"
	PATH_SUFFIXES include
)

if(BLIS_INCLUDE_DIRS)
	set(HAVE_BLIS_H 1)
endif()

if(ENABLE_STATIC)
	find_library(BLIS_LIBRARIES
		NAMES blisstatic blisstaticd blis blisd
		HINTS "${BLIS_LIBRARIES_HINTS}"
	)
else()
	find_library(BLIS_LIBRARIES
		NAMES blis blisd
		HINTS "${BLIS_LIBRARIES_HINTS}"
	)
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Blis
	DEFAULT_MSG
	BLIS_INCLUDE_DIRS BLIS_LIBRARIES HAVE_BLIS_H
)

mark_as_advanced(BLIS_INCLUDE_DIRS BLIS_LIBRARIES HAVE_BLIS_H)
