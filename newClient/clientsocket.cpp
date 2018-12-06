#include "clientsocket.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QDateTime>

ClientSocket::ClientSocket(QObject *parent)
    : QTcpSocket(parent)
{
    type="";
    nextBlockSize = 0;

    //�ļ�����
    recvtotalBytes = 0;
    recvbytesReceived = 0;
    recvfileNameSize = 0;
    getpassfile=false;

    in.setDevice(this);         //in����������豸�����
    in.setVersion(QDataStream::Qt_4_7);

    connect(this, SIGNAL(readyRead()), this, SLOT(readClient()));
    connect(this,SIGNAL(error(QAbstractSocket::SocketError))
            ,this,SLOT(tcperror(QAbstractSocket::SocketError)));
}

void ClientSocket::tcperror(QAbstractSocket::SocketError er)
{
    switch (er) {
    case QAbstractSocket::RemoteHostClosedError:
        showmessagebox(tr("TCP������ʾ"),tr("�������ѹرգ�"));
        break;
    case QAbstractSocket::HostNotFoundError:
        showmessagebox(tr("TCP������ʾ"),tr("�Ҳ���������������IP��ַ�Ͷ˿ںź��ٳ��ԣ�"));
        break;
    case QAbstractSocket::ConnectionRefusedError:
        showmessagebox(tr("TCP������ʾ"),tr("���ӱ��ܾ�����ȷ�Ϸ������������У�"));
        break;
    default:
        showmessagebox(tr("TCP������ʾ"),tr("�����Ĵ�����: %1.").arg(this->errorString()));
    }
    sendLocalInfo("tcperror", "");
}

void ClientSocket::showmessagebox(QString title, QString msg)
{
    switch( QMessageBox::information(0,title,msg,tr("ȷ��"),0))
    {
    case 0:break;
    }

}


//������Ϣ//
void ClientSocket::writeMessage(QString type, QString msg)
{
    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_4_7);
    out << qint64(0) << type << msg;
    out.device()->seek(0);
    out << qint64(block.size() - sizeof(qint64));
    this->write(block);
}

//��������//
void ClientSocket::sendRequest(QString type,QString msga,QString msgb)
{
    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_4_7);
    out << qint64(0) << type << msga << msgb;

    out.device()->seek(0);//ָ��out�Ŀ�ͷ

    out << qint64(block.size() - sizeof(qint64));//���ǵ�һ��ֵ(��һ��ֵΪ�������ݶεĴ�С)
    this->write(block);
}

//�����ļ�//
void ClientSocket::loadinfile(QString path)
{
    sendcurrentFileName=path.right(path.size() - path.lastIndexOf('/')-1);

    //�ļ�����//
    sendloadSize = 4*1024;
    sendtotalBytes = 0;
    sendbytesWritten = 0;
    sendbytesToWrite = 0;

    connect(this,SIGNAL(bytesWritten(qint64)),this,SLOT(updateProgress(qint64)));
    sendlocalFile = new QFile(path);
    if(!sendlocalFile->open(QFile::ReadOnly))
    {
        sendLocalInfo("fileerror","openfileError");
        return;
    }

    sendLocalInfo("begin",sendcurrentFileName);

    //�ļ��ܴ�С
    sendtotalBytes = sendlocalFile->size();

    QString type="file";
    QDataStream sendOut(&sendoutBlock,QIODevice::WriteOnly);
    sendOut.setVersion(QDataStream::Qt_4_6);
    sendOut << qint64(0) << type << qint64(0) << qint64(0) << sendcurrentFileName;
    sendtotalBytes += sendoutBlock.size();
    sendOut.device()->seek(0);
    sendOut<<qint64(sendoutBlock.size() - sizeof(qint64)) << type
            << sendtotalBytes <<qint64((sendoutBlock.size() - sizeof(qint64)*2));
    sendbytesToWrite = sendtotalBytes - this->write(sendoutBlock);
    sendoutBlock.resize(0);
}

void ClientSocket::updateProgress(qint64 numBytes)//���½�����
{
    sendbytesWritten += (int)numBytes;

    if(sendbytesToWrite > 0) //����Ѿ�����������
    {
        sendoutBlock = sendlocalFile->read(qMin(sendbytesToWrite,sendloadSize));
        sendbytesToWrite -= (int)this->write(sendoutBlock);
        sendoutBlock.resize(0);
    }
    else
    {
        disconnect(this,SIGNAL(bytesWritten(qint64)),this,SLOT(updateProgress(qint64)));
        sendlocalFile->close(); //���û�з����κ����ݣ���ر��ļ�
    }
    //���½�����
    QString pm; pm.setNum(sendtotalBytes);
    QString pv; pv.setNum(sendbytesWritten);
    sendLocalInfo("progressm",pm);
    sendLocalInfo("progressv",pv);

    if(sendbytesWritten == sendtotalBytes) //�������
    {
        sendtotalBytes = 0;
        sendbytesWritten = 0;
        sendbytesToWrite = 0;

        sendLocalInfo("sendfiledone",sendcurrentFileName);

        disconnect(this,SIGNAL(bytesWritten(qint64)),this,SLOT(updateProgress(qint64)));
        sendlocalFile->close();
    }
}


void ClientSocket::readClient()
{

    if(!getpassfile)
    {
        //foreverѭ�����б�Ҫ��ֱ�����㹻����Ϣ����ʱ���ܱ�֤��������
        forever
        {
            if (nextBlockSize == 0)
            {
                if (this->bytesAvailable() < sizeof(qint64))
                {
                    break;
                }
                in >> nextBlockSize;//��ȡ�ֶδ�С                
            }

            if (this->bytesAvailable() < nextBlockSize)   //���foreverѭ�����ȴ����ݵ�����
                break;

            in >> type;

            if(type=="msg")
            {
                 receiveMessage();
            }

            else if(type=="file")//�����ļ�
            {
                startReceiveFile();
            }

            else if(type=="img")
            {
                in >> pixmap;
                emit imageReceived(pixmap);
            }

            else if(type=="update")
            {
                QString newline;
                in>>newline;
                sendLocalInfo(type, newline);
            }
            else if(type=="dir")
            {         
                receiveDir();
            }

            else
            {
                sendLocalInfo("dataerror", "");
            }

            nextBlockSize = 0;
        }
    }
    else
    {
        receiveFileProcess();
    }
}

//��Ϣ����
void ClientSocket::receiveMessage()
{
    QString msg;
    in >> msg;
    nextBlockSize = 0;
    emit messageReceived(msg);
}

//�ļ�����
void ClientSocket::startReceiveFile()
{
    getpassfile=true;
    if(recvbytesReceived <= sizeof(qint64)*2)
    {
        if((bytesAvailable() >= sizeof(qint64)*2)&&(recvfileNameSize == 0))
        {
            in >> recvtotalBytes >> recvfileNameSize;
            recvbytesReceived += sizeof(qint64) * 2;

            in >> recvfileName;

            if(QFile::exists(recvfileName))
            {
                recvfileName="["+QDateTime::currentDateTime().toString("yyyyMMddhhmmss")+"] "+recvfileName;
            }

            recvbytesReceived += recvfileNameSize;
            recvlocalFile = new QFile(recvfileName);
            if(!recvlocalFile->open(QFile::WriteOnly))
            {
                sendLocalInfo("fileerror","openfileError");
                return;
            }
            sendLocalInfo("begin",recvfileName);
        }
        else return;
    }
    if(recvbytesReceived < recvtotalBytes)
    {
        recvbytesReceived += bytesAvailable();
        recvinBlock = readAll();
        recvlocalFile->write(recvinBlock);
        recvinBlock.resize(0);
    }
    //���½�����
    QString pm; pm.setNum(recvtotalBytes);
    QString pv; pv.setNum(recvbytesReceived);
    sendLocalInfo("progressm",pm);
    sendLocalInfo("progressv",pv);

    if(recvbytesReceived == recvtotalBytes) //�����������
    {
        recvlocalFile->close();
        sendLocalInfo("receivefiledone",recvfileName);
        recvtotalBytes = 0;
        recvbytesReceived = 0;
        recvfileNameSize = 0;
        getpassfile=false;
    }
}

void ClientSocket::receiveFileProcess()
{
    if(recvbytesReceived < recvtotalBytes)
    {
        recvbytesReceived += bytesAvailable();
        recvinBlock = readAll();
        recvlocalFile->write(recvinBlock);
        recvinBlock.resize(0);
    }
    //���½�����
    QString pm; pm.setNum(recvtotalBytes);
    QString pv; pv.setNum(recvbytesReceived);
    sendLocalInfo("progressm",pm);
    sendLocalInfo("progressv",pv);

    if(recvbytesReceived == recvtotalBytes)
    {
        recvlocalFile->close();
        sendLocalInfo("receivefiledone",recvfileName);
        recvtotalBytes = 0;
        recvbytesReceived = 0;
        recvfileNameSize = 0;
        getpassfile=false;
    }

}

bool ClientSocket::isSendingFile()
{
    if (sendbytesToWrite != 0)
        return true;
    else
        return false;
}

void ClientSocket::receiveDir()
{
    QList<myFileInfo> list;
    myFileInfo info;

    int size;
    in >> size;

    for (int i=0; i<size; i++)
    {
       in >> info.fileName >> info.fileSize >> info.fileDate >> info.fileAbsolutePath >> info.isDir;
       list.append(info);
    }

    emit dirReceived(list);
}
