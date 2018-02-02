#Show the QML files to lupdate for translation purposes - not for the actual build
lupdate_only{
  SOURCES *= $${PWD}/StatusTray.qml \
		$${PWD}/status_tray/TrayIcon.qml \
		$${PWD}/status_tray/VolumeButton.qml \
		$${PWD}/status_tray/NetworkButton.qml \
		$${PWD}/status_tray/BatteryButton.qml \
		$${PWD}/status_tray/UpdateButton.qml \
		$${PWD}/Clock_Digital.qml \
		$${PWD}/Spacer.qml
}
