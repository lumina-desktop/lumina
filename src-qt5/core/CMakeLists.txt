# Lumina Components
add_subdirectory(lumina-desktop)
add_subdirectory(lumina-session)
add_subdirectory(lumina-open)
add_subdirectory(lumina-info)
add_subdirectory(lumina-pingcursor)
add_subdirectory(lumina-checkpass)
# add_subdirectory(lumina-desktop-unified)  # DISABLED - requires src-cpp framework
add_subdirectory(lumina-theme-engine)
# add_subdirectory(colors)  # LEGACY - deprecated color files (use lumina-theme-engine instead)
add_subdirectory(menu-scripts)
add_subdirectory(themes)

# Additional files

# This is a typo, also in the original core.pro, should be xtrafiles/theme.cfg
#
#install(
#    FILES extrafiles/theme.cfg
#    DESTINATION ${CMAKE_INSTALL_DATADIR}/fluxbox/style/lumina-dark/theme.cfg
#)

install(
    FILES xtrafiles/globs2
    DESTINATION ${CMAKE_INSTALL_DATADIR}/lumina-desktop
)

# The icon theme lives outside of core, but should be
# installed along with it. The icon-themes here is the
# binary-directory to use during the build; there isn't
# much to do there except write Makefiles, since the
# icon themes do nothing but copy files out of the source-tree.
add_subdirectory(../../icon-theme icon-themes)
