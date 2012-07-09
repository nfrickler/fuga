#ifndef __FugaWindow_H__
#define __FugaWindow_H__

#include <QMainWindow>
#include <QtGui>
#include <iostream>

#include "Fuga.h"

class Fuga;

class FugaWindow : public QMainWindow {
	Q_OBJECT

	public:
        FugaWindow (Fuga* in_Fuga);
        void showLogin();
        void showSelection();
        void showModule();

    protected:
        Fuga* m_Fuga;
		void drawMenu();
        void clearWindows();

        QMainWindow* m_second_window;
        QLineEdit* m_login_name;
        QLineEdit* m_login_password;
        QLineEdit* m_login_password2;
        std::string m_name;
        std::string m_password;
        QSignalMapper* m_selection_signalmapper;

    public slots:
        void slot_clicked_login();

    signals:
        void sig_login(std::string in_name, std::string in_password);
};

#endif // __FugaWindow_H__
