//===========================================
//  Lumina-DE source code
//  Copyright (c) 2015, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
//  This plugin is a simple DBUS monitor which display's messages that come in
//===========================================
#ifndef _LUMINA_DESKTOP_MESSAGE_CENTER_PLUGIN_H
#define _LUMINA_DESKTOP_MESSAGE_CENTER_PLUGIN_H

#include <QListWidget>
#include <QToolButton>
#include <QFrame>

#include <QTimer>
#include "../LDPlugin.h"

class MessageCenterPlugin : public LDPlugin{
	Q_OBJECT
public:
	MessageCenterPlugin(QWidget* parent, QString ID);
	~MessageCenterPlugin();
	
private:
	//QDBusConnection *sess, *sys;
	QListWidget *list_messages;
	QFrame *frame;
	QToolButton *tool_clearall; //clear all messages
	QToolButton *tool_clearone; //clear selected message

private slots:
	//void newMessage(QDBusMessage *message);
	void clearAllMessages();
	void clearSelectedMessage();

	void loadIcons();

public slots:
	void LocaleChange(){
	  QTimer::singleShot(0,this, SLOT(loadIcons()));
	}
	void ThemeChange(){
	  QTimer::singleShot(0,this, SLOT(loadIcons()));
	}

};
#endif
