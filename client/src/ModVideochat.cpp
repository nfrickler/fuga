#include "ModVideochat.h"
#include <QtGui>
#include <iostream>
#include <QMessageBox>
#include <sstream>

using namespace std;

/* constructor
 */
ModVideochat::ModVideochat(MyWindow* mywindow, Supervisor* mysupervisor) {

	// save input in obj-vars
	m_main_window = mywindow;
	m_supervisor = mysupervisor;
	m_udpserver = NULL;

	// ask for name
	askForPerson();

	m_main_window->show();
	//m_main_window->showFullScreen();
	//myownwebcam->resize(1000,1000);

	cout << "ModVideochat: everything done" << endl;
}

/* show input-fields for person
 */
void ModVideochat::askForPerson () {

	// delete centralWidget
	delete m_main_window->centralWidget();

	// create new centralwidget
	QWidget *central_widget = new QWidget();
	m_main_window->setCentralWidget(central_widget);

	// create elements
	m_name_input = new QLineEdit();
	m_name_input->setText(tr("Testuser1"));

	// create buttons
	QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok
														| QDialogButtonBox::Cancel);
	connect(buttonBox, SIGNAL(accepted()), this, SLOT(setConnectionData()));
	connect(buttonBox, SIGNAL(rejected()), qApp, SLOT(quit()));

	// set layout
	QFormLayout* subLayout = new QFormLayout();
	subLayout->addRow(tr("Name"), m_name_input);

	QVBoxLayout* mainLayout = new QVBoxLayout();
	mainLayout->addLayout(subLayout, Qt::AlignBottom);
	mainLayout->addWidget(buttonBox, Qt::AlignTop);
	central_widget->setLayout(mainLayout);

	// show
	m_main_window->show();
}

/* set connection data from input-fields
 */
void ModVideochat::setConnectionData () {

	// read data from input-fields
	m_name = m_name_input->text().toStdString();

	// wait for videoReady
	m_islistening = false;
	connect(m_supervisor->getContactHandler(), SIGNAL(videoReady(std::string)), this, SLOT(slot_showVideo(std::string)));
	m_timer = new QTimer();
	connect(m_timer, SIGNAL(timeout()), this, SLOT(slot_videoFailed()));
	m_timer->start(10000);

	// add contact
	m_supervisor->getContactHandler()->addContact(m_name, true);
}

void ModVideochat::slot_showVideo (string name) {
	if ((name.compare(m_name) != 0) || m_islistening) return;
	m_timer->stop();
	m_islistening = true;
	showVideo();
}
void ModVideochat::slot_videoFailed () {
	cout << "ModVideochat: Connection to other host failed!" << endl;
	m_timer->stop();
	m_supervisor->showSelector();
}

/* show videochat
 */
void ModVideochat::showVideo () {

	// create new central Widget
	delete m_main_window->centralWidget();
	QWidget *central_widget = new QWidget();
	m_main_window->setCentralWidget(central_widget);

	// start server-thread for own webcam
	startServer(new QHostAddress(m_supervisor->getConfig()->getConfig("udp_ip").c_str()),
				m_supervisor->getConfig()->getInt("udp_port"),
				m_supervisor->getConfig()->getInt("udp_mtu"),
				m_supervisor->getConfig()->getInt("udp_quality"),
				m_supervisor->getConfig()->getInt("img_width"),
				m_supervisor->getConfig()->getInt("img_height"),
				m_supervisor->getConfig()->getConfig("video_path"),
				m_supervisor->getConfig()->getBool("video_isstreaming")
	);

	// chat
	vector<string> partner;
	partner.push_back(m_name);
	//m_chatbox = new Chatbox(m_supervisor, m_supervisor->getMe()->getName(), partner);

	// misc
	stringstream webcam_label("");
	webcam_label << "Webcam von " << m_name;

	// start video
	m_mywebcam = m_supervisor->getContactHandler()->startVideo(m_name);
	//connect(myownwebcam1, SIGNAL(is_resized(int,int)), m_mywebcam, SLOT(doResize(int,int)));

	// set layout
	QLabel* mylabel = new QLabel(tr(webcam_label.str().c_str()));
	mylabel->setMaximumHeight(30);
	QHBoxLayout* mainLayout = new QHBoxLayout();

	mainLayout->addWidget(m_mywebcam);
	mainLayout->addWidget(mylabel);
	//mainLayout->addWidget(m_chatbox);

	// show window
	central_widget->setLayout(mainLayout);
	m_main_window->show();

	// start listening
	m_mywebcam->start();
}

/* handle errors
 */
void ModVideochat::newError(char *msg) {
	QMessageBox::critical((QWidget*) this, "Error", tr(msg));
}
