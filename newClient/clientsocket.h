#ifndef CLIENTSOCKET_H
#define CLIENTSOCKET_H

#include <QtNetwork/QTcpSocket>
#include <QFile>
#include <QPixmap>
#include "myFileInfo.h"

class ClientSocket : public QTcpSocket
{
    Q_OBJECT

public:
    ClientSocket(QObject *parent = 0);

private:
    qint64 nextBlockSize;
    QString type;

    QDataStream in;
    //�����ļ�//
    qint64 recvtotalBytes;  //����ܴ�С��Ϣ
    qint64 recvbytesReceived;  //���յ����ݵĴ�С
    qint64 recvfileNameSize;  //�ļ����Ĵ�С��Ϣ
    QString recvfileName;   //����ļ���
    QFile *recvlocalFile;   //�����ļ�
    QByteArray recvinBlock;   //���ݻ�����
    bool getpassfile;

    //�����ļ�//
    QFile *sendlocalFile;
    QString sendcurrentFileName;
    qint64 sendtotalBytes;//�����ܴ�С
    qint64 sendbytesWritten;//�Ѿ��������ݴ�С
    qint64 sendbytesToWrite;//ʣ�����ݴ�С
    qint64 sendloadSize;//ÿ�η������ݵĴ�С
    QByteArray sendoutBlock;  //���ݻ������������ÿ��Ҫ���͵�����

    QPixmap pixmap;
signals:
    void sendLocalInfo(QString tip, QString msg);
    void messageReceived(QString msg);
    void imageReceived(QPixmap pixmap);
    void dirReceived(QList<myFileInfo> list);
public slots:
    void writeMessage(QString type, QString msg);
    void sendRequest(QString type,QString msga,QString msgb);
    void loadinfile(QString path);
    void updateProgress(qint64 numBytes);

    void readClient();
    void receiveMessage();
    void startReceiveFile();
    void receiveFileProcess();
    void receiveDir();

    void tcperror(QAbstractSocket::SocketError er);
    void showmessagebox(QString title,QString msg);
    bool isSendingFile();
};

#endif // CLIENTSOCKET_H
