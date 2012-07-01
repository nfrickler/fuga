#ifndef __SUPERVISOR_H__
#define __SUPERVISOR_H__

#include "MyWindow.h"
#include "MyTcpsocket.h"
#include "ContactHandler.h"
#include "FugaConfig.h"
#include "MeUser.h"
#include <QtGui>

class MyWindow;
class MyTcpsocket;
class ContactHandler;
class FugaConfig;
class MeUser;

class Supervisor  : public QObject {
	Q_OBJECT

	public:
		Supervisor();
		void showSelector();

		// get objects
		MyTcpsocket* getTcp();
		ContactHandler* getContactHandler();
        FugaConfig* getConfig();
		MyWindow* getWindow();
		MeUser* getMe();

	protected:
		MyTcpsocket* m_tcpsocket;
		ContactHandler* m_contacthandler;
        FugaConfig* m_config;
		MeUser* m_meuser;
		MyWindow* m_main_window;

	public slots:
		void slot_showSelector();

	private slots:
		void startModule();

};

#endif // __SUPERVISOR_H__
