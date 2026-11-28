# lumina-core/lumina-checkpass build, CMake edition. EXPERIMENTAL.
#
# SPDX-License-Identifier: BSD-2-Clause
# SPDX-FileCopyrightText: Copyright 2025 jt (q5sys) <jt@obs-sec.com>

# lumina-checkpass is a simple C program for password validation
# It requires PAM library and needs to be installed with setuid permissions

# Find PAM library
find_library(PAM_LIBRARY pam REQUIRED)
if(NOT PAM_LIBRARY)
    message(FATAL_ERROR "PAM library not found. Required for lumina-checkpass.")
endif()

# Create the executable from C source
add_executable(lumina-checkpass main.c)

# Set C standard (not C++)
set_target_properties(lumina-checkpass PROPERTIES
    C_STANDARD 99
    C_STANDARD_REQUIRED ON
)

# Link with PAM library
target_link_libraries(lumina-checkpass PRIVATE ${PAM_LIBRARY})

# Install to sbin directory (not regular bin)
install(TARGETS lumina-checkpass DESTINATION sbin)

# Set special permissions for password checking (setuid root)
# This is critical for PAM authentication to work properly
install(CODE "
    execute_process(COMMAND chmod 4555 \${CMAKE_INSTALL_PREFIX}/sbin/lumina-checkpass)
    message(STATUS \"Set setuid permissions on lumina-checkpass\")
")
