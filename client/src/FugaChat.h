#ifndef FugaChat_H
#define FugaChat_H

#include "Fuga.h"
#include <QWidget>
#include <vector>
#include <QTextEdit>
#include <QLineEdit>

class FugaContact;

struct chatmsg {
	std::string name;
	std::string msg;
};

class FugaChat : public QWidget {
    Q_OBJECT

	public:
        FugaChat(Fuga* in_Fuga, std::string myname, std::vector<std::string> mypartners);

	protected:
        Fuga* m_Fuga;
		QLineEdit* m_input;
		QTextEdit* m_msgbox;
		std::vector<chatmsg> m_history;
		std::vector<std::string> m_partners;
        std::string m_name;

		void addMsg (std::string name, std::string msg);

	protected slots:
        void slot_send();

	public slots:
        void slot_receive (std::string,std::string,std::vector<std::string>);

};

#endif // FugaChat_H
