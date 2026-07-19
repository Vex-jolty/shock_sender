cmake_minimum_required(VERSION 3.30 FATAL_ERROR)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_C_STANDARD 23)

project(shock_sender VERSION 0.0.3)

if (MINGW)
	set(VCPKG_TARGET_TRIPLET "x64-mingw-static")
elseif(MSVC)
	set(VCPKG_TARGET_TRIPLET "x64-windows")
elseif(GNU)
	set(VCPKG_TARGET_TRIPLET "x64-linux")
endif()

add_library(${PROJECT_NAME} SHARED
	src/shock_sender.cpp
	src/shock_logger.cpp
)

if(POLICY CMP0167)
  cmake_policy(SET CMP0167 NEW)
endif()

if (NOT MSVC)
	target_compile_options(${PROJECT_NAME} PRIVATE
		-ffunction-sections
		-fdata-sections
	)

	if (MINGW)
		target_compile_options(${PROJECT_NAME} PRIVATE
			-Wa,-mbig-obj
		)
	endif()

	target_link_options(${PROJECT_NAME} PRIVATE
		-Wl,--gc-sections
	)
else()
	if(NOT DEFINED _WIN32_WINNT)
		set(_WIN32_WINNT "0x0A00")
		target_compile_definitions(${PROJECT_NAME} PRIVATE WIN32 _WIN32_WINNT=${_WIN32_WINNT})
	endif()
	set_property(TARGET ${PROJECT_NAME} PROPERTY
		MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>"
	)
	target_compile_options(${PROJECT_NAME} PRIVATE
		/bigobj
	)
	target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/include/msvc_compat)
endif()

if(VCPKG_BUILD)
	list(APPEND CMAKE_PREFIX_PATH "${CMAKE_BINARY_DIR}/vcpkg_installed/${VCPKG_TARGET_TRIPLET}")
endif()

if(VCPKG_BUILD)
	include(${VCPKG_ROOT_DIR}/scripts/buildsystems/vcpkg.cmake)
endif()

find_package(Boost REQUIRED COMPONENTS json asio beast)
set(LINKED_LIBS Boost::json Boost::asio Boost::beast)

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
	endif()
	list(APPEND LINKED_LIBS ws2_32)
endif()

if (NOT EXISTS ${CMAKE_BINARY_DIR}/shock_config.ini)
	file(COPY ${CMAKE_SOURCE_DIR}/shock_config.example.ini DESTINATION ${CMAKE_BINARY_DIR})
	file(RENAME ${CMAKE_BINARY_DIR}/shock_config.example.ini ${CMAKE_BINARY_DIR}/shock_config.ini)
endif()

file(COPY ${CMAKE_SOURCE_DIR}/include/shock_sender.h DESTINATION ${CMAKE_BINARY_DIR})

target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/include ${Boost_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} PRIVATE ${LINKED_LIBS})

if (LINUX)
	install(TARGETS ${PROJECT_NAME}
		BUNDLE DESTINATION .
		RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
	)
	install(FILES ${CMAKE_BINARY_DIR}/shock_config.ini DESTINATION /etc/shock_sender)
	install(FILES ${CMAKE_BINARY_DIR}/shock_sender.h DESTINATION /usr/include)
endif()