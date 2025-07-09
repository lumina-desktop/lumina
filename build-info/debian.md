Provided by `Koratsuki`

The following procedure was done on a fresh netinstall Debian 11/Devuan 4 system, on Ubuntu maybe some dependencies could have a different name, but is almost the same.

After setting up dns config:

nano /etc/resolv.conf

And the repositories config:

nano /etc/apt/sources.list

We'll update/upgrade the currrent system:

apt update && apt upgrade -y

I will install some tools I need:

apt install mc net-tools dnsutils git rsync wget

Now, download the lumina package:

cd /opt
wget -c https://github.com/lumina-desktop/lumina/archive/v1.6.2.tar.gz

Or:

git clone https://github.com/lumina-desktop/lumina.git

tar -zxvf lumina-1.6.2.tar.gz
cd lumina-1.6.2

Reading the README.md we will need some tools to compile the desktop environment:

apt install -y qml qt5-qmake qtbase5-{dev,dev-tools} qtbase5-private-dev qtchooser qtdeclarative5-{dev,dev-tools} qt5-qmltooling-plugins \
 qml-module-qtquick2 qml-module-qtqml-models2 qml-module-qtmultimedia qml-module-qtgraphicaleffects build-essential \
 qml-module-qtquick-{controls,controls2,dialogs,extras,layouts,privatewidgets,virtualkeyboard,window2,templates2} qtbase5-dev qtchooser qt5-qmake qtbase5-dev-tools \
 libqt5x11extras5 libqt5x11extras5-dev libqt5multimedia5 libqt5multimediawidgets5 libqt5svg5 libqt5svg5-dev qtmultimedia5-dev libxdamage-dev \
 libxcb-{ewmh-dev,dpms0-dev,util0-dev,icccm4-dev,image0-dev,composite0-dev,damage0-dev,randr0-dev} libxcursor-dev compton compton-{conf,conf-l10n}

Let's build the project:

qmake PREFIX=/opt/lumina LIBPREFIX=/opt/lumina/lib CONFIG+=WITH_I18N

This line will show you something like this:

Project MESSAGE: Build OS Info: Linux, x86_64, #1 SMP Debian 5.10.84-1 (2021-12-08)
Project MESSAGE:  - Detected Qt Version: 5.15.2
Project MESSAGE: Build Settings Loaded: Linux -Devuan
 
Now let's compile & install:

make -j`nproc`

make install

echo 'export PATH=$PATH:/opt/lumina/bin' >> /etc/profile
source /etc/profile

Linking icons:

ln -s /opt/lumina/share/icons/material-design-light /usr/share/icons/
ln -s /opt/lumina/share/icons/material-design-dark /usr/share/icons/

Edit and adjust for the session manager:

nano /usr/share/xsessions/Lumina-DE.desktop

Exec=/opt/lumina/bin/start-lumina-desktop
TryExec=/opt/lumina/bin/start-lumina-desktop
DesktopNames=Lumina-DE
Type=XSession
Terminal=false

[X-Window Manager]
SessionManaged=true

So, now restart your sessions manager[lighdm/sddm/gdm] and choose Lumina-DE, login and enjoy!

