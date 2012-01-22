#ifndef MEUSER_H
#define MEUSER_H

#include "MyUser.h"
#include "Supervisor.h"
#include <QtGui>

class Supervisor;
class MyUser;

class MeUser : public MyUser {
    Q_OBJECT

	public:
		MeUser(Supervisor* mysupervisor); //:MyUser(mysupervisor, "") {};
		void drawLogin();
		void finishLogin();
		void failLogin();
		void finishRegist();
		void failRegist();
		std::string getName();

	protected:

		// widgets
		QMainWindow* m_second_window;
		QLineEdit* m_login_name;
		QLineEdit* m_login_password;
		QLineEdit* m_login_password2;
		std::string m_name;
		Supervisor* m_supervisor;
		MyUser* m_user;
		std::string m_password;

	public slots:
		void slotLogin();
		void slotDrawLogin();
		void slotDrawRegist();
		void slotRegist();

};

#endif // MEUSER_H
