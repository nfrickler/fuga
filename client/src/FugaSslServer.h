#ifndef FUGASSLSERVER_H
#define FUGASSLSERVER_H

#include <QTcpServer>
#include <QSslSocket>

class FugaSslServer : public QTcpServer {
    Q_OBJECT

    public:
        FugaSslServer(QObject* parent = NULL);
        void start(QString in_cert, QString in_key, quint16 in_port);

    protected:
        void incomingConnection(int in_socketdescriptor);

    signals:
        void sig_newconnection(QSslSocket* in_socket);

    protected slots:
        void slot_sslerror(const QList<QSslError> &);

    private:
        QString m_cert;
        QString m_key;
        QSslSocket* m_socket;
};

#endif // FUGASSLSERVER_H
