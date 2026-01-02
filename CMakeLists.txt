cmake_minimum_required(VERSION 3.26.0 FATAL_ERROR)
set(CMAKE_SYSTEM_VERSION 10.0 CACHE STRING "" FORCE)
set(CMAKE_CXX_STANDARD 20 CACHE STRING "The C++ standard to use")
set(CMAKE_C_STANDARD 23 CACHE STRING "The C standard to use")

project(ShockSender VERSION 0.0.1)

add_library(shock_sender SHARED shock_sender.cpp)

if(POLICY CMP0167)
  cmake_policy(SET CMP0167 NEW)
endif()

find_package(Boost REQUIRED COMPONENTS json)

if (NOT MSVC)
	target_compile_options(shock_sender PRIVATE
		-ffunction-sections
		-fdata-sections
	)

	target_link_options(shock_sender PRIVATE
		-Wl,--gc-sections
	)
else()
	target_compile_options(shock_sender PRIVATE
		-/bigobj
	)
endif()

set(LINKED_LIBS Boost::json)

if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
	set_source_files_properties(
		shock_sender.cpp
		PROPERTIES
			COMPILE_OPTIONS "-fno-function-sections;-fno-data-sections"
	)
	# These settings are specifically for cross-compiling from Linux to Windows
	if (MINGW AND CMAKE_CROSSCOMPILING)
		set_source_files_properties(
			shock_sender.cpp
			PROPERTIES
				COMPILE_OPTIONS "-Wa,-mbig-obj -static"
		)

		set_target_properties(shock_sender PROPERTIES
			PREFIX ""
			SUFFIX ".dll"
		)
		list(APPEND LINKED_LIBS ws2_32)
	endif()
endif()

target_include_directories(shock_sender PRIVATE ${Boost_INCLUDE_DIRS})
target_link_libraries(shock_sender PRIVATE ${LINKED_LIBS})