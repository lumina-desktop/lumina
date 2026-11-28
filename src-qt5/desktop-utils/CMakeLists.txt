find_package(
    Qt5
    ${QT_VERSION}
    CONFIG
    REQUIRED
        PrintSupport
)

# Lumina Components
add_subdirectory(lumina-fm)
add_subdirectory(lumina-screenshot)
add_subdirectory(lumina-fileinfo)
add_subdirectory(lumina-textedit)
add_subdirectory(lumina-archiver)
add_subdirectory(lumina-mediaplayer)
add_subdirectory(lumina-photo)
add_subdirectory(lumina-sudo)
