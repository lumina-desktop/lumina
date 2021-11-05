// =========================
//  Project Trident Network Manager
//  Available under the 3-clause BSD License
//  Initially written in February of 2019
//   by Ken Moore <ken@project-trident.org>
// =========================
#ifndef _NETWORK_BACKEND_H
#define _NETWORK_BACKEND_H

#include <QObject>
#include <QStringList>
#include <QProcess>
#include <QJsonObject>
#include <QJsonArray>
#include <QNetworkConfigurationManager>
#include <QThread>
#include <QFile>
#include <QDir>
#include <QFileInfo>
#include <QDateTime>

class Networking : public QObject {
	Q_OBJECT
public:
	enum State{ StateUnknown=0, StateRunning, StateStopped, StateRestart };
	Networking(QObject *parent = 0);
	~Networking();

	QStringList list_devices();
	QJsonObject list_config();
	QJsonObject current_info(QString device);
	bool set_config(QJsonObject config);
	State deviceState(QString device);

	// Wifi specific functionality
	QJsonObject wifi_scan_results();
	QJsonArray known_wifi_networks();
	QJsonObject active_wifi_network();
	bool is_known(QJsonObject obj);

	bool save_wifi_network(QJsonObject obj, bool clearonly = false);
	bool remove_wifi_network(QString id);
	bool connect_to_wifi_network(QJsonObject obj, bool noretry = false);

	// DNS specific functionality
	QString current_dns();
	QJsonObject custom_dns_settings();
	bool save_custom_dns_settings(QJsonObject);

	// Wireguard specific functionality
	QJsonObject current_wireguard_profiles();
	bool add_wireguard_profile(QString path);
	bool remove_wireguard_profile(QString name);
	bool start_wireguard_profile(QString name);
	bool stop_wireguard_profile(QString name);

	// Firewall functionality
	bool firewall_is_running();
	bool start_firewall();
	bool stop_firewall();
	QJsonObject current_firewall_files();
	bool change_firewall_profile(QString path);
	bool save_firewall_rules(QString path, QStringList contents);
	bool remove_firewall_rules(QString path);
	QJsonObject known_services();

	//General Purpose functions
	static QStringList readFile(QString path);
	static bool writeFile(QString path, QStringList contents);
	static bool sameNetwork(QJsonObject A, QJsonObject B);
	bool writeFileAsRoot(QString path, QStringList contents, QStringList loadCmd = QStringList(), QString perms = "644");
	bool runScriptAsRoot(QStringList script, QString sname);

	static QString CmdOutput(QString proc, QStringList args, QProcess *qsudoproc = 0);
	static int CmdReturnCode(QString proc, QStringList args, QProcess *qsudoproc = 0);
	static bool CmdReturn(QString proc, QStringList args, QProcess *qsudoproc = 0);

private:
	QNetworkConfigurationManager *NETMAN;
	QJsonObject last_wifi_scan;
	QProcess *qSudoProc;
	void performWifiScan(QStringList wifi_devices); //designed to be run in a separate thread
	void parseWifiScanResults(QStringList info);
	QString knownNetworkID(QJsonObject info);

public slots:
	bool setDeviceState(QString device, State stat);
	void startWifiScan();

private slots:

signals:
	void starting_wifi_scan();
	void new_wifi_scan_results();
	void finished_wifi_scan();
};
#endif
