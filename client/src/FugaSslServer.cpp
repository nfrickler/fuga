#include "FugaSslServer.h"
#include <QFile>
#include <QSslKey>
#include <QSslCertificate>
#include <iostream>
#include <QSslCipher>

using namespace std;

// constructor
FugaSslServer::FugaSslServer(QObject* parent)
    : QTcpServer(parent)
{

}

// start listening
void FugaSslServer::start(QString in_cert, QString in_key, quint16 in_port) {
    listen(QHostAddress::Any, in_port);
    m_cert = in_cert;
    m_key = in_key;
}

// handle incoming connection
void FugaSslServer::incomingConnection(int in_socketdescriptor) {
    QSslSocket* socket = new QSslSocket();
    socket->ignoreSslErrors();
    if (socket->setSocketDescriptor(in_socketdescriptor)) {

    /* print supported and default ciphers
        cout << "  Supported ciphers:";
        foreach( QSslCipher cipher, QSslSocket::supportedCiphers() )
             cout << cipher.name().toAscii().data();

        cout << "  Default ciphers:";
        foreach( QSslCipher cipher, QSslSocket::defaultCiphers() )
            cout << cipher.name().toAscii().data();
    */

        // read key
        QByteArray key;
        QFile fileKey(m_key.toAscii().data());
        if(fileKey.open(QIODevice::ReadOnly)) {
            key = fileKey.readAll();
            fileKey.close();
        } else {
            qDebug() << fileKey.errorString();
        }
        QSslKey sslKey(key, QSsl::Rsa, QSsl::Pem, QSsl::PrivateKey, QByteArray("bluebell"));

        // read certificate
        QFile certfile(m_cert.toAscii().data());
        certfile.open(QIODevice::ReadOnly);
        QSslCertificate sslCert(&certfile);

        socket->setPrivateKey(sslKey);
        socket->setLocalCertificate(sslCert);

        emit sig_newconnection(socket);
    } else {
        delete socket;
    }
}

// handle ssl errors
void FugaSslServer::slot_sslerror(const QList<QSslError> &) {
    cout << "FugaSslServer: SSL errors" << endl;
    m_socket->ignoreSslErrors();
}
