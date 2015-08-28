Summary:            A lightweight, portable desktop environment
Name:               lumina-desktop
Version:            0.8.6
Release:            3%{?dist}
License:            BSD
Group:              System Environment/Base
Source0:            http://ftp.lumina-desktop.org/%{name}-%{version}.tar.gz
URL:                http://lumina-desktop.org
ExcludeArch:        s390 s390x
BuildRequires:      gcc, gcc-c++, qt-devel, qt-config, qt5-qttools, qt5-qttools-devel, qt5-qtbase-gui, qt5-qtmultimedia-devel, qt5-qtsvg-devel, qt5-qtx11extras-devel, xcb-util-image, xcb-util-image-devel, xcb-util-wm-devel, libxcb-devel, xcb-util-devel, phonon-devel, phonon-qt5-devel, libXcomposite-devel, libXdamage-devel, libXrender-devel, qt5-qtdeclarative-devel
Requires: fluxbox, qt5-style-oxygen, plasma-oxygen, xscreensaver, xbacklight, alsa-utils, acpi, numlockx, pavucontrol, xterm 

%description
The Lumina Desktop Environment is a lightweight system interface 
that is designed for use on any Unix-like operating system. 

%prep
%autosetup -n lumina-%{version}-Release

%build
qmake-qt5 CONFIG+=configure\ NO_I18N PREFIX=/usr/local LIBPREFIX=/lib64 

%install
make 
make INSTALL_ROOT=%{buildroot} install

%files
/usr/local/bin/Lumina-DE
/usr/local/bin/lumina-open
/usr/local/bin/lumina-config
/usr/local/bin/lumina-fm
/usr/local/bin/lumina-screenshot
/usr/local/bin/lumina-search
/usr/local/bin/lumina-info
/usr/local/bin/lumina-xconfig
/usr/local/bin/lumina-fileinfo
/usr/local/etc/luminaDesktop.conf.dist
/lib64/libLuminaUtils.so
/lib64/libLuminaUtils.so.1
/lib64/libLuminaUtils.so.1.0
/lib64/libLuminaUtils.so.1.0.0
/usr/local/include/LuminaXDG.h
/usr/local/include/LuminaUtils.h
/usr/local/include/LuminaX11.h
/usr/local/include/LuminaThemes.h
/usr/local/include/LuminaOS.h
/usr/local/include/LuminaSingleApplication.h
/usr/local/share/applications/lumina-fm.desktop
/usr/local/share/applications/lumina-screenshot.desktop
/usr/local/share/applications/lumina-search.desktop
/usr/local/share/applications/lumina-info.desktop
/usr/local/share/pixmaps/Lumina-DE.png
/usr/local/share/pixmaps/Insight-FileManager.png
/usr/share/xsessions/Lumina-DE.desktop
/usr/local/share/Lumina-DE/desktop-background.jpg
/usr/local/share/Lumina-DE/defaultapps.conf
/usr/local/share/Lumina-DE/luminaDesktop.conf
/usr/local/share/Lumina-DE/fluxbox-init-rc
/usr/local/share/Lumina-DE/fluxbox-keys
/usr/local/share/Lumina-DE/Login.ogg
/usr/local/share/Lumina-DE/Logout.ogg
/usr/local/share/Lumina-DE/colors/Lumina-Red.qss.colors
/usr/local/share/Lumina-DE/colors/Lumina-Green.qss.colors
/usr/local/share/Lumina-DE/colors/Lumina-Purple.qss.colors
/usr/local/share/Lumina-DE/colors/Lumina-Gold.qss.colors
/usr/local/share/Lumina-DE/colors/Lumina-Glass.qss.colors
/usr/local/share/Lumina-DE/colors/PCBSD10-Default.qss.colors
/usr/local/share/Lumina-DE/themes/Lumina-default.qss.template
/usr/local/share/Lumina-DE/themes/None.qss.template
/usr/local/share/Lumina-DE/quickplugins/quick-sample.qml
/usr/local/share/wallpapers/Lumina-DE/Lumina_Wispy_gold_1920x1080.jpg
/usr/local/share/wallpapers/Lumina-DE/Lumina_Wispy_green_1920x1080.jpg
/usr/local/share/wallpapers/Lumina-DE/Lumina_Wispy_purple_1920x1080.jpg
/usr/local/share/wallpapers/Lumina-DE/Lumina_Wispy_red_1920x1080.jpg

%changelog
* Thu Jul 30 2015 Jesse Smith jsmith@resonatingmedia.com
- Initial build
