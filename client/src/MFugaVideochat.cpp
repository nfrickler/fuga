#include "MFugaVideochat.h"
#include <QtGui>
#include <QMessageBox>
#include <iostream>
#include <sstream>

#include "FugaContact.h"

using namespace std;

// constructors
MFugaVideochat::MFugaVideochat(FugaWindow* mywindow, Fuga* in_Fuga) {

	// save input in obj-vars
	m_main_window = mywindow;
    m_Fuga = in_Fuga;

	// ask for name
	askForPerson();

	m_main_window->show();
	//m_main_window->showFullScreen();
	//myownwebcam->resize(1000,1000);

    cout << "MFugaVideochat: everything done" << endl;
}

/* show input-fields for person
 */
void MFugaVideochat::askForPerson () {

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
void MFugaVideochat::setConnectionData () {

	// read data from input-fields
	m_name = m_name_input->text().toStdString();

    // load contact
    m_Contact = m_Fuga->getContacts()->getContact(m_name);

	// wait for videoReady
	m_islistening = false;
    connect(m_Contact, SIGNAL(sig_info()), this, SLOT(slot_showVideo()));
	m_timer = new QTimer();
	connect(m_timer, SIGNAL(timeout()), this, SLOT(slot_videoFailed()));
	m_timer->start(10000);

	// add contact
   // m_Fuga->getContacts()->addContact(m_name, true);
}

void MFugaVideochat::slot_showVideo () {
	m_timer->stop();
	m_islistening = true;
	showVideo();
}
void MFugaVideochat::slot_videoFailed () {
    cout << "MFugaVideochat: Connection to other host failed!" << endl;
	m_timer->stop();
    m_Fuga->getWindow()->showSelection();
}

/* show videochat
 */
void MFugaVideochat::showVideo () {

	// create new central Widget
	delete m_main_window->centralWidget();
	QWidget *central_widget = new QWidget();
	m_main_window->setCentralWidget(central_widget);

	// start streaming
    m_Contact->startStreaming();

	// create label for video
	stringstream webcam_label("");
	webcam_label << "Webcam von " << m_name;
	QLabel* mylabel = new QLabel(tr(webcam_label.str().c_str()));
	mylabel->setSizePolicy(QSizePolicy(QSizePolicy::Minimum, QSizePolicy::Maximum));

	// create myvideo
    FugaVideo* othercam = m_Contact->Video();
	if (othercam == NULL) {
		newError("Could not start Video!");
        m_Fuga->getWindow()->showSelection();
		return;
	}
	othercam->setStyleSheet("background:#000;");
	othercam->setSizePolicy(QSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding));

	// create chat
	vector<string> partner;
	partner.push_back(m_name);
    //m_chatbox = new Chatbox(m_Fuga, m_Fuga->getMe()->getName(), partner);
    //m_chatbox->setStyleSheet("background:#222;");

	// merge in layout
	QGridLayout* layout_grid = new QGridLayout();
	layout_grid->addWidget(mylabel, 1, 0);
	layout_grid->addWidget(othercam, 0, 0);
    //layout_grid->addWidget(m_chatbox, 0, 2, 1, 2);
	central_widget->setLayout(layout_grid);
	central_widget->setStyleSheet("color:#FFF;background:#000;");

	// start listening
	m_main_window->show();
	othercam->start();
}

/* handle errors
 */
void MFugaVideochat::newError(char *msg) {
	QMessageBox::critical((QWidget*) this, "Error", tr(msg));
}