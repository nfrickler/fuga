#include "MeUser.h"
#include "MyUser.h"
#include <iostream>

using namespace std;

/* constructor
 */
MeUser::MeUser (Supervisor* mysupervisor) {
	// : MyUser(mysupervisor, "") {

	// save input
	m_supervisor = mysupervisor;
	m_name = "";
	m_second_window = NULL;

}

// # ################ login ################################ #

/* drawLogin-slot
 */
void MeUser::slotDrawLogin () {
	drawLogin();
}

/* login
 */
void MeUser::drawLogin () {

	// delete centralWidget
	delete m_supervisor->getWindow()->centralWidget();

	QLabel *image = new QLabel();
	image->setPixmap( QPixmap( "../images/bg01.jpg" ) );

	// create login-elements
	QLabel *name_label = new QLabel(("Dein Loginname"));
	m_login_name = new QLineEdit();
	m_login_name->setText(tr("Testuser1"));
	QLabel *pass_label = new QLabel(("Dein Passwort"));
	m_login_password = new QLineEdit();
	m_login_password->setInputMethodHints(Qt::ImhHiddenText| Qt::ImhNoPredictiveText|Qt::ImhNoAutoUppercase);
	m_login_password->setEchoMode(QLineEdit::Password);
	m_login_password->setText(tr("passpass"));
	QPushButton *form_submit = new QPushButton(("Einloggen"));
	QPushButton *form_cancel = new QPushButton(("Abbrechen"));
	QPushButton *form_regist = new QPushButton(("Registrieren"));
	connect(form_submit, SIGNAL(clicked()), this, SLOT(slotLogin()));
	connect(form_cancel, SIGNAL(clicked()), qApp, SLOT(quit()));
	connect(form_regist, SIGNAL(clicked()), this, SLOT(slotDrawRegist()));

	// combine login-elements in layout
	QGridLayout *mainLayout = new QGridLayout;
	mainLayout->addWidget(name_label, 0, 0, Qt::AlignTop);
	mainLayout->addWidget(m_login_name, 0, 1);
	mainLayout->addWidget(pass_label, 1, 0);
	mainLayout->addWidget(m_login_password, 1, 1);
	mainLayout->addWidget(form_submit, 2, 0);
	mainLayout->addWidget(form_cancel, 2, 1);
	mainLayout->addWidget(form_regist, 3, 1);
	mainLayout->addWidget(image, 3, 0);

	// create widget for layout
	QWidget *login_widget = new QWidget();
	login_widget->setLayout(mainLayout);

	// create new window and add central element
	if (m_second_window == NULL) m_second_window = new QMainWindow();
	m_second_window->setWindowTitle("Login");
	m_second_window->resize(320, 220);
	m_second_window->setCentralWidget(login_widget);
	m_second_window->move(QApplication::desktop()->screen()->rect().center()-m_second_window->rect().center());
	m_second_window->show();
}

/* login-slot
 */
void MeUser::slotLogin () {

	// validate input
	m_name = m_login_name->text().toStdString();
	string password = m_login_password->text().toStdString();
	cout << "Your name is " << m_name << std::endl;

	// login
	m_supervisor->getTcp()->doLogin(m_name, password);
}

/* finish login
 */
void MeUser::finishLogin () {
	cout << "Welcome!" << endl;

	// delete login-window
	delete m_second_window;

	// show selector
	m_supervisor->showSelector();
}

/* fail login
 */
void MeUser::failLogin () {
	cout << "Login failed!" << endl;

	// print fail-message
	delete m_second_window->centralWidget();
	QWidget* fail_message = new QWidget();
	QLabel* mylabel = new QLabel("Login ist fehlgeschlagen!");
	QPushButton* form_submit = new QPushButton(("Erneut versuchen"));
	connect(form_submit, SIGNAL(clicked()), this, SLOT(slotDrawLogin()));

	QGridLayout* mainLayout = new QGridLayout;
	mainLayout->addWidget(mylabel, 0, 0, Qt::AlignTop);
	mainLayout->addWidget(form_submit, 1, 0, Qt::AlignTop);
	fail_message->setLayout(mainLayout);

	// show message
	m_second_window->setCentralWidget(fail_message);
	m_second_window->show();
}


// # ################ registration ################################ #

/* registrieren-slot
 */
void MeUser::slotDrawRegist () {
		cout << "Registrieren..." << endl;

		delete m_second_window->centralWidget();

		// create regist-elements
		QLabel* name_label = new QLabel("Loginname");
		m_login_name = new QLineEdit();
		QLabel* pass_label = new QLabel("Password");
		m_login_password = new QLineEdit();
		m_login_password->setInputMethodHints(Qt::ImhHiddenText| Qt::ImhNoPredictiveText|Qt::ImhNoAutoUppercase);
		m_login_password->setEchoMode(QLineEdit::Password);
		QLabel* pass_label2 = new QLabel("Password wiederholen");
		m_login_password2 = new QLineEdit();
		m_login_password2->setInputMethodHints(Qt::ImhHiddenText| Qt::ImhNoPredictiveText|Qt::ImhNoAutoUppercase);
		m_login_password2->setEchoMode(QLineEdit::Password);
		QPushButton *form_submit = new QPushButton(("ok"));
		QPushButton *form_cancel = new QPushButton(("Abbrechen"));
		connect(form_submit, SIGNAL(clicked()), this, SLOT(slotRegist()));
		connect(form_cancel, SIGNAL(clicked()), qApp, SLOT(quit()));

		// combine login-elements in layout
		QGridLayout* mainLayout = new QGridLayout;
		mainLayout->addWidget(name_label, 0, 0);
		mainLayout->addWidget(m_login_name, 0, 1);
		mainLayout->addWidget(pass_label, 1, 0);
		mainLayout->addWidget(m_login_password, 1, 1);
		mainLayout->addWidget(pass_label2, 2, 0);
		mainLayout->addWidget(m_login_password2, 2, 1);
		mainLayout->addWidget(form_submit, 3, 0);
		mainLayout->addWidget(form_cancel, 3, 1);

		// create widget for layout
		QWidget* regist = new QWidget();
		regist->setLayout(mainLayout);

		// create new window and add central element
		m_second_window->setWindowTitle("Registrieren");
		m_second_window->resize(520, 220);
		m_second_window->setCentralWidget(regist);
		m_second_window->show();
}


/* user regestrieren
 */
void MeUser::slotRegist() {
		m_name = m_login_name->text().toStdString();
		m_password = m_login_password->text().toStdString();
		string password2= m_login_password2->text().toStdString();
		if ( m_password.compare(password2) == 0 ) {
			   // delete m_second_window;
				m_supervisor->getTcp()->add_usr(m_name, m_password);
		}
		else {
				cout << "Registrierung failed" << endl;
				failRegist();
		}
}

/* finish regist
 */
void MeUser::finishRegist () {
		cout << "Registrierung ok!" << m_name << m_password << endl;
		m_supervisor->getTcp()->doLogin(m_name, m_password);
}


/* fail regist
 */
void MeUser::failRegist () {
		cout << "Regist failed!" << endl;

		//delete centralWidget
		delete m_supervisor->getWindow()->centralWidget();


		// create regist_failed-elements
		QLabel *msg_label = new QLabel(("Registrierung failed"));
		QLabel *msg_label2 = new QLabel(("Benutzer vergeben oder zu kurz oder"));
		QLabel *msg_label3 = new QLabel(("Password zu kurz"));
		QPushButton *form_submit = new QPushButton(("Erneut versuchen"));
		connect(form_submit, SIGNAL(clicked()), this, SLOT(slotDrawRegist()));

		// combine regist_failed-elements in layout
		QGridLayout *mainLayout = new QGridLayout;
		mainLayout->addWidget(msg_label, 0, 0, Qt::AlignTop);
		mainLayout->addWidget(msg_label2, 1, 0, Qt::AlignTop);
		mainLayout->addWidget(msg_label3, 2, 0, Qt::AlignTop);
		mainLayout->addWidget(form_submit, 3, 0);

		// create widget for layout
		QWidget *login_widget = new QWidget();
		login_widget->setLayout(mainLayout);

		// create new window and add central element
		// if (m_second_window == NULL) m_second_window = new QMainWindow();
	   // m_second_window = new QMainWindow();
		m_second_window->setWindowTitle("Registrieren");
		m_second_window->resize(320, 220);
		m_second_window->setCentralWidget(login_widget);
		m_second_window->show();
}

// # ################ logout ############################## #


// # ################ registration ######################## #


// # ################ edit data ########################### #


/* get my name
 */
string MeUser::getName () {
	return m_name;
}
