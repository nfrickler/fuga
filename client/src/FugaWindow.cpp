#include "FugaWindow.h"

using namespace std;

// constructor
FugaWindow::FugaWindow(Fuga* in_Fuga) {

    // fill object vars
    m_Fuga = in_Fuga;
    m_second_window = NULL;

	// settings
	this->setAttribute(Qt::WA_NativeWindow,true);
	setWindowTitle("Hello World!");
	resize(1000, 600);

	// drawMenu
	drawMenu();

	this->move(QApplication::desktop()->screen()->rect().center()-this->rect().center());
}

// draw menu
void FugaWindow::drawMenu () {
	QMenuBar *menuBar1 = new QMenuBar(0);

	// file menu
	QMenu* fileMenu = menuBar()->addMenu(tr("&Datei"));
	menuBar1->addMenu(fileMenu);

    // back to selection
	QAction* gotoSelector = new QAction(tr("&Zur Auswahl"),this);
	gotoSelector->setShortcut(tr("CTRL+Z"));
    connect(gotoSelector, SIGNAL(triggered()), m_Fuga, SLOT(slot_mode_select()), Qt::UniqueConnection);
	fileMenu->addAction(gotoSelector);

    // back to login
    QAction* gotoLogin = new QAction(tr("&Ausloggen"),this);
    gotoLogin->setShortcut(tr("CTRL+D"));
    connect(gotoLogin, SIGNAL(triggered()), m_Fuga, SLOT(slot_mode_login()), Qt::UniqueConnection);
    fileMenu->addAction(gotoLogin);

	// exit
	QAction* exit = new QAction(tr("&Beenden"),this);
	exit->setShortcuts(QKeySequence::Close);
	exit->setStatusTip(tr("Jetzt beenden"));
    connect(exit, SIGNAL(triggered()), this, SLOT(close()), Qt::UniqueConnection);
	fileMenu->addAction(exit);

	// help menu
	QMenu* helpMenu = menuBar()->addMenu(tr("&Hilfe"));
	menuBar1->addMenu(helpMenu);

	// homepage
	QAction* webpage = new QAction(tr("&Homepage"),this);
	webpage->setShortcut(tr("CTRL+H"));
	helpMenu->addAction(webpage);
}

// show login window
void FugaWindow::showLogin () {
    clearWindows();
    cout << "FugaWindow: Draw login" << endl;

    QLabel *image = new QLabel();
    image->setPixmap( QPixmap( "../images/bg01.jpg" ) );

    // create login elements
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

    connect(form_submit, SIGNAL(clicked()), this, SLOT(slot_clicked_login()), Qt::UniqueConnection);
    connect(this, SIGNAL(sig_login(std::string,std::string)),
            m_Fuga, SLOT(slot_login(std::string,std::string)), Qt::UniqueConnection);
    connect(form_cancel, SIGNAL(clicked()), qApp, SLOT(quit()), Qt::UniqueConnection);

    // combine login-elements in layout
    QGridLayout *mainLayout = new QGridLayout;
    mainLayout->addWidget(name_label, 0, 0, Qt::AlignTop);
    mainLayout->addWidget(m_login_name, 0, 1);
    mainLayout->addWidget(pass_label, 1, 0);
    mainLayout->addWidget(m_login_password, 1, 1);
    mainLayout->addWidget(form_submit, 2, 0);
    mainLayout->addWidget(form_cancel, 2, 1);
    mainLayout->addWidget(image, 3, 0);

    // create widget for layout
    QWidget *login_widget = new QWidget();
    login_widget->setLayout(mainLayout);

    // create new window and add central element
    m_second_window = new QMainWindow();
    m_second_window->setWindowTitle("Login");
    m_second_window->resize(320, 220);
    m_second_window->setCentralWidget(login_widget);
    m_second_window->move(QApplication::desktop()->screen()->rect().center()-m_second_window->rect().center());
    m_second_window->show();
}

// slot: clicked login
void FugaWindow::slot_clicked_login() {
    cout << "FugaWindow: Someone clicked login!" << endl;

    // get input of user
    string name = m_login_name->text().toStdString();
    string password = m_login_password->text().toStdString();
    delete m_login_name;
    delete m_login_password;

    // emit login signal
    emit sig_login(name, password);
}

// show selection window
void FugaWindow::showSelection () {
    clearWindows();

    // init selection
    QLabel *selector_text = new QLabel(QObject::tr("Was möchtest du machen?"));
    QGridLayout *mainLayout = new QGridLayout();
    mainLayout->addWidget(selector_text, 0, 0, 1, 3, Qt::AlignBottom);

    // add selection handler
    m_selection_signalmapper = new QSignalMapper(this);
    connect(m_selection_signalmapper, SIGNAL(mapped(QString)),
            m_Fuga, SLOT(slot_startModule(QString)),Qt::UniqueConnection);

    // add all modules for selection
    QLabel *current_label = new QLabel(tr("Starte einen Videochat"));
    QPushButton *but_videochat = new QPushButton(tr("Videochat"));

    // add signals
    m_selection_signalmapper->setMapping(but_videochat, QString("Videochat"));
    connect(but_videochat, SIGNAL(clicked()), m_selection_signalmapper, SLOT (map()));

    // add to layout
    mainLayout->addWidget(but_videochat, 1, 1, Qt::AlignTop);
    mainLayout->addWidget(current_label, 1, 2, Qt::AlignTop);

    // show main window
    QWidget *mainWidget = new QWidget();
    setCentralWidget(mainWidget);
    centralWidget()->setLayout(mainLayout);
    show();
    activateWindow();
}

// clear all windows
void FugaWindow::clearWindows() {
    if (centralWidget()) delete centralWidget();
    if (m_second_window != NULL) {
        delete m_second_window;
        m_second_window = NULL;
    }
    //hide();
}
