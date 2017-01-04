#Lumina Desktop Panel Plugin files

SOURCES += $$PWD/userbutton/LUserButton.cpp \
	$$PWD/userbutton/UserWidget.cpp \
	$$PWD/userbutton/UserItemWidget.cpp \
	$$PWD/desktopbar/LDeskBar.cpp \
	$$PWD/taskmanager/LTaskManagerPlugin.cpp \
	$$PWD/taskmanager/LTaskButton.cpp \
	$$PWD/systemtray/LSysTray.cpp \
	$$PWD/systemtray/TrayIcon.cpp \
	$$PWD/clock/LClock.cpp \
	$$PWD/battery/LBattery.cpp \
	$$PWD/desktopswitcher/LDesktopSwitcher.cpp \
	$$PWD/systemdashboard/LSysDashboard.cpp \
	$$PWD/systemdashboard/SysMenuQuick.cpp \
	$$PWD/showdesktop/LHomeButton.cpp \
	$$PWD/appmenu/LAppMenuPlugin.cpp \
	$$PWD/applauncher/AppLaunchButton.cpp \
	$$PWD/systemstart/LStartButton.cpp \
	$$PWD/systemstart/StartMenu.cpp \
	$$PWD/systemstart/ItemWidget.cpp \
	$$PWD/audioplayer/LPAudioPlayer.cpp \
	$$PWD/audioplayer/PPlayerWidget.cpp \
	$$PWD/jsonmenu/PPJsonMenu.cpp
	
HEADERS += $$PWD/RotateToolButton.h \
	$$PWD/userbutton/LUserButton.h \
	$$PWD/userbutton/UserWidget.h \
	$$PWD/userbutton/UserItemWidget.h \
	$$PWD/desktopbar/LDeskBar.h \
	$$PWD/systemtray/LSysTray.h \
	$$PWD/systemtray/TrayIcon.h \
	$$PWD/spacer/LSpacer.h \
	$$PWD/line/LLine.h \
	$$PWD/clock/LClock.h \
	$$PWD/battery/LBattery.h \
	$$PWD/desktopswitcher/LDesktopSwitcher.h \
	$$PWD/taskmanager/LTaskManagerPlugin.h \
	$$PWD/taskmanager/LTaskButton.h \
	$$PWD/systemdashboard/LSysDashboard.h \
	$$PWD/systemdashboard/SysMenuQuick.h \
	$$PWD/showdesktop/LHomeButton.h \
	$$PWD/appmenu/LAppMenuPlugin.h \
	$$PWD/applauncher/AppLaunchButton.h \
	$$PWD/systemstart/LStartButton.h \
	$$PWD/systemstart/StartMenu.h \
	$$PWD/systemstart/ItemWidget.h \
	$$PWD/audioplayer/LPAudioPlayer.h \
	$$PWD/audioplayer/PPlayerWidget.h \
	$$PWD/jsonmenu/PPJsonMenu.h
#	$$PWD/quickcontainer/QuickPPlugin.h

FORMS +=	 $$PWD/userbutton/UserWidget.ui \
	$$PWD/systemdashboard/SysMenuQuick.ui \
	$$PWD/systemstart/StartMenu.ui \
	$$PWD/audioplayer/PPlayerWidget.ui
	
