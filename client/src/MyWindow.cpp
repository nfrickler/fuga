#include "MyWindow.h"
#include <QtGui>
#include <iostream>

using namespace std;

/* constructor
 */
MyWindow::MyWindow(Supervisor* mysupervisor) {

	// save input
	m_supervisor = mysupervisor;

	// settings
	this->setAttribute(Qt::WA_NativeWindow,true);
	setWindowTitle("Hello World!");
	resize(1000, 600);

	// drawMenu
	drawMenu();

	this->move(QApplication::desktop()->screen()->rect().center()-this->rect().center());
}

/* draw menu
 */
void MyWindow::drawMenu () {
	QMenuBar *menuBar1 = new QMenuBar(0);

	// file menu
	QMenu* fileMenu = menuBar()->addMenu(tr("&Datei"));
	menuBar1->addMenu(fileMenu);

	// zur Auswahl
	QAction* gotoSelector = new QAction(tr("&Zur Auswahl"),this);
	gotoSelector->setShortcut(tr("CTRL+Z"));
	connect(gotoSelector, SIGNAL(triggered()), m_supervisor, SLOT(slot_showSelector()));
	fileMenu->addAction(gotoSelector);

	// exit
	QAction* exit = new QAction(tr("&Beenden"),this);
	exit->setShortcuts(QKeySequence::Close);
	exit->setStatusTip(tr("Jetzt beenden"));
	connect(exit, SIGNAL(triggered()), this, SLOT(close()));
	fileMenu->addAction(exit);

	// help menu
	QMenu* helpMenu = menuBar()->addMenu(tr("&Hilfe"));
	menuBar1->addMenu(helpMenu);

	// homepage
	QAction* webpage = new QAction(tr("&Homepage"),this);
	webpage->setShortcut(tr("CTRL+H"));
	helpMenu->addAction(webpage);

}
