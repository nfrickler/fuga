#include "MFugaVideochat.h"
#include "FugaChat.h"
#include <QMessageBox>
#include <iostream>
#include <sstream>

#include "FugaContact.h"

using namespace std;

// constructor
MFugaVideochat::MFugaVideochat(FugaWindow* mywindow, Fuga* in_Fuga) {

	// save input in obj-vars
	m_main_window = mywindow;
    m_Fuga = in_Fuga;

	// ask for name
	askForPerson();
}

// show input-fields for person
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
    QDialogButtonBox* buttonBox =
        new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
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

// set connection data from input-fields
void MFugaVideochat::setConnectionData () {

    // load contact
    m_Contact = m_Fuga->getContacts()->getContact(m_name_input->text().toStdString());

    // wait for videoReady
    connect(m_Contact, SIGNAL(sig_fetched()), this, SLOT(slot_showVideo()));
	m_timer = new QTimer();
	connect(m_timer, SIGNAL(timeout()), this, SLOT(slot_videoFailed()));
	m_timer->start(10000);
}

// video is ready, we have fetched all data
void MFugaVideochat::slot_showVideo () {
    m_timer->stop();
    showVideo();
}

void MFugaVideochat::slot_videoFailed () {
    cout << "MFugaVideochat: Connection to other host failed!" << endl;
	m_timer->stop();
    m_Fuga->getWindow()->showSelection();
}

// show videochat
void MFugaVideochat::showVideo () {

    // start streaming
    m_Contact->startStreaming();

	// create new central Widget
	delete m_main_window->centralWidget();
	QWidget *central_widget = new QWidget();
	m_main_window->setCentralWidget(central_widget);

	// create label for video
	stringstream webcam_label("");
    webcam_label << "Webcam von " << m_Contact->name();
	QLabel* mylabel = new QLabel(tr(webcam_label.str().c_str()));
	mylabel->setSizePolicy(QSizePolicy(QSizePolicy::Minimum, QSizePolicy::Maximum));

	// create myvideo
    FugaVideo* othercam = m_Contact->Video();
	if (othercam == NULL) {
        showError("Could not start Video!");
        m_Fuga->getWindow()->showSelection();
		return;
	}
	othercam->setStyleSheet("background:#000;");
	othercam->setSizePolicy(QSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding));

	// create chat
	vector<string> partner;
    partner.push_back(m_Contact->name());
    m_Chat = new FugaChat(m_Fuga, m_Fuga->getMe()->name(), partner);
    m_Chat->setStyleSheet("background:#222;");

	// merge in layout
	QGridLayout* layout_grid = new QGridLayout();
	layout_grid->addWidget(mylabel, 1, 0);
	layout_grid->addWidget(othercam, 0, 0);
    layout_grid->addWidget(m_Chat, 0, 2, 1, 2);
	central_widget->setLayout(layout_grid);
	central_widget->setStyleSheet("color:#FFF;background:#000;");

	// start listening
    othercam->start();
}
