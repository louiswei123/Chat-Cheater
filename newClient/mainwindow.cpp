#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QCloseEvent>
#include <QDateTime>
#include <QUrl>
#include <QTimer>
#include <QListWidgetItem>
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

    ui->progressBar->hide();

    ui->superButton->setEnabled(false);
    ui->superButton->setToolTip(tr("���Ƚ�������"));

    this->setWindowTitle(tr("Chat Client"));
    info_label->setText(tr("��ӭʹ�ã�"));
    status_label->setText(tr("δ����"));

    this->setBackGround(tr(":/actions/icons/blue.png"));
    this->setWindowIcon(QIcon(":/actions/icons/windowicon.png"));

    connect(ui->ip_LineEdit, SIGNAL(textEdited(QString)), this, SLOT(checkip(QString)));
    connect(ui->action_quit, SIGNAL(triggered()), this, SLOT(close()));

    tcpSck = new ClientSocket(this);
    connect(tcpSck, SIGNAL(connected()), this, SLOT(connectionSuccess()));
    connect(tcpSck, SIGNAL(disconnected()), this, SLOT(connectionClosed()));
    connect(tcpSck, SIGNAL(sendLocalInfo(QString,QString)), this, SLOT(getLocalInfo(QString,QString)));
    connect(this, SIGNAL(writeMessage(QString,QString)), tcpSck, SLOT(writeMessage(QString,QString)));
    connect(tcpSck, SIGNAL(messageReceived(QString)), this, SLOT(showMessage(QString)));
    connect(this, SIGNAL(beginLoadFile(QString)), tcpSck, SLOT(loadinfile(QString)));

    this->setAcceptDrops(true);    //�����Ϸ�

    historyDialog = new HistoryDialog(this);

    isLogin = false;
}

MainWindow::~MainWindow()
{
    delete ui;
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

void MainWindow::connectionSuccess()
{
    isLogin = true;

    ui->action_connect->setText(tr("�Ͽ�"));
    ui->action_connect->setToolTip(tr("�Ͽ�"));
    ui->action_connect->setIcon(QIcon(":/actions/icons/disconnect.png"));

    ui->superButton->setEnabled(true);
    ui->superButton->setToolTip(tr("���볬��ģʽ"));
    status_label->setText(tr("�����ӣ�"));
    info_label->setText("");
}

void MainWindow::connectionClosed()
{
    isLogin = false;

    ui->action_connect->setText(tr("����"));
    ui->action_connect->setToolTip(tr("����"));
    ui->action_connect->setIcon(QIcon(":/actions/icons/connect.png"));

    ui->superButton->setEnabled(false);
    ui->superButton->setToolTip(tr("���Ƚ�������"));
    status_label->setText(tr("�����ѶϿ���"));

    info_label->clear();

    controlWindow->close();
}

void MainWindow::checkip(QString ip)
{
    if(ip.size()>0)
    {
        QChar c=ip.at(ip.size()-1);
        if( (c>='0'&&c<='9') || (c=='.') )
        {
            info_label->setText(tr(""));
        }
        else
        {
            ui->ip_LineEdit->setText(ip.remove(ip.size()-1,ip.size()));
            info_label->setText(tr("����������������ֻ�."));
        }

        QString partip=ip;
        QString parttwo;
        if(ip.indexOf('.')>0) parttwo=partip.remove(0,partip.lastIndexOf('.')+1);
        else parttwo=partip;

        if(parttwo.toInt()<=255);
        else
        {
            info_label->setText(tr("�������:ÿ�ֶ�ֵ��Ϊ0-255��"));
            ui->ip_LineEdit->setText(ip.remove(ip.size()-1,ip.size()));
        }

        if((ip.lastIndexOf('.')>0&&(ip.size()-ip.lastIndexOf('.'))==4)||(ip.lastIndexOf('.')<0&&ip.size()==3))
        {
            int pointnum=0;
            for(int i=0;i<ip.size();i++)
            {
                if(ip.at(i)=='.')pointnum+=1;
            }
            if(pointnum==3);
            else ui->ip_LineEdit->setText(ip.append("."));
        }
    }
}

void MainWindow::on_sendButton_clicked()
{
    QString msg = ui->textEdit->toPlainText();

    if (msg == "")
    {
        info_label->setText("��Ϣ����Ϊ��");
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

void MainWindow::showMessage(QString msg)
{
    QDateTime time=QDateTime::currentDateTime();
    QString timeStr=time.toString("h:m:s ap");

    QListWidgetItem *item = new QListWidgetItem(timeStr + "\n" + msg);
    item->setTextColor(QColor(255, 0, 0));
    ui->listWidget->addItem(item);

    timeStr = time.toString("yyyy-MM-dd h:m:s ap");
    historyDialog->appendMsg(timeStr + " from server\n" + msg);

}

void MainWindow::getLocalInfo(QString tip, QString msg)
{
    if(tip=="begin")
    {
        ui->progressBar->show();
        info_label->setText(tr("���ڽ���") + msg);
        //ui->progressBar->setFormat("�ļ���"+msg+" �ѽ����ļ�%p%");
    }
    if(tip=="progressm")
    {
        ui->progressBar->setMaximum(msg.toInt());
    }
    if(tip=="progressv")
    {
        ui->progressBar->setValue(msg.toInt());
    }

    if(tip=="sendfiledone")
    {
        info_label->setText("���ͳɹ���" + msg);
        ui->progressBar->hide();
    }

    if(tip=="receivefiledone")
    {
        info_label->setText("���ճɹ���" + msg);
        ui->progressBar->hide();
    }

    if(tip=="fileerror")
    {
        info_label->setText(tr("�ļ���ʧ��"));
    }
    if(tip=="img")
    {
        this->setWindowTitle(this->windowTitle()+msg);
    }
    if(tip=="dataerror")
    {
        info_label->setText(tr("���մ���"));
    }
    if(tip=="tcperror")
    {
        status_label->setText(tr("��ӭʹ��"));
    }
    if(tip=="update")
    {
        controlWindow->insertwidget(msg);
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

void MainWindow::on_action_connect_triggered()
{
    if (isLogin)
    {
        tcpSck->disconnectFromHost();
    }
    else
    {
        QString ip  = ui->ip_LineEdit->text();
        tcpSck->connectToHost(ip, 3535);
        status_label->setText(tr("�������ӡ���"));
    }
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    switch( QMessageBox::information(this,tr("��ʾ"),tr("��ȷ��Ҫ�˳�ô?"),tr("ȷ��"),tr("ȡ��"),0,1))
    {
    case 0:{
            event->accept();
            break;
        }
    case 1:event->ignore();//ȡ��
        break;
    }
}

//Super Mode
void MainWindow::GotoSuperMode()
{
    controlWindow = new ControlWindow(this);
    connect(tcpSck, SIGNAL(imageReceived(QPixmap)), controlWindow, SLOT(imageReceived(QPixmap)));
    connect(tcpSck, SIGNAL(dirReceived(QList<myFileInfo>)), controlWindow, SLOT(dirReceived(QList<myFileInfo>)));
    connect(controlWindow, SIGNAL(sendRequest(QString,QString,QString)),
            tcpSck, SLOT(sendRequest(QString,QString,QString)));
    connect(controlWindow, SIGNAL(controlWindowClosed()), this, SLOT(controlWindowClosed()));

    controlWindow->show();
    controlWindow->stopImage(false);
    ui->action_sendFile->setEnabled(false);
    ui->superButton->setEnabled(false);
}

void MainWindow::on_superButton_clicked()
{
    GotoSuperMode();
}


void MainWindow::on_action_super_triggered()
{
    GotoSuperMode();
}

void MainWindow::controlWindowClosed()
{
    ui->action_sendFile->setEnabled(true);
    ui->superButton->setEnabled(true);
}

void MainWindow::aboutApp()
{
    switch( QMessageBox::information( this,tr("���ڳ���")
                                      ,tr("������Ϊ���칤�ߵĿͻ��ˣ��������ӷ���ˡ�"
                                          "\n���ԣ��ż��룬�����Σ�������for�����ƴ�����"),tr("ȷ��"),0))
    {
    case 0:break;
    }
}

void MainWindow::on_action_about_triggered()
{
    aboutApp();
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

