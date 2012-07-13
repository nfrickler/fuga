#include "FugaChat.h"
#include "FugaHelperFuncs.h"
#include <iostream>
#include <sstream>
#include <vector>

using namespace std;

// constructor
FugaChat::FugaChat (Fuga* in_Fuga, string myname, vector<string> mypartners) {
	this->setMaximumWidth(350);
	this->setMinimumWidth(250);

	// save input
    m_Fuga = in_Fuga;
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
    connect(m_input, SIGNAL(returnPressed()), this, SLOT(slot_send()), Qt::UniqueConnection);

    // connect partners
    connect(m_Fuga->getContacts(), SIGNAL(sig_received(std::string,std::string,std::vector<std::string>)),
            this, SLOT(slot_receive(std::string,std::string,std::vector<std::string>)), Qt::UniqueConnection);

	// layout
	QVBoxLayout *my_layout = new QVBoxLayout();
	my_layout->addWidget(m_msgbox, Qt::AlignBottom);
	my_layout->addWidget(m_input, Qt::AlignTop);

	this->setWindowOpacity(0.7);
	this->setLayout(my_layout);
}

// ##################### slots ###################### #

// send own message
void FugaChat::slot_send() {
	string msg = m_input->text().toAscii().data();
	if (msg.compare("") == 0) return;

	// send message to all partners
	stringstream out ("");
    out << "m_chat_msg-" << m_name << "," << msg << ";";
	for (vector<string>::iterator i = m_partners.begin(); i != m_partners.end(); ++i) {
        m_Fuga->getContacts()->getContact(*i)->send(out.str());
	}

	// add to own msgbox
	addMsg(m_name, msg);

	// clear text
	m_input->setText("");
}

// get new messages of contacts
void FugaChat::slot_receive (std::string, std::string in_type, std::vector<std::string> in_data) {
    if (in_type.compare("m_chat_msg") != 0) return;
    if (in_data.size() != 2) return;

    addMsg(in_data[0], in_data[1]);
}

// add msg to message-box
void FugaChat::addMsg (string name, string msg) {

    // skip network name if in same network
    vector<string> splitted = split(name,"%");
    if (splitted[1].compare(m_Fuga->getMe()->network()) == 0) {
        name = splitted[0];
    }

	// add msg to msgbox
	stringstream newtext ("");
	newtext << m_msgbox->toPlainText().toAscii().data();
	newtext << name << ":	" << msg << endl;
	m_msgbox->setPlainText(QString(newtext.str().c_str()));
	m_msgbox->moveCursor(QTextCursor::End);
}
