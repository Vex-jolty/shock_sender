cmake_minimum_required(VERSION 3.26.0 FATAL_ERROR)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_C_STANDARD 23)

project(shock_sender VERSION 0.0.1)

add_library(${PROJECT_NAME} SHARED shock_sender.cpp)

if(POLICY CMP0167)
  cmake_policy(SET CMP0167 NEW)
endif()

if (NOT MSVC)
	target_compile_options(${PROJECT_NAME} PRIVATE
		-ffunction-sections
		-fdata-sections
		-Wa,-mbig-obj
	)

	target_link_options(${PROJECT_NAME} PRIVATE
		-Wl,--gc-sections
	)
else()
	target_compile_options(${PROJECT_NAME} PRIVATE
		-/bigobj
	)
endif()

find_package(Boost REQUIRED COMPONENTS json)
set(LINKED_LIBS Boost::json)

if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
	set_source_files_properties(
		shock_sender.cpp
		PROPERTIES
			COMPILE_OPTIONS "-fno-function-sections;-fno-data-sections"
	)
	# These settings are specifically for cross-compiling from Linux to Windows
	if (MINGW AND CMAKE_CROSSCOMPILING)
		target_link_options(${PROJECT_NAME} PRIVATE
			-static
			-static-libgcc
			-static-libstdc++
		)
		set_source_files_properties(
			shock_sender.cpp
			PROPERTIES
				COMPILE_OPTIONS "-Wa,-mbig-obj"
				LINK_OPTIONS "-static"
		)

		set_target_properties(${PROJECT_NAME} PROPERTIES
			PREFIX ""
			SUFFIX ".dll"
		)
		list(APPEND LINKED_LIBS ws2_32)
	endif()
endif()

if (NOT EXISTS ${CMAKE_BINARY_DIR}/shock_config.ini)
	file(COPY ${CMAKE_SOURCE_DIR}/shock_config.example.ini DESTINATION ${CMAKE_BINARY_DIR})
	file(RENAME ${CMAKE_BINARY_DIR}/shock_config.example.ini ${CMAKE_BINARY_DIR}/shock_config.ini)
endif()

target_include_directories(${PROJECT_NAME} PRIVATE ${Boost_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} PRIVATE ${LINKED_LIBS})