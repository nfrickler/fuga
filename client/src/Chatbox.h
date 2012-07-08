#ifndef CHATBOX_H
#define CHATBOX_H

#include "Fuga.h"
#include <QWidget>
#include <vector>
#include <QTextEdit>
#include <QLineEdit>

struct chatmsg {
	std::string name;
	std::string msg;
};

class Chatbox : public QWidget {
    Q_OBJECT

	public:
        Chatbox(Fuga* in_Fuga, std::string myname, std::vector<std::string> mypartners);

	protected:
        Fuga* m_Fuga;
		QLineEdit* m_input;
		QTextEdit* m_msgbox;
		std::vector<chatmsg> m_history;
		std::vector<std::string> m_partners;
		std::string m_name;

		std::vector<std::string> split(std::string s, std::string delim);
		void addMsg (std::string name, std::string msg);

	protected slots:
		void sendMsg();

	public slots:
		void getNewMsgs (std::string type, std::string fulltype, std::string msg);

};

#endif // CHATBOX_H
