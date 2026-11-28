# Man page helper for the Lumina CMake build.
#
# lumina_add_manpage(<target> <file> <section>)
#
#   <target>   The executable the man page belongs to. The page is
#              compressed after this target is built.
#   <file>     Man page source in the current source directory,
#              for example "lumina-info.1".
#   <section>  Man section number, for example 1 or 8.
#
# The source file is passed through configure_file(@ONLY), so
# "@PROJECT_VERSION@" in the page is replaced with the version from
# project() in the top-level CMakeLists.txt. The result is gzipped and
# installed into man/man<section>.

function(lumina_add_manpage target file section)
    set(configured_page ${CMAKE_CURRENT_BINARY_DIR}/${file})
    set(compressed_page ${CMAKE_CURRENT_BINARY_DIR}/${file}.gz)

    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/${file} ${configured_page} @ONLY)

    add_custom_command(
        TARGET ${target}
        POST_BUILD
        COMMAND gzip -c ${configured_page} > ${compressed_page}
        BYPRODUCTS ${compressed_page}
    )

    install(
        FILES ${compressed_page}
        DESTINATION ${CMAKE_INSTALL_DATADIR}/man/man${section}
    )
endfunction()
