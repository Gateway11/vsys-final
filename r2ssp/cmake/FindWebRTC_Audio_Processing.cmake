
# - Find the webrtc_audio_processing include file and library
#
#  WEBRTC_AUDIO_PROCESSING_FOUND - system has blis
#  WEBRTC_AUDIO_PROCESSING_INCLUDE_DIRS - the webrtc_audio_processing include directory
#  WEBRTC_AUDIO_PROCESSING_LIBRARIES - The libraries needed to use libwebrtc_audio_processing

if(APPLE AND NOT IOS)
	set(WEBRTC_AUDIO_PROCESSING_HINTS "/usr")
endif()
if(WEBRTC_AUDIO_PROCESSING_HINTS)
	set(WEBRTC_AUDIO_PROCESSING_LIBRARIES_HINTS "${WEBRTC_AUDIO_PROCESSING_HINTS}/lib")
endif()

find_path(WEBRTC_AUDIO_PROCESSING_INCLUDE_DIRS
	NAMES webrtc_audio_processing/webrtc/modules/audio_processing/include/audio_processing.h
	HINTS "${WEBRTC_AUDIO_PROCESSING_HINTS}"
	PATH_SUFFIXES include
)

if(WEBRTC_AUDIO_PROCESSING_INCLUDE_DIRS)
	set(HAVE_WEBRTC_AUDIO_PROCESSING_H 1)
endif()

if(ENABLE_STATIC)
	find_library(WEBRTC_AUDIO_PROCESSING_LIBRARIES
		NAMES webrtc_audio_processingstatic webrtc_audio_processingstaticd webrtc_audio_processing webrtc_audio_processingd
		HINTS "${WEBRTC_AUDIO_PROCESSING_LIBRARIES_HINTS}"
	)
else()
	find_library(WEBRTC_AUDIO_PROCESSING_LIBRARIES
		NAMES webrtc_audio_processing webrtc_audio_processingd
		HINTS "${WEBRTC_AUDIO_PROCESSING_LIBRARIES_HINTS}"
	)
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(WebRTC_Audio_Processing
	DEFAULT_MSG
	WEBRTC_AUDIO_PROCESSING_INCLUDE_DIRS WEBRTC_AUDIO_PROCESSING_LIBRARIES HAVE_WEBRTC_AUDIO_PROCESSING_H
)

mark_as_advanced(WEBRTC_AUDIO_PROCESSING_INCLUDE_DIRS WEBRTC_AUDIO_PROCESSING_LIBRARIES HAVE_WEBRTC_AUDIO_PROCESSING_H)
