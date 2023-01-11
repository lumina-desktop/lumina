cmake_minimum_required(VERSION 3.22 FATAL_ERROR)
project(lumina-desktop-utils VERSION 1.6.2 LANGUAGES C CXX) # TODO: inherit this from top-level

# CMake Settings (policies to do AUTOMOC as much as possible)
cmake_policy(SET CMP0057 NEW)
cmake_policy(SET CMP0071 NEW)
include(GNUInstallDirs)

# C++ Settings (pretend everything is modern)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include OS detection module
include("../OS-detect.cmake" NO_POLICY_SCOPE)

# ECM setup
include(FeatureSummary)
find_package(ECM 5.88.0 NO_MODULE)
set_package_properties(
    ECM
    PROPERTIES
    TYPE REQUIRED
    DESCRIPTION "Extra CMake Modules."
    URL "https://commits.kde.org/extra-cmake-modules"
)
feature_summary(
    WHAT REQUIRED_PACKAGES_NOT_FOUND
    FATAL_ON_MISSING_REQUIRED_PACKAGES
)

set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

# Dependencies
set(QT_VERSION 5.15.0)
find_package(
    Qt5
    ${QT_VERSION}
    CONFIG
    REQUIRED
        Concurrent
        Core
        Gui
        Multimedia
        MultimediaWidgets
        Network
        Quick
        Qml
        Svg
        Widgets
        X11Extras
        LinguistTools
        ThemeSupport
        DBus
        PrintSupport
)
find_package(
	Qt5Gui
	${QT_VERSION}
	REQUIRED
	Private
)
find_package(X11)
find_package(
    XCB
    MODULE
    COMPONENTS
        XCB
        KEYSYMS
        XKB
        XFIXES
        AUX
        COMPOSITE
        DAMAGE
        DPMS
        EWMH
        ICCCM
        IMAGE
        SCREENSAVER
    OPTIONAL_COMPONENTS XTEST
)
if(NOT X11_FOUND OR NOT XCB_XCB_FOUND)
    message(FATAL_ERROR "Required XCB component for X11 not found.")
endif()

add_definitions(
	-DL_ETCDIR=QStringLiteral\("${L_ETCDIR}"\)
	-DL_SHAREDIR=QStringLiteral\("${L_SHAREDIR}"\)
)

# Build options
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)

# Lumina Components
add_subdirectory(../core/libLumina liblumina)
add_subdirectory(lumina-fm)
add_subdirectory(lumina-screenshot)
add_subdirectory(lumina-fileinfo)
add_subdirectory(lumina-textedit)
add_subdirectory(lumina-archiver)
add_subdirectory(lumina-mediaplayer)
add_subdirectory(lumina-photo)
add_subdirectory(lumina-sudo)






