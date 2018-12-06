#ifndef CLIENTSOCKET_H
#define CLIENTSOCKET_H

#include <QtNetwork/QTcpSocket>
#include <QFile>
#include <QPixmap>
#include <QtSql/QSqlTableModel>
#include <QCursor>
#include <QDir>

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
    bool hideLocalInfo;

    //QPixmap//
    QPixmap pixmap;

    //Keybord and Mouse Control
    int ww;
    int hh;
    QCursor mouse;
    QSqlTableModel *keyid;
    void mousepress(bool LR,int x,int y);
    void mousemove(int x,int y);
    void mouserelease(bool LR,int x,int y);
    void mousedoubleclick();
    void keybord(bool PR,int key,QString text);
    int keytokey(int key);
signals:
    void sendLocalInfo(QString tip, QString msga, QString msgb);
    void messageReceived(QString msg);
    void runThread();
public slots:
    //message//
    void writeMessage(QString type, QString msg);

    //Image//
    void writeimage(QByteArray ba);

    //file//
    void loadinfile(QString filename);
    void updateProgress(qint64 numBytes);

    //getfile//
    void sendLocalDir(QDir dir);
    //read//
    void readClient();
    void receiveMessage();
    void startReceiveFile();
    void receiveFileProcess();
};

#endif // CLIENTSOCKET_H
