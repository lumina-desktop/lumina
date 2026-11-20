cmake_minimum_required(VERSION 3.16)
project(lumina-desktop VERSION 1.98.0 LANGUAGES C CXX)

# CMake Settings (policies to do AUTOMOC as much as possible)
cmake_policy(SET CMP0057 NEW)
cmake_policy(SET CMP0071 NEW)
if(POLICY CMP0177)
    cmake_policy(SET CMP0177 NEW)
endif()
include(GNUInstallDirs)

# C++ Settings (pretend everything is modern)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include OS detection module
include("OS-detect.cmake" NO_POLICY_SCOPE)

# Man page helper: lumina_add_manpage(<target> <file> <section>)
include("LuminaManPage.cmake")

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

# Qt Version Selection
# This is the Qt6 source tree (src-qt6). Qt6 is the default here.
# The Qt5 code paths are retained under version guards so that this tree can
# still be regression-built against Qt5 with -DWITH_QT6=OFF. The pristine
# Qt5-only tree lives in src-qt5/.
option(WITH_QT6 "Build with Qt6 instead of Qt5" ON)

if(WITH_QT6)
    # Qt6 build
    set(QT_MIN_VERSION 6.2.0)
    set(QT "Qt6")
    find_package(
        Qt6
        ${QT_MIN_VERSION}
        CONFIG
        REQUIRED
            Concurrent
            Core
            Core5Compat
            Gui
            Multimedia
            MultimediaWidgets
            Network
            Quick
            Qml
            Svg
            SvgWidgets
            Widgets
            LinguistTools
            DBus
    )
    # Qt6 does not have X11Extras or ThemeSupport as separate modules
    set(LUMINA_HAS_X11EXTRAS OFF)
    set(LUMINA_HAS_THEMESUPPORT OFF)
    # Qt6 GuiPrivate for theme engine
    find_package(Qt6 ${QT_MIN_VERSION} CONFIG COMPONENTS GuiPrivate)
    if(Qt6GuiPrivate_FOUND)
        set(LUMINA_HAS_GUIPRIVATE ON)
    else()
        set(LUMINA_HAS_GUIPRIVATE OFF)
    endif()
    message(STATUS "Building with Qt6 (${Qt6Core_VERSION})")
else()
    # Qt5 build (default)
    set(QT_MIN_VERSION 5.15.0)
    set(QT "Qt5")
    find_package(
        Qt5
        ${QT_MIN_VERSION}
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
    )
    find_package(
        Qt5Gui
        ${QT_MIN_VERSION}
        REQUIRED
        Private
    )
    set(LUMINA_HAS_X11EXTRAS ON)
    set(LUMINA_HAS_THEMESUPPORT ON)
    set(LUMINA_HAS_GUIPRIVATE ON)
    message(STATUS "Building with Qt5 (${Qt5Core_VERSION})")
endif()

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
add_subdirectory(core/libLumina)
add_subdirectory(core)
add_subdirectory(core-utils)
set(Utils ON)
if(Utils)
    add_subdirectory(desktop-utils)
endif()
