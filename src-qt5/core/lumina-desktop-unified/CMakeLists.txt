# lumina-core/lumina-desktop-unified build, CMake edition. EXPERIMENTAL.
#
# SPDX-License-Identifier: BSD-2-Clause
# SPDX-FileCopyrightText: Copyright 2025 jt (q5sys) <jt@obs-sec.com>
# SPDX-FileCopyrightText: Copyright 2022 Adriaan de Groot <adridg@FreeBSD.org>
# NOTE: This CMake file is currently DISABLED
# lumina-desktop-unified is a complex next-generation desktop component
# that requires significant framework components (src-cpp) to be converted first

# Uncomment the following when ready to enable:

# # Require Qt 5.7+ as specified in the .pro file
# if(Qt5Core_VERSION VERSION_LESS "5.7.0")
#     message(FATAL_ERROR "Qt 5.7+ is required to use the Lumina Desktop!")
# endif()

# # Find required Qt5 modules
# find_package(Qt5 5.7.0 REQUIRED COMPONENTS
#     Core
#     Gui
#     Network
#     Widgets
#     X11Extras
#     Multimedia
#     MultimediaWidgets
#     Concurrent
#     Svg
#     Quick
#     Qml
# )

# # Find required XCB libraries
# find_library(XCB_LIBRARY xcb REQUIRED)
# find_library(XCB_EWMH_LIBRARY xcb-ewmh REQUIRED)
# find_library(XCB_ICCCM_LIBRARY xcb-icccm REQUIRED)
# find_library(XCB_IMAGE_LIBRARY xcb-image REQUIRED)
# find_library(XCB_COMPOSITE_LIBRARY xcb-composite REQUIRED)
# find_library(XCB_DAMAGE_LIBRARY xcb-damage REQUIRED)
# find_library(XCB_UTIL_LIBRARY xcb-util REQUIRED)
# find_library(XCB_KEYSYMS_LIBRARY xcb-keysyms REQUIRED)
# find_library(XCB_SCREENSAVER_LIBRARY xcb-screensaver REQUIRED)
# find_library(XDAMAGE_LIBRARY Xdamage REQUIRED)

# # Translation files
# set(TS_FILES
#     i18n/lumina-desktop_af.ts
#     i18n/lumina-desktop_ar.ts
#     i18n/lumina-desktop_az.ts
#     i18n/lumina-desktop_bg.ts
#     i18n/lumina-desktop_bn.ts
#     i18n/lumina-desktop_bs.ts
#     i18n/lumina-desktop_ca.ts
#     i18n/lumina-desktop_cs.ts
#     i18n/lumina-desktop_cy.ts
#     i18n/lumina-desktop_da.ts
#     i18n/lumina-desktop_de.ts
#     i18n/lumina-desktop_el.ts
#     i18n/lumina-desktop_en_GB.ts
#     i18n/lumina-desktop_en_ZA.ts
#     i18n/lumina-desktop_es.ts
#     i18n/lumina-desktop_et.ts
#     i18n/lumina-desktop_eu.ts
#     i18n/lumina-desktop_fa.ts
#     i18n/lumina-desktop_fi.ts
#     i18n/lumina-desktop_fr.ts
#     i18n/lumina-desktop_fr_CA.ts
#     i18n/lumina-desktop_gl.ts
#     i18n/lumina-desktop_he.ts
#     i18n/lumina-desktop_hi.ts
#     i18n/lumina-desktop_hr.ts
#     i18n/lumina-desktop_hu.ts
#     i18n/lumina-desktop_id.ts
#     i18n/lumina-desktop_is.ts
#     i18n/lumina-desktop_it.ts
#     i18n/lumina-desktop_ja.ts
#     i18n/lumina-desktop_ka.ts
#     i18n/lumina-desktop_ko.ts
#     i18n/lumina-desktop_lt.ts
#     i18n/lumina-desktop_lv.ts
#     i18n/lumina-desktop_mk.ts
#     i18n/lumina-desktop_mn.ts
#     i18n/lumina-desktop_ms.ts
#     i18n/lumina-desktop_mt.ts
#     i18n/lumina-desktop_nb.ts
#     i18n/lumina-desktop_nl.ts
#     i18n/lumina-desktop_pa.ts
#     i18n/lumina-desktop_pl.ts
#     i18n/lumina-desktop_pt.ts
#     i18n/lumina-desktop_pt_BR.ts
#     i18n/lumina-desktop_ro.ts
#     i18n/lumina-desktop_ru.ts
#     i18n/lumina-desktop_sk.ts
#     i18n/lumina-desktop_sl.ts
#     i18n/lumina-desktop_sr.ts
#     i18n/lumina-desktop_sv.ts
#     i18n/lumina-desktop_sw.ts
#     i18n/lumina-desktop_ta.ts
#     i18n/lumina-desktop_tg.ts
#     i18n/lumina-desktop_th.ts
#     i18n/lumina-desktop_tr.ts
#     i18n/lumina-desktop_uk.ts
#     i18n/lumina-desktop_uz.ts
#     i18n/lumina-desktop_vi.ts
#     i18n/lumina-desktop_zh_CN.ts
#     i18n/lumina-desktop_zh_HK.ts
#     i18n/lumina-desktop_zh_TW.ts
#     i18n/lumina-desktop_zu.ts
# )

# # Main source files
# set(MAIN_SOURCES
#     main.cpp
#     LSession.cpp
#     BootSplash.cpp
# )

# set(MAIN_HEADERS
#     global-includes.h
#     global-objects.h
#     LSession.h
#     BootSplash.h
#     JsonMenu.h
# )

# set(MAIN_FORMS
#     BootSplash.ui
# )

# # Events module sources
# set(EVENTS_SOURCES
#     src-events/LShortcutEvents.cpp
#     src-events/NativeEventFilter.cpp
#     src-events/NativeKeyToQt.cpp
#     src-events/NativeWindowSystem.cpp
#     src-events/Window-mgmt.cpp
# )

# set(EVENTS_HEADERS
#     src-events/LShortcutEvents.h
#     src-events/NativeEventFilter.h
#     src-events/NativeWindowSystem.h
# )

# # Screensaver module sources
# set(SCREENSAVER_SOURCES
#     src-screensaver/LLockScreen.cpp
#     src-screensaver/LScreenSaver.cpp
#     src-screensaver/SSBaseWidget.cpp
# )

# set(SCREENSAVER_HEADERS
#     src-screensaver/LLockScreen.h
#     src-screensaver/LScreenSaver.h
#     src-screensaver/SSBaseWidget.h
# )

# set(SCREENSAVER_FORMS
#     src-screensaver/LLockScreen.ui
# )

# # Desktop module sources
# set(DESKTOP_SOURCES
#     src-desktop/DesktopManager.cpp
#     src-desktop/QMLImageProvider.cpp
# )

# set(DESKTOP_HEADERS
#     src-desktop/RootWindow.h
#     src-desktop/DesktopManager.h
# )

# # Option to use QML or Widgets interface (default to Widgets for now)
# option(USE_QML "Use QML-based interface instead of Widgets" OFF)

# if(USE_QML)
#     set(DESKTOP_SOURCES ${DESKTOP_SOURCES}
#         src-desktop/RootWindow-QML.cpp
#     )
#     set(DESKTOP_HEADERS ${DESKTOP_HEADERS}
#         src-desktop/QMLImageProvider.h
#     )
#     add_definitions(-DUSE_QML="true")
# else()
#     set(DESKTOP_SOURCES ${DESKTOP_SOURCES}
#         src-desktop/RootWindow-Widgets.cpp
#     )
#     add_definitions(-DUSE_WIDGETS="true")
# endif()

# # TODO: Add src-cpp and framework sources when those CMake files are ready
# # This component depends heavily on the src-cpp framework which needs to be
# # converted to CMake first

# # Process UI forms
# qt5_wrap_ui(UI_HEADERS ${MAIN_FORMS} ${SCREENSAVER_FORMS})

# # Process translations (if i18n files exist)
# if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/i18n")
#     qt5_add_translation(QM_FILES ${TS_FILES})
# endif()

# # Create the executable
# add_executable(lumina-desktop-unified
#     ${MAIN_SOURCES}
#     ${MAIN_HEADERS}
#     ${EVENTS_SOURCES}
#     ${EVENTS_HEADERS}
#     ${SCREENSAVER_SOURCES}
#     ${SCREENSAVER_HEADERS}
#     ${DESKTOP_SOURCES}
#     ${DESKTOP_HEADERS}
#     ${UI_HEADERS}
#     ${QM_FILES}
# )

# # Set C++11 standard
# set_target_properties(lumina-desktop-unified PROPERTIES
#     CXX_STANDARD 11
#     CXX_STANDARD_REQUIRED ON
# )

# # Include directories
# target_include_directories(lumina-desktop-unified PRIVATE
#     ${CMAKE_CURRENT_SOURCE_DIR}
#     ${CMAKE_CURRENT_SOURCE_DIR}/src-events
#     ${CMAKE_CURRENT_SOURCE_DIR}/src-screensaver
#     ${CMAKE_CURRENT_SOURCE_DIR}/src-desktop
# )

# # Link libraries
# target_link_libraries(lumina-desktop-unified PRIVATE
#     libLumina
#     Qt5::Core
#     Qt5::Gui
#     Qt5::Network
#     Qt5::Widgets
#     Qt5::X11Extras
#     Qt5::Multimedia
#     Qt5::MultimediaWidgets
#     Qt5::Concurrent
#     Qt5::Svg
#     ${XCB_LIBRARY}
#     ${XCB_EWMH_LIBRARY}
#     ${XCB_ICCCM_LIBRARY}
#     ${XCB_IMAGE_LIBRARY}
#     ${XCB_COMPOSITE_LIBRARY}
#     ${XCB_DAMAGE_LIBRARY}
#     ${XCB_UTIL_LIBRARY}
#     ${XCB_KEYSYMS_LIBRARY}
#     ${XCB_SCREENSAVER_LIBRARY}
#     ${XDAMAGE_LIBRARY}
#     c
# )

# # Add QML/Quick libraries if using QML interface
# if(USE_QML)
#     target_link_libraries(lumina-desktop-unified PRIVATE
#         Qt5::Quick
#         Qt5::Qml
#     )
# endif()

# # Install executable
# install(TARGETS lumina-desktop-unified DESTINATION bin)

# # Install desktop session file
# install(
#     FILES lumina-desktop-unified.desktop
#     DESTINATION ${CMAKE_INSTALL_DATADIR}/xsessions
# )

# # Install defaults
# install(
#     DIRECTORY defaults/
#     DESTINATION ${CMAKE_INSTALL_DATADIR}/lumina-desktop
# )

# # Install extra files
# install(
#     DIRECTORY extrafiles/
#     DESTINATION ${CMAKE_INSTALL_DATADIR}/lumina-desktop
# )

# # Install translations (if they exist)
# if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/i18n" AND QM_FILES)
#     install(
#         FILES ${QM_FILES}
#         DESTINATION ${CMAKE_INSTALL_DATADIR}/lumina-desktop/i18n/
#     )
# endif()

message(STATUS "lumina-desktop-unified CMake file created but DISABLED")
message(STATUS "This component requires src-cpp framework conversion first")
message(STATUS "Enable by uncommenting the CMake code in this file")
