#
# Try to find EMBREE
# Once done this will define
#
# EMBREE_FOUND           - system has EMBREE
# EMBREE_INCLUDE_DIRS    - the EMBREE include directories
# EMBREE_LIBRARIES       - Link these to use EMBREE
#

FIND_PATH(EMBREE_INCLUDE_DIR embree2/rtcore.h
	  PATHS
		${PROJECT_SOURCE_DIR}/../../external/embree/include
		${PROJECT_SOURCE_DIR}/../external/embree/include
		${PROJECT_SOURCE_DIR}/../libigl/external/embree/include
		NO_DEFAULT_PATH
    )

#message(FATAL_ERROR ${PROJECT_SOURCE_DIR}/../libigl/external/embree)
#message(FATAL_ERROR ${EMBREE_INCLUDE_DIR})

SET(SEARCH_PATHS "${EMBREE_INCLUDE_DIR}/../" "${EMBREE_INCLUDE_DIR}/../build" "${EMBREE_INCLUDE_DIR}/../lib")

FIND_LIBRARY(EMBREE_CORE_LIBRARY NAMES simd PATHS ${SEARCH_PATHS} PATH_SUFFIXES a lib)
FIND_LIBRARY(EMBREE_CORE_LIBRARY3  NAMES embree_sse41 PATHS ${SEARCH_PATHS} PATH_SUFFIXES a lib)
FIND_LIBRARY(EMBREE_CORE_LIBRARY4  NAMES embree_sse42 PATHS ${SEARCH_PATHS} PATH_SUFFIXES a lib)
FIND_LIBRARY(EMBREE_CORE_LIBRARY5 NAMES transport PATHS ${SEARCH_PATHS} PATH_SUFFIXES a lib)
FIND_LIBRARY(EMBREE_CORE_LIBRARY6 NAMES image PATHS ${SEARCH_PATHS} PATH_SUFFIXES a lib)
FIND_LIBRARY(EMBREE_CORE_LIBRARY7 NAMES lexers PATHS ${SEARCH_PATHS} PATH_SUFFIXES a lib)
FIND_LIBRARY(EMBREE_CORE_LIBRARY8 NAMES embree PATHS ${SEARCH_PATHS} PATH_SUFFIXES dylib a lib)
FIND_LIBRARY(EMBREE_CORE_LIBRARY9 NAMES sys PATHS ${SEARCH_PATHS} PATH_SUFFIXES a lib)

if(EMBREE_CORE_LIBRARY AND EMBREE_INCLUDE_DIR)
set(EMBREE_FOUND TRUE)
endif(EMBREE_CORE_LIBRARY AND EMBREE_INCLUDE_DIR)

IF (EMBREE_FOUND)
   message(STATUS "Found EMBREE: ${EMBREE_INCLUDE_DIR}")

   SET(EMBREE_LIBRARIES
   "${EMBREE_CORE_LIBRARY}"
   "${EMBREE_CORE_LIBRARY3}"
   "${EMBREE_CORE_LIBRARY4}"
   "${EMBREE_CORE_LIBRARY5}"
   "${EMBREE_CORE_LIBRARY6}"
   "${EMBREE_CORE_LIBRARY7}"
   "${EMBREE_CORE_LIBRARY8}"
   "${EMBREE_CORE_LIBRARY9}"
   )
   SET(EMBREE_INCLUDE_DIRS ${EMBREE_INCLUDE_DIR} ${EMBREE_INCLUDE_DIR}/embree)
ELSE (EMBREE_FOUND)
    message(STATUS "could NOT find EMBREE")
ENDIF (EMBREE_FOUND)
