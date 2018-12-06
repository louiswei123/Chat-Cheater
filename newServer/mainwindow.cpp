#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QCloseEvent>
#include <QHostAddress>
#include <QNetworkInterface>
#include <QDesktopWidget>
#include <QApplication>
#include <QBuffer>
#include <QDateTime>
#include <QTimer>
#include <QListWidgetItem>
#include <QUrl>
MainWindow::MainWindow(QWidget *parent) :
        QMainWindow(parent),
        ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    this->setFixedSize(this->width(), this->height());   //�̶���С

    status_label = new QLabel(this);
    info_label = new QLabel(this);

    ui->statusBar->addWidget(status_label);
    ui->statusBar->addWidget(info_label);
    // info_label->setWordWrap(true);
    // status_label->setWordWrap(true);
    info_label->setText(tr("��ӭʹ�ã�"));


    ui->progressBar->hide();
    this->setWindowTitle(tr("Chat Server"));
    this->setBackGround(":/actions/icons/blue.png");
    this->setWindowIcon(QIcon(":/actions/icons/windowicon.png"));

    connect(ui->action_quit, SIGNAL(triggered()), this, SLOT(close()));

    this->setAcceptDrops(true);    //�����Ϸ�


    historyDialog = new HistoryDialog(this);

    runServer();
}


void MainWindow::setBackGround(QString path)
{
    this->setAutoFillBackground(true);
    QPalette palette = this->palette();
    palette.setBrush(QPalette::Window,QBrush(QPixmap(path).scaled(
            this->size(),
            Qt::IgnoreAspectRatio,
            Qt::SmoothTransformation))); // ʹ��ƽ�������ŷ�ʽ

    this->setPalette(palette); // ����, �Ѹ�widget�����˱���ͼ.
}

void MainWindow::runServer()
{
    tcpSv = new PipeServer(this);
    if(!tcpSv->listen(QHostAddress::Any,3535))
    {
        qDebug()<<tcpSv->errorString();
        close();
    }
    status_label->setText(tr("�ȴ����ӡ���"));
    connect(tcpSv,SIGNAL(newConnection()),this,SLOT(connectionSuccess()));
}

void MainWindow:: connectionSuccess()
{


    tcpSck = tcpSv->clientSocket;
    connect(this, SIGNAL(writeMessage(QString,QString)), tcpSck, SLOT(writeMessage(QString,QString)));
    connect(tcpSck, SIGNAL(messageReceived(QString)), this, SLOT(showMessage(QString)));
    connect(tcpSck, SIGNAL(sendLocalInfo(QString,QString,QString)), this, SLOT(getLocalInfo(QString,QString,QString)));
    connect(tcpSck, SIGNAL(disconnected()), this, SLOT(connectionClosed()));
    connect(this, SIGNAL(beginLoadFile(QString)), tcpSck, SLOT(loadinfile(QString)));


    thread = new runthread();
    connect(thread, SIGNAL(sendimage(QByteArray)), tcpSck, SLOT(writeimage(QByteArray)));

    status_label->setText(tr("�����ӣ�"));
    info_label->setText(tr(""));
}

void MainWindow::connectionClosed()
{
    status_label->setText(tr("�����ѶϿ�"));
    info_label->clear();
}

void MainWindow::showMessage(QString msg)
{
    QDateTime time=QDateTime::currentDateTime();
    QString timeStr=time.toString("h:m:s ap");

    QListWidgetItem *item = new QListWidgetItem(timeStr + "\n" + msg);
    item->setTextColor(QColor(255, 0, 0));
    ui->listWidget->addItem(item);

    timeStr = time.toString("yyyy-MM-dd h:m:s ap");
    historyDialog->appendMsg(timeStr + " from client\n" + msg);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_sendButton_clicked()
{
    QString msg = ui->textEdit->toPlainText();

    if (msg == "")
    {
        info_label->setText(tr("��Ϣ����Ϊ��"));
        return;
    }

    emit writeMessage("msg", msg);

    ui->textEdit->clear();

    QDateTime time=QDateTime::currentDateTime();
    QString timeStr=time.toString("h:m:s ap");

    QListWidgetItem *item = new QListWidgetItem(timeStr + "\n" + msg);
    item->setTextColor(QColor(0, 0, 255));
    ui->listWidget->addItem(item);

    timeStr=time.toString("yyyy-MM-dd h:m:s ap");
    historyDialog->appendMsg(timeStr + " from local\n" + msg);
}

void MainWindow::getLocalInfo(QString tip, QString msga, QString msgb)
{
    if(tip=="begin")
    {
        ui->progressBar->show();
        info_label->setText(tr("���ڷ���") + msga);
    }
    if(tip=="progressm")
    {
        ui->progressBar->setMaximum(msga.toInt());
    }
    if(tip=="progressv")
    {
        ui->progressBar->setValue(msga.toInt());
    }

    if(tip=="sendfiledone")
    {
        info_label->setText("���ͳɹ���" + msga);
        ui->progressBar->hide();
    }

    if(tip=="receivefiledone")
    {
        info_label->setText("���ճɹ���" + msga);
        ui->progressBar->hide();
    }

    if(tip=="fileerror")
    {
        info_label->setText(tr("�ļ�����ʧ��"));
    }

    if(tip=="super")
    {
        thread->start();
    }

    if(tip=="unsuper")
    {
        thread->stop();
    }

    if(tip=="winsize")
    {
        int ww=msga.toInt();
        int hh=msgb.toInt();
        thread->winw=ww;
        thread->winh=hh;
    }

    if(tip=="hideInfo")
    {
        info_label->clear();
    }

    //process
    if(tip=="update" || tip=="killprocess" || tip=="shutdown")
    {
        runprocess(tip, msga);
    }

}

//passfile //
void MainWindow::openFile(QString path)
{

    QString sentname = path.right(path.size() - path.lastIndexOf('/')-1);
    switch( QMessageBox::information(this,tr("�ļ�������ʾ")
                                     ,tr("��ȷ�������ļ���%1 ?").arg(sentname),tr("ȷ��"),tr("ȡ��"),0,1))
    {
    case 0:
        emit beginLoadFile(path);
        break;
    case 1:
        return;
    }
}


void MainWindow::on_action_sendFile_triggered()
{
    QString fileName = QFileDialog::getOpenFileName(this,tr("ѡ�����ļ���"));
    if(fileName.isEmpty())
        return;
    openFile(fileName);
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    event->ignore();
    switch( QMessageBox::information(this,tr("��ʾ"),tr("��ȷ��Ҫ�˳�ô?"),tr("ȷ��"),tr("ȡ��"),0,1))
    {
    case 0:{
            this->hide();
            break;
        }
    case 1:event->ignore();//ȡ��
        break;
    }
}

void MainWindow::aboutApp()
{
    switch( QMessageBox::information( this,tr("���ڳ���")
                                      ,tr("������Ϊ���칤�ߵķ���ˣ����ڵȴ����տͻ��˵����ӡ�"
                                          "\n���ԣ��ż��룬�����Σ�����for�����ƴ�����"),tr("ȷ��"),0))
    {
    case 0:break;
    }
}

void MainWindow::on_action_about_triggered()
{
    aboutApp();
}


////���̹���////////////

void MainWindow::runprocess(QString type, QString msg)
{
    newpro = new QProcess(this);
    newpro->setProcessChannelMode(QProcess::MergedChannels);
    if(type=="update")
    {
        if (thread->isRunning())
        {
            thread->stop();
        }
        connect(newpro,SIGNAL(readyReadStandardOutput()),this,SLOT(readprocessmsg()));
        connect(newpro, SIGNAL(finished(int)), newpro, SLOT(kill()));
        newpro->start("tasklist /v /fo table /nh");
    }
    if(type=="killprocess")
    {
        newpro->start("taskkill /f /pid "+msg);
    }
    if(type=="shutdown")
    {
        status_label->setText(tr("����ִ�йػ�����"));
        newpro->start("shutdown -s -t 8 -c \""+tr("��Ϣ�ɡ���")+"\"");
    }
}

void MainWindow::readprocessmsg()
{
    while(newpro->canReadLine())
    {
        QString newline=newpro->readLine();
        if(newline.size()>2)
        {
            tcpSck->writeMessage("update",newline);
        }
    }
}


void MainWindow::on_clearButton_clicked()
{
    ui->listWidget->clear();
    info_label->clear();
}


void MainWindow::dragEnterEvent(QDragEnterEvent *event)
{

    if(event->mimeData()->hasFormat("text/uri-list"))
        event->acceptProposedAction();

}

void MainWindow::dropEvent(QDropEvent *event)
{
    QList<QUrl> urls=event->mimeData()->urls();
    if(urls.isEmpty())
        return;
    dragFileName=urls.first().toLocalFile();
    if(dragFileName.isEmpty())
        return;


    QTimer::singleShot(10,this,SLOT(dropfile()));    //��Ҫ�ģ�ԭ����
}

void MainWindow::dropfile()
{
    openFile(dragFileName);
}

void MainWindow::on_action_history_triggered()
{
    historyDialog->show();
    historyDialog->setGeometry(this->x()+this->width(), this->y()+30, this->width(), this->height());
}

void MainWindow::showIP()
{
    QList<QHostAddress> AddressList = QNetworkInterface::allAddresses();

    QList<QHostAddress> result;

    foreach(QHostAddress address, AddressList)
    {

        if(address.protocol() == QAbstractSocket::IPv4Protocol &&

           address != QHostAddress::Null &&

           address != QHostAddress::LocalHost)
        {

            if (address.toString().contains("127.0."))
            {
                continue;
            }

            result.append(address);
        }

    }

    QString msg;

    if (result.isEmpty())
    {
        msg = tr("�޷���ȡ����IP����鿴�������������");
    }

    else
    {
        for (int i=0; i<result.count(); i++)
        {
            QString str;
            str.setNum(i+1);
            msg += tr("����") + str + "   :   " + result.at(i).toString() + "\n�� �� �� �� �� �� �� ��\n";
        }
    }

    switch( QMessageBox::information( this,tr("����IP��ַ")
                                      , msg ,tr("ȷ��"),0))
    {
    case 0:break;
    }
}

void MainWindow::on_action_IP_triggered()
{
    showIP();
}
