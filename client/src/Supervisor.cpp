#include "Supervisor.h"
#include "ModVideochat.h"
#include <QtGui>
#include <iostream>

using namespace std;

/* constructor
 */
Supervisor::Supervisor () {

	// create objects
	m_main_window = new MyWindow(this);
	m_contacthandler = new ContactHandler(this);
    m_config = new FugaConfig();
	m_tcpsocket = new MyTcpsocket(this);
	m_meuser = new MeUser(this);

	// draw login
	m_meuser->drawLogin();
}

/* return objects
 */
MyWindow* Supervisor::getWindow () {
	return m_main_window;
}

MyTcpsocket* Supervisor::getTcp () {
	return m_tcpsocket;
}
ContactHandler* Supervisor::getContactHandler () {
	return m_contacthandler;
}
FugaConfig* Supervisor::getConfig () {
	return m_config;
}
MeUser* Supervisor::getMe() {
	return m_meuser;
}

/* SLOT: show selector
 */
void Supervisor::slot_showSelector() {
	showSelector();
}

/* show module selector
 */
void Supervisor::showSelector () {
	cout << "Show selector..." << endl;

	// modules
	//char* allmodules[1][2];
	//allmodules[0][0] = "Videochat";
	//allmodules[0][1] = "Starte einen Videochat";

	// init selection
	QLabel *selector_text = new QLabel(QObject::tr("Was möchtest du machen?"));
	QGridLayout *mainLayout = new QGridLayout();
	mainLayout->addWidget(selector_text, 0, 0, 1, 3, Qt::AlignBottom);

	// add all modules for selection
	QLabel *current_label = new QLabel(tr("Starte einen Videochat"));
	QPushButton *current_selector = new QPushButton(tr("Videochat"));
	QObject::connect(current_selector, SIGNAL(clicked()), this, SLOT(startModule()));
	mainLayout->addWidget(current_selector, 1, 1, Qt::AlignTop);
	mainLayout->addWidget(current_label, 1, 2, Qt::AlignTop);

/*
	// add all modules to selection
	for (unsigned int i = 1; i < sizeof(allmodules); i++) {
		QString myqcurrent = tr(allmodules[i][1]);
		QLabel *current_label = new QLabel(tr(allmodules[i][0]));
		QPushButton *current_selector = new QPushButton(tr(allmodules[i][0]));
		QObject::connect(current_selector, SIGNAL(clicked()), this, SLOT(startModule(tr(allmodules[i][0]))));
		mainLayout->addWidget(current_selector, i, 1, Qt::AlignTop);
		mainLayout->addWidget(current_label, i, 2, Qt::AlignTop);
	}
*/

	// show main window
	QWidget *mainWidget = new QWidget();
	m_main_window->setCentralWidget(mainWidget);
	m_main_window->centralWidget()->setLayout(mainLayout);
	m_main_window->show();
	m_main_window->activateWindow();

}

/* start selected module
 */
void Supervisor::startModule () {

	// get videochat-object
	new ModVideochat(m_main_window, this);

}
