#include "Chatbox.h"
#include <iostream>
#include <sstream>

using namespace std;

/* constructor
 */
Chatbox::Chatbox (Supervisor* mysupervisor, string myname, vector<string> mypartners) {
	this->setMaximumWidth(350);
	this->setMinimumWidth(250);

	// save input
	m_supervisor = mysupervisor;
	m_name = myname;
	m_partners = mypartners;

	// build
	m_msgbox = new QTextEdit();
	m_msgbox->setReadOnly(true);
	m_msgbox->setWindowOpacity(0.7);
	m_msgbox->zoomOut(1);
	m_msgbox->setMaximumWidth(350);
	m_msgbox->setMinimumWidth(250);
	m_input = new QLineEdit();
	m_input->setWindowOpacity(0.7);
	m_input->setMaximumHeight(30);
	connect(m_input, SIGNAL(returnPressed()), this, SLOT(sendMsg()));
	connect(m_supervisor->getTcp(), SIGNAL(newMsg(std::string, std::string, std::string)),
			this, SLOT(getNewMsgs(std::string, std::string, std::string)));

	// layout
	QVBoxLayout *my_layout = new QVBoxLayout();
	my_layout->addWidget(m_msgbox, Qt::AlignBottom);
	my_layout->addWidget(m_input, Qt::AlignTop);

	this->setWindowOpacity(0.7);
	this->setLayout(my_layout);
	this->show();
}

// ##################### slots ###################### #

/* send own message
 */
void Chatbox::sendMsg() {
	string msg = m_input->text().toAscii().data();
	if (msg.compare("") == 0) return;

	// send message to all partners
	stringstream out ("");
	out << "m_chat_msg-" << m_name << "|" << msg << ";";
	for (vector<string>::iterator i = m_partners.begin(); i != m_partners.end(); ++i) {
		m_supervisor->getTcp()->sendTo(*i, out.str());
	}

	// add to own msgbox
	addMsg(m_name, msg);

	// clear text
	m_input->setText("");
}

/* get new messages from tcp
 */
void Chatbox::getNewMsgs (string type, string fulltype, string msg) {
	if (type.compare("chat") != 0) return;
	cout << fulltype << endl;

	// split name and message
	vector<string> tmp = split(msg, "|");
	if (tmp.size() != 2) return;

	addMsg(tmp[0], tmp[1]);
}

/* add msg to message-box
 */
void Chatbox::addMsg (string name, string msg) {

	// add msg to msgbox
	stringstream newtext ("");
	newtext << m_msgbox->toPlainText().toAscii().data();
	newtext << name << ":	" << msg << endl;
	m_msgbox->setPlainText(QString(newtext.str().c_str()));
	m_msgbox->moveCursor(QTextCursor::End);
}

/* split string by "delim"
 * @param string: string to split
 * @param char: delimiter
 */
vector<string> Chatbox::split(string s, string delim) {
	vector<string> elems;
	stringstream ss(s);
	string item;
	char delimiter = *delim.data();
	while(std::getline(ss, item, delimiter)) {
		elems.push_back(item);
	}
	return elems;
}
