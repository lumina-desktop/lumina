# Lumina Components
add_subdirectory(lumina-desktop)
add_subdirectory(lumina-session)
add_subdirectory(lumina-open)
add_subdirectory(lumina-info)
add_subdirectory(lumina-pingcursor)
add_subdirectory(lumina-theme-engine)

# Additional files

install(
    FILES menu-scripts/README.md menu-scripts/ls.json.sh
    DESTINATION ${CMAKE_INSTALL_DATADIR}/lumina-desktop/menu-scripts
)
install(
    FILES
        themes/DarkGlass.qss.template
        themes/Glass.qss.template
        themes/Lumina-default.qss.template
        themes/None.qss.template
    DESTINATION ${CMAKE_INSTALL_DATADIR}/lumina-desktop/themes
)

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
