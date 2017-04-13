#QMake project file for installing the material-icons-light theme.

#Load all the OS-specific install paths
include(../../src-qt5/OS-detect.pri)

#ensure QMake realizes that nothing is actually going to get compiled
TEMPLATE = aux

BASE_DIR = $${L_SHAREDIR}/icons/material-design-light

#Now setup the base directory/files
base.path = $${BASE_DIR}
base.files = LICENSE index.theme

actions.path = $${BASE_DIR}/scalable/actions
actions.files = actions/*.svg

applications.path = $${BASE_DIR}/scalable/applications
applications.files = applications/*.svg

categories.path = $${BASE_DIR}/scalable/categories
categories.files = categories/*.svg

devices.path = $${BASE_DIR}/scalable/devices
devices.files = devices/*.svg

emblems.path = $${BASE_DIR}/scalable/emblems
emblems.files = emblems/*.svg

emotes.path = $${BASE_DIR}/scalable/emotes
emotes.files = emotes/*.svg

international.path = $${BASE_DIR}/scalable/international
international.files = international/*.svg

mimetypes.path = $${BASE_DIR}/scalable/mimetypes
mimetypes.files = mimetypes/*.svg

places.path = $${BASE_DIR}/scalable/places
places.files = places/*.svg

status.path = $${BASE_DIR}/scalable/status
status.files = status/*.svg

INSTALLS = base actions applications categories devices emblems emotes international mimetypes places status
