#include "MFugaVideochat.h"
#include "FugaHelperFuncs.h"
#include "FugaChat.h"
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

// destructor
MFugaVideochat::~MFugaVideochat() {
    if (!m_partner.empty()) doDisconnect();
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
    m_partner = m_name_input->text().toStdString();
    FugaContact* Partner = m_Fuga->getContacts()->getContact(m_partner);

    // user exists?
    if (Partner->isAccepted() && Partner->isFetched()) {
        showVideo();
        return;
    }

    // wait for videoReady
    connect(m_Fuga->getContacts(), SIGNAL(sig_connected(std::string)),
            this, SLOT(slot_showVideo(std::string)));
    connect(m_Fuga->getContacts(), SIGNAL(sig_disconnected(std::string)),
            this, SLOT(slot_disconnected(std::string)));
    m_timer = new QTimer();
	connect(m_timer, SIGNAL(timeout()), this, SLOT(slot_videoFailed()));
	m_timer->start(10000);
}

// video is ready, we have fetched all data
void MFugaVideochat::slot_showVideo (std::string in_name) {
    if (in_name != m_partner) return;
    if (m_timer) m_timer->stop();
    showVideo();
}

void MFugaVideochat::slot_videoFailed () {
    cout << "MFugaVideochat: Connection to other host failed!" << endl;
	m_timer->stop();
    m_Fuga->getWindow()->showSelection();
}

// show videochat
void MFugaVideochat::showVideo () {
    cout << "MFugaVideochat: Contact ready. Start!" << endl;

    FugaContact* Partner = m_Fuga->getContacts()->getContact(m_partner);

    // start streaming
    Partner->startStreaming();

	// create new central Widget
	delete m_main_window->centralWidget();
	QWidget *central_widget = new QWidget();
	m_main_window->setCentralWidget(central_widget);

	// create label for video
	stringstream webcam_label("");
    webcam_label << "Webcam von " << Partner->name();
	QLabel* mylabel = new QLabel(tr(webcam_label.str().c_str()));
	mylabel->setSizePolicy(QSizePolicy(QSizePolicy::Minimum, QSizePolicy::Maximum));

	// create myvideo
    FugaVideo* othercam = Partner->Video();
	if (othercam == NULL) {
        showError("Could not start Video!");
        doDisconnect();
        m_Fuga->slot_mode_select();
		return;
	}
	othercam->setStyleSheet("background:#000;");
	othercam->setSizePolicy(QSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding));

	// create chat
    vector<string> partner;
    partner.push_back(Partner->name());
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

// the partner disconnected
void MFugaVideochat::slot_disconnected(string in_name) {
    if (in_name != m_partner) return;
    showError("Der Chatpartner hat den Chat verlassen.");
    doDisconnect();
    m_Fuga->slot_mode_select();
}

// do disconnect
void MFugaVideochat::doDisconnect() {
    FugaContact* Partner = m_Fuga->getContacts()->getContact(m_partner);
    disconnect(m_Fuga->getContacts(), SIGNAL(sig_disconnected(std::string)),
               this, SLOT(slot_disconnected(std::string)));
    Partner->doDisconnect();
    m_partner = "";
    if (m_Chat) delete m_Chat;
}

