# Enable hardened build by default
%global _hardened_build 1

# Enable pulling translations by default
%bcond_without pull_translations

Summary:            A lightweight, portable desktop environment
Name:               lumina-desktop
Version:            0.8.8
Release:            1%{?dist}
License:            BSD
Group:              User Interface/Desktops
# Main source
Source0:            https://github.com/pcbsd/lumina/archive/v%{version}-Release/lumina-%{version}-Release.tar.gz
%if %{with pull_translations}
# Translations
Source1:            https://github.com/pcbsd/lumina-i18n/raw/master/dist/lumina-i18n.txz
%endif
URL:                http://lumina-desktop.org

# Exclude IBM ESA/390 and ESA System/z architectures
ExcludeArch:        s390 s390x

# Compiler requirements
BuildRequires:      gcc, gcc-c++

%if %{with pull_translations}
# Translation installation requirements
BuildRequires:      tar
%endif

# Qt requirements
BuildRequires:      qt5-qttools, qt5-qttools-devel, qt5-linguist
BuildRequires:      qt5-qtbase-devel, qt5-qtmultimedia-devel, qt5-qtdeclarative-devel
BuildRequires:      qt5-qtsvg-devel, qt5-qtx11extras-devel

# X component requirements
BuildRequires:      xcb-util-image, xcb-util-image-devel, xcb-util-wm-devel, libxcb-devel, xcb-util-devel
BuildRequires:      libXcomposite-devel, libXdamage-devel, libXrender-devel

# Runtime requirements
Requires:           alsa-utils, acpi, numlockx, pavucontrol, sysstat
Requires:           xscreensaver, xbacklight, xterm
Requires:           qt5-style-oxygen, plasma-oxygen 
Requires:           fluxbox

# Enforce the library subpackage version requirement
Requires:           %{name}-libs = %{version}-%{release}

# Desktop requirements
Requires:           lumina-open = %{version}-%{release}
Requires:           lumina-config = %{version}-%{release}
Requires:           lumina-fm = %{version}-%{release}
Requires:           lumina-screenshot = %{version}-%{release}
Requires:           lumina-search = %{version}-%{release}
Requires:           lumina-info = %{version}-%{release}
Requires:           lumina-xconfig = %{version}-%{release}
Requires:           lumina-fileinfo = %{version}-%{release}


%description
The Lumina Desktop Environment is a lightweight system interface
that is designed for use on any Unix-like operating system.

%package libs
Summary:            Libraries for Lumina Desktop
Group:              System Environment/Libraries

%description libs
This package provides the libraries for the Lumina Desktop
Environment.

%package devel
Summary:            Development libraries for Lumina Desktop
Group:              Development/Libraries
Requires:           %{name}-libs = %{version}-%{release}

%description devel
This package provides the files needed to develop plugins
or extensions for the Lumina Desktop Environment, or
to develop applications that use Lumina Desktop libraries.

%package -n lumina-open
Summary:            xdg-open style utility for Lumina Desktop
Group:              User Interface/Desktops
Requires:           %{name}-libs = %{version}-%{release}

%description -n lumina-open
This package provides lumina-open, which handles opening of
files and URLs according to the system-wide mimetype association.
It also provides an optional selector if more than one application
is assigned with the given url or file type.

%package -n lumina-config
Summary:            Configuration utility for Lumina Desktop
Group:              User Interface/Desktops
Requires:           %{name}-libs = %{version}-%{release}

%description -n lumina-config
This package provides lumina-config, which allows changing
various aspects of lumina and fluxbox, like the wallpaper being
used, theme, icons, panel (and plugins), startup and default
applications, desktop menu and more.

%package -n lumina-fm
Summary:            File manager for Lumina Desktop
Group:              User Interface/Desktops
Requires:           %{name}-libs = %{version}-%{release}

%description -n lumina-fm
This package provides lumina-fm, which is a simple file manager
with support for multiple view modes, tabbed browsing,
including an integrated slideshow-based picture viewer.


%package -n lumina-screenshot
Summary:            Screenshot utility for Lumina Desktop
Group:              User Interface/Desktops
Requires:           %{name}-libs = %{version}-%{release}

%description -n lumina-screenshot
This package provides lumina-screenshot, which is a simple
screenshot utility that allows to snapshot the whole desktop
or a single window after a configurable delay.

Optionally the window border can be hidden when taking a
screenshot of a single window.


%package -n lumina-search
Summary:            Search utility for Lumina Desktop
Group:              User Interface/Desktops
Requires:           %{name}-libs = %{version}-%{release}

%description -n lumina-search
This package provides lumina-search, which is a simple
search utility that allows to search for applications or
files and directories in the home directory and launch
or open them.


%package -n lumina-info
Summary:            Basic information utility for Lumina Desktop
Group:              User Interface/Desktops
Requires:           %{name}-libs = %{version}-%{release}

%description -n lumina-info
This package provides lumina-info, which is a simple
utility that displays various information about the Lumina
installation, like paths, contributors, license or version.


%package -n lumina-xconfig
Summary:            X server display configuration tool for Lumina Desktop
Group:              User Interface/Desktops
Requires:           %{name}-libs = %{version}-%{release}

%description -n lumina-xconfig
This package provides lumina-xconfig, which is a simple
multi-head aware display configuration tool for configuring
the X server.


%package -n lumina-fileinfo
Summary:            Desktop file editor for Lumina Desktop
Group:              User Interface/Desktops
Requires:           %{name}-libs = %{version}-%{release}

%description -n lumina-fileinfo
This package provides lumina-fileinfo, which is an
advanced desktop file (menu) editor.


%prep
%setup -q -n lumina-%{version}-Release

%build
%qmake_qt5 CONFIG+=configure PREFIX="%{_prefix}" LIBPREFIX="%{_libdir}" QT5LIBDIR="%{_qt5_prefix}"
make %{?_smp_mflags}

%install
# Install the desktop
make INSTALL_ROOT=%{buildroot} install

%if %{with pull_translations}
# Install translations
mkdir -p %{buildroot}%{_datadir}/Lumina-DE/i18n
tar xvf %{SOURCE1} -C %{buildroot}%{_datadir}/Lumina-DE/i18n
%endif

# Move config folder to correct location
mv %{buildroot}%{_prefix}%{_sysconfdir} %{buildroot}

# Fix paths in desktop files
find %{buildroot}/ -name *.desktop -exec sed -i "s:/usr/local/:/usr/:g" {} \;

# Create proper config file
cp %{buildroot}%{_datadir}/Lumina-DE/luminaDesktop.conf %{buildroot}%{_sysconfdir}/luminaDesktop.conf
sed -i "s:/usr/local/share/applications/firefox.desktop:firefox:g" %{buildroot}%{_sysconfdir}/luminaDesktop.conf
sed -i "s:/usr/local/share/applications/thunderbird.desktop:thunderbird:g" %{buildroot}%{_sysconfdir}/luminaDesktop.conf


%post libs -p /sbin/ldconfig
%postun libs -p /sbin/ldconfig

%files libs
%license LICENSE
%{_libdir}/libLuminaUtils.so.1
%{_libdir}/libLuminaUtils.so.1.0
%{_libdir}/libLuminaUtils.so.1.0.0

%files devel
%license LICENSE
%{_libdir}/libLuminaUtils.so
%{_includedir}/LuminaXDG.h
%{_includedir}/LuminaUtils.h
%{_includedir}/LuminaX11.h
%{_includedir}/LuminaThemes.h
%{_includedir}/LuminaOS.h
%{_includedir}/LuminaSingleApplication.h

%files
%license LICENSE
%{_bindir}/Lumina-DE
%config(noreplace) %{_sysconfdir}/luminaDesktop.conf
%{_sysconfdir}/luminaDesktop.conf.dist
%{_datadir}/pixmaps/Lumina-DE.png
%{_datadir}/xsessions/Lumina-DE.desktop
%{_datadir}/Lumina-DE/desktop-background.jpg
%{_datadir}/Lumina-DE/luminaDesktop.conf
%{_datadir}/Lumina-DE/fluxbox-init-rc
%{_datadir}/Lumina-DE/fluxbox-keys
%{_datadir}/Lumina-DE/Login.ogg
%{_datadir}/Lumina-DE/Logout.ogg
%{_datadir}/Lumina-DE/colors/Lumina-Red.qss.colors
%{_datadir}/Lumina-DE/colors/Lumina-Green.qss.colors
%{_datadir}/Lumina-DE/colors/Lumina-Purple.qss.colors
%{_datadir}/Lumina-DE/colors/Lumina-Gold.qss.colors
%{_datadir}/Lumina-DE/colors/Lumina-Glass.qss.colors
%{_datadir}/Lumina-DE/colors/PCBSD10-Default.qss.colors
%{_datadir}/Lumina-DE/themes/Lumina-default.qss.template
%{_datadir}/Lumina-DE/themes/None.qss.template
%{_datadir}/Lumina-DE/quickplugins/quick-sample.qml
%{_datadir}/Lumina-DE/colors/Blue-Light.qss.colors
%{_datadir}/Lumina-DE/colors/Grey-Dark.qss.colors
%{_datadir}/Lumina-DE/colors/Solarized-Dark.qss.colors
%{_datadir}/Lumina-DE/colors/Solarized-Light.qss.colors
%{_datadir}/wallpapers/Lumina-DE/Lumina_Wispy_gold.jpg
%{_datadir}/wallpapers/Lumina-DE/Lumina_Wispy_green.jpg
%{_datadir}/wallpapers/Lumina-DE/Lumina_Wispy_purple.jpg
%{_datadir}/wallpapers/Lumina-DE/Lumina_Wispy_red.jpg
%{_datadir}/Lumina-DE/i18n/lumina-desktop*.qm

%files -n lumina-open
%license LICENSE
%{_bindir}/lumina-open
%{_datadir}/Lumina-DE/i18n/lumina-open*.qm

%files -n lumina-config
%license LICENSE
%{_bindir}/lumina-config
%{_datadir}/Lumina-DE/i18n/lumina-config*.qm

%files -n lumina-fm
%license LICENSE
%{_bindir}/lumina-fm
%{_datadir}/Lumina-DE/i18n/lumina-fm*.qm
%{_datadir}/pixmaps/Insight-FileManager.png
%{_datadir}/applications/lumina-fm.desktop

%files -n lumina-screenshot
%license LICENSE
%{_bindir}/lumina-screenshot
%{_datadir}/Lumina-DE/i18n/lumina-screenshot*.qm
%{_datadir}/applications/lumina-screenshot.desktop

%files -n lumina-search
%license LICENSE
%{_bindir}/lumina-search
%{_datadir}/Lumina-DE/i18n/lumina-search*.qm
%{_datadir}/applications/lumina-search.desktop

%files -n lumina-info
%license LICENSE
%{_bindir}/lumina-info
%{_datadir}/Lumina-DE/i18n/lumina-info*.qm
%{_datadir}/applications/lumina-info.desktop

%files -n lumina-xconfig
%license LICENSE
%{_bindir}/lumina-xconfig
%{_datadir}/Lumina-DE/i18n/lumina-xconfig*.qm

%files -n lumina-fileinfo
%license LICENSE
%{_bindir}/lumina-fileinfo
%{_datadir}/Lumina-DE/i18n/lumina-fileinfo*.qm


%changelog
* Wed Dec 23 2015 Neal Gompa <ngompa13@gmail.com>
- Update to 0.8.8
- Bring it closer to Fedora guidelines
- Break out Lumina Desktop package into subpackages
- Unify 32-bit and 64-bit packaging

* Tue Oct 27 2015 Jesse Smith <jsmith@resonatingmedia.com>
- Update to 0.8.7

* Thu Aug 20 2015 Jesse Smith <jsmith@resonatingmedia.com>
- Updated for 32-bit

* Thu Jul 30 2015 Jesse Smith <jsmith@resonatingmedia.com>
- Initial build
