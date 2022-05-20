cmake_minimum_required(VERSION 3.16.0)

# Configure project
project(kapow VERSION 1.6.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui LinguistTools Network PrintSupport Widgets)
if(UNIX AND NOT APPLE)
	include(GNUInstallDirs)
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
	add_compile_definitions(
		QT_STRICT_ITERATORS
		QT_NO_NARROWING_CONVERSIONS_IN_CONNECT
		QT_DISABLE_DEPRECATED_BEFORE=0x060300
	)
endif()

# Create tests
option(ENABLE_TESTS "Enable unit tests" OFF)
if (ENABLE_TESTS)
	find_package(Qt6 REQUIRED COMPONENTS Test)
	enable_testing()
	add_subdirectory(tests)
endif()

# Create translations
file(GLOB translations_SRCS translations/*.ts)
qt6_add_translation(translations_QM ${translations_SRCS})

add_custom_target(update_translations
	COMMAND Qt6::lupdate -no-obsolete -locations none src -ts ${translations_SRCS}
	WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
	VERBATIM
)

# Create program
qt6_add_executable(kapow
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
	icons/icons.qrc
	${translations_QM}
)

target_link_libraries(kapow PRIVATE
	Qt6::Core
	Qt6::Gui
	Qt6::Network
	Qt6::PrintSupport
	Qt6::Widgets
)

target_compile_definitions(kapow PRIVATE VERSIONSTR="${PROJECT_VERSION}")

# Install
if(UNIX AND NOT APPLE)
	install(TARGETS kapow RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
	install(FILES ${translations_QM} DESTINATION ${CMAKE_INSTALL_DATADIR}/kapow/translations)
	install(FILES doc/kapow.1 DESTINATION ${CMAKE_INSTALL_MANDIR}/man1 COMPONENT doc)
	install(DIRECTORY icons/hicolor DESTINATION ${CMAKE_INSTALL_DATADIR}/icons)
	install(FILES icons/kapow.appdata.xml DESTINATION ${CMAKE_INSTALL_DATADIR}/metainfo)
	install(FILES icons/kapow.desktop DESTINATION ${CMAKE_INSTALL_DATADIR}/applications)
elseif(WIN32)
	# Use Qt6 macro until CMake provides something
	# https://bugreports.qt.io/browse/QTBUG-87618
	set_target_properties(kapow PROPERTIES
		WIN32_EXECUTABLE TRUE
		QT_TARGET_VERSION "${PROJECT_VERSION}"
		QT_TARGET_COMPANY_NAME "Graeme Gott"
		QT_TARGET_DESCRIPTION "Punch clock"
		QT_TARGET_COPYRIGHT "Copyright (C) 2022 Graeme Gott"
		QT_TARGET_PRODUCT_NAME "Kapow"
		QT_TARGET_RC_ICONS ${CMAKE_SOURCE_DIR}/icons/kapow.ico
	)
	_qt_internal_generate_win32_rc_file(kapow)
endif()
