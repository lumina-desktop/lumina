include("$${PWD}/../../OS-detect.pri")

TEMPLATE = subdirs
CONFIG += recursive

SUBDIRS += main \
           wallpaper \
           autostart \
           defaultapps \
           keyboard \
           window-manager \
           desktop \
           panels \
           menu \
           locale \
           session \
           compositor \
           sound-theme \
           mouse \
           bluetooth

# Future LCC applications will be added here:
# SUBDIRS +=
