# SPDX-FileCopyrightText: 2021-2023 Graeme Gott <graeme@gottcode.org>
#
# SPDX-License-Identifier: GPL-3.0-or-later

cmake_minimum_required(VERSION 3.16)

# Configure project
project(kapow VERSION 1.6.0 LANGUAGES CXX)

set(project_copyright "2008-2024 Graeme Gott")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui LinguistTools Network PrintSupport Widgets)
include(GNUInstallDirs)

add_compile_definitions(
	QT_NO_KEYWORDS
	$<$<CONFIG:DEBUG>:QT_STRICT_ITERATORS>
	$<$<CONFIG:DEBUG>:QT_NO_NARROWING_CONVERSIONS_IN_CONNECT>
	$<$<CONFIG:DEBUG>:QT_DISABLE_DEPRECATED_BEFORE=0x060700>
)

# Version number
include(cmake/AddVersionCompileDefinition.cmake)
add_version_compile_definition(src/main.cpp VERSIONSTR)

# Create tests
option(ENABLE_TESTS "Enable unit tests" OFF)
if (ENABLE_TESTS)
	find_package(Qt6 REQUIRED COMPONENTS Test)
	enable_testing()
	add_subdirectory(tests)
endif()

# Create program
qt_add_executable(kapow
	src/contact.cpp
	src/date_editor.cpp
	src/filter_model.cpp
	src/locale_dialog.cpp
	src/main.cpp
	src/paths.cpp
	src/project.cpp
	src/project_delegate.cpp
	src/rates.cpp
	src/report.cpp
	src/session.cpp
	src/session_delegate.cpp
	src/session_dialog.cpp
	src/session_model.cpp
	src/settings.cpp
	src/time_editor.cpp
	src/window.cpp
	src/3rdparty/qtsingleapplication/qtsingleapplication.cpp
	src/3rdparty/qtsingleapplication/qtlocalpeer.cpp
	${translations_QM}
)

target_include_directories(kapow PRIVATE
	src/3rdparty/qtsingleapplication
)

target_link_libraries(kapow PRIVATE
	Qt6::Core
	Qt6::Gui
	Qt6::Network
	Qt6::PrintSupport
	Qt6::Widgets
)

# Create translations
file(GLOB translations_SRCS translations/*.ts)
qt_add_translations(kapow
	TS_FILES ${translations_SRCS}
	QM_FILES_OUTPUT_VARIABLE translations_QM
	LUPDATE_OPTIONS -no-obsolete -locations none
)

# Optimize build
option(ENABLE_LINK_TIME_OPTIMIZATION "Enable link time optimization" OFF)
if(ENABLE_LINK_TIME_OPTIMIZATION)
	include(CheckIPOSupported)
	check_ipo_supported(RESULT result)
	if(result)
		set_target_properties(kapow PROPERTIES INTERPROCEDURAL_OPTIMIZATION TRUE)
	endif()
endif()

option(ENABLE_STRIP "Enable automatic stripping of builds" OFF)
if(ENABLE_STRIP)
	add_custom_command(TARGET kapow
		POST_BUILD
		COMMAND ${CMAKE_STRIP} $<TARGET_FILE:kapow>
	)
endif()

# Install
target_sources(kapow PRIVATE icons/icons.qrc)

if(APPLE)
	set(datadir "../Resources")

	set_target_properties(kapow PROPERTIES
		OUTPUT_NAME Kapow
		MACOSX_BUNDLE TRUE
		MACOSX_BUNDLE_INFO_PLIST ${CMAKE_SOURCE_DIR}/mac/Info.plist.in
	)

	include(cmake/BundleResources.cmake)
	bundle_data(kapow ${CMAKE_SOURCE_DIR}/icons/kapow.icns Resources)
	bundle_translations(kapow "${translations_QM}")
elseif(WIN32)
	set(datadir ".")

	# Use Qt6 macro until CMake provides something
	# https://bugreports.qt.io/browse/QTBUG-87618
	set_target_properties(kapow PROPERTIES
		OUTPUT_NAME Kapow
		WIN32_EXECUTABLE TRUE
		QT_TARGET_VERSION "${PROJECT_VERSION}"
		QT_TARGET_COMPANY_NAME "Graeme Gott"
		QT_TARGET_DESCRIPTION "Punch clock"
		QT_TARGET_COPYRIGHT "\\xA9 ${project_copyright}"
		QT_TARGET_PRODUCT_NAME "Kapow"
		QT_TARGET_RC_ICONS ${CMAKE_SOURCE_DIR}/icons/kapow.ico
	)
	_qt_internal_generate_win32_rc_file(kapow)
else()
	file(RELATIVE_PATH datadir ${CMAKE_INSTALL_FULL_BINDIR} ${CMAKE_INSTALL_FULL_DATADIR}/kapow)

	install(TARGETS kapow RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
	install(FILES ${translations_QM} DESTINATION ${CMAKE_INSTALL_DATADIR}/kapow/translations)
	install(FILES doc/kapow.1 DESTINATION ${CMAKE_INSTALL_MANDIR}/man1 COMPONENT doc)
	install(DIRECTORY icons/hicolor DESTINATION ${CMAKE_INSTALL_DATADIR}/icons)
	install(FILES icons/kapow.appdata.xml DESTINATION ${CMAKE_INSTALL_DATADIR}/metainfo)
	install(FILES icons/kapow.desktop DESTINATION ${CMAKE_INSTALL_DATADIR}/applications)
endif()

set_property(SOURCE src/main.cpp APPEND PROPERTY COMPILE_DEFINITIONS KAPOW_DATADIR="${datadir}")
