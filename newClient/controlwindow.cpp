#include "controlwindow.h"
#include "ui_controlwindow.h"
#include <QPainter>
#include <QMessageBox>
#include <QResizeEvent>
#include <QTableWidgetItem>

ControlWindow::ControlWindow(QWidget *parent) :
        QMainWindow(parent),
        ui(new Ui::ControlWindow)
{
    ui->setupUi(this);

    //toolbar
    newtool =new mytool(this);
    this->addToolBar(newtool);

    this->setWindowIcon(QIcon(":/actions/icons/windowicon.png"));
    this->setWindowTitle(tr("����ģʽ��"));

    connect(newtool->action_showimage,SIGNAL(triggered(bool)),this,SLOT(stopImage(bool)));
    connect(newtool->action_fullscreen,SIGNAL(triggered()),this,SLOT(fullScreen()));  //ȫ��
    connect(newtool->action_process,SIGNAL(triggered(bool)),this,SLOT(flashProcess(bool)));  //����
    connect(newtool->action_passfile,SIGNAL(triggered(bool)),this,SLOT(getFile(bool)));  //���ļ�
    connect(newtool->action_shutdown,SIGNAL(triggered()),this,SLOT(shutdown()));    //�ػ�


    newtool->lockbox->click();

    ui->killButton->setEnabled(false);

    ui->tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);   //��Ŀ���ɱ༭
    ui->tableWidget->setAlternatingRowColors(true);        //���л���ɫ
    ui->process_widget->hide();

    ui->tableWidget_file->setEditTriggers(QAbstractItemView::NoEditTriggers);   //��Ŀ���ɱ༭
    ui->tableWidget_file->setAlternatingRowColors(true);        //���л���ɫ
    ui->file_widget->hide();

    pixmap.load(":/actions/icons/jordan.png");
    this->update();

    this->resize(560, 320);
    processrow = 0;
}

ControlWindow::~ControlWindow()
{
    delete ui;
}

void ControlWindow::connectionClosed()
{
    ui->killButton->setEnabled(false);
}

//ͼ����ʾ
void ControlWindow::stopImage(bool stop)
{
    if (stop)
    {
        sendRequest("unsuper", "", "");
        newtool->action_showimage->setText(tr("��ȡ����"));
        newtool->action_showimage->setToolTip(tr("��ȡԶ�̻���"));
        newtool->action_showimage->setIcon(QIcon(":/actions/icons/start.png"));
    }
    else
    {
        sendRequest("super", "", "");
        newtool->action_showimage->setText(tr("��ͣ����"));
        newtool->action_showimage->setToolTip(tr("ֹͣ���洫��"));
        newtool->action_showimage->setIcon(QIcon(":/actions/icons/pause.png"));
    }
}

void ControlWindow::imageReceived(QPixmap pixmapReceived)
{
    pixmap = pixmapReceived;
    this->update();
}

void ControlWindow::resizeEvent(QResizeEvent *event)
{
    event->accept();

    QString winwidth;
    winwidth.setNum(this->width());
    QString winheight;
    winheight.setNum(this->height());
    sendRequest("winsize",winwidth,winheight);
}

void ControlWindow::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.drawPixmap( 0, 0, this->width(), this->height(), pixmap);
}

void ControlWindow::fullScreen()
{
    if(this->isFullScreen())
    {
        this->showNormal();
        newtool->action_fullscreen->setText(tr("ȫ����ʾ"));
        newtool->action_fullscreen->setToolTip(tr("��󻯽�����ʾ"));
    }
    else
    {
        this->showFullScreen();
        newtool->action_fullscreen->setText(tr("ȡ��ȫ��"));
        newtool->action_fullscreen->setToolTip(tr("ȡ����󻯽�����ʾ"));
    }
}

//һ���ػ�//
void ControlWindow::shutdown()
{
    sendRequest("shutdown","","");
}


//���̹���//
void ControlWindow::updateprocess()
{

    ui->tableWidget->setRowCount(0);
    sendRequest("update","","");
}

void ControlWindow::insertwidget(QString newline)
{
    QString oneline=newline;
    QStringList fields;
    for(int i=0;i<oneline.size();i++)
    {
        QString needstring=oneline;
        QString cutstring=oneline;

        if(oneline.at(i)==' '&&oneline.at(i+1)==' ')
        {
            needstring.remove(i,needstring.size());
            cutstring.remove(0,i);
            fields << needstring;

            for(int j=0;j<cutstring.size();j++)
            {
                if(cutstring.at(j)!=' '||j==cutstring.size())
                {
                    cutstring.remove(0,j);
                    oneline=cutstring;
                    i=0;
                    break;
                }
            }
        }
    }

    ui->tableWidget->setRowCount(ui->tableWidget->rowCount()+1);
    for (int k = 0; k < fields.count(); ++k)
    {
        ui->tableWidget->setItem(ui->tableWidget->rowCount()-1, k,new QTableWidgetItem(fields[k]));

    }

    ui->tableWidget->resizeColumnsToContents();
}
void ControlWindow::killprocess()
{
    switch( QMessageBox::information(this,tr("������ʾ"),tr("ȷ��Ҫɱ������%1")
                                     .arg(ui->tableWidget->item(processrow,0)->text()),tr("ȷ��"),tr("ȡ��"),0,1))
    {
    case 0:{
            QString getpid=ui->tableWidget->item(processrow,1)->text();
            getpid.remove(getpid.indexOf(' '),getpid.size());
            sendRequest("killprocess",getpid,"");
            updateprocess();
            break;
        }
    case 1:break;

    }
}

//��ʾ�����б�ֹͣͼ����ʾ

void ControlWindow::flashProcess(bool flash)
{
    if(flash)
    {
        ui->process_widget->show();

        ui->file_widget->hide();

        newtool->action_showimage->setEnabled(false);
        updateprocess();
    }
    else
    {
        stopImage(false);
        newtool->action_showimage->setEnabled(true);
        ui->process_widget->hide();
    }

}

void ControlWindow::on_tableWidget_clicked(QModelIndex index)
{
    processrow=index.row();
    ui->killButton->setEnabled(true);
}

void ControlWindow::on_refreshButton_clicked()
{
    updateprocess();
}

void ControlWindow::on_killButton_clicked()
{
    killprocess();
}


//������
void ControlWindow::mousePressEvent(QMouseEvent *event)
{
    event->accept();

    QString xx;
    xx.setNum(event->x());
    QString yy;
    yy.setNum(event->y());
    if(event->button()==Qt::LeftButton)
    {
        sendRequest("Lmousepress",xx,yy);
    }
    if(event->button()==Qt::RightButton)
    {
        sendRequest("Rmousepress",xx,yy);
    }

}

void ControlWindow::mouseMoveEvent(QMouseEvent *event)
{
    event->accept();

    QString xx;
    xx.setNum(event->x());
    QString yy;
    yy.setNum(event->y());
    sendRequest("mousemov",xx,yy);

}

void ControlWindow::mouseReleaseEvent(QMouseEvent *event)
{
    event->accept();

    QString xx;
    xx.setNum(event->x());
    QString yy;
    yy.setNum(event->y());
    if(event->button()==Qt::LeftButton)
    {
        sendRequest("Lmouserelease",xx,yy);
    }
    if(event->button()==Qt::RightButton)
    {
        sendRequest("Rmouserelease",xx,yy);
    }

}

void ControlWindow::mouseDoubleClickEvent(QMouseEvent *event)
{
    event->accept();

    sendRequest("doubleclick","","");

}

void ControlWindow::keyPressEvent(QKeyEvent *event)
{
    event->accept();

    QString keynum;
    keynum.setNum(event->key());
    sendRequest("keypress",keynum,event->text());

}

void ControlWindow::keyReleaseEvent(QKeyEvent *event)
{
    event->accept();

    QString keynum;
    keynum.setNum(event->key());
    sendRequest("keyrelease",keynum,event->text());

}

//�ļ���ȡ
void ControlWindow::getFile(bool on)
{
    if (on)
    {
        newtool->action_showimage->setEnabled(false);
        ui->process_widget->hide();

        ui->file_widget->show();

        sendRequest("getdir", "", "");

    }
    else
    {
        stopImage(false);
        newtool->action_showimage->setEnabled(true);
        ui->file_widget->hide();
    }
}

void ControlWindow::dirReceived(QList<myFileInfo> list)
{
    fileList.clear();
    fileList = list;

    currentPath = fileList.at(0).fileAbsolutePath;
    ui->lineEdit_file->setText(currentPath);

    showFileInfoList();
}

void ControlWindow::showFileInfoList()
{
    ui->tableWidget_file->clearContents();
    ui->tableWidget_file->setRowCount(fileList.count());


    for (qint32 i=0; i<fileList.count(); i++)
    {
        myFileInfo tempFileInfo = fileList.at(i);
        QIcon icon;

        if (tempFileInfo.isDir)
        {
            icon.addFile(":/actions/icons/dir.png");
            tempFileInfo.fileSize = "--";
        }

        else
        {
            icon.addFile(":/actions/icons/file.png");
            tempFileInfo.fileSize += "�ֽ�";
        }

        QTableWidgetItem *item = new QTableWidgetItem(icon, tempFileInfo.fileName);
        ui->tableWidget_file->setItem(i, 0, item);
        ui->tableWidget_file->setItem(i, 1, new QTableWidgetItem(tempFileInfo.fileSize));
        ui->tableWidget_file->setItem(i, 2, new QTableWidgetItem(tempFileInfo.fileDate));
    }

}

void ControlWindow::closeEvent(QCloseEvent *event)
{
    stopImage(true);
    emit controlWindowClosed();
    event->accept();
}

void ControlWindow::on_tableWidget_file_doubleClicked(QModelIndex index)
{
    int r = index.row();
    if (fileList.at(r).isDir)
    {
        sendRequest("cd", currentPath, fileList.at(r).fileName);
    }
    else
    {
        switch( QMessageBox::information(this ,tr("��ȡ�ļ���ʾ"),tr("��ȷ��Ҫ��ȡ�ļ���") + fileList.at(r).fileName + "?",tr("ȷ��"),tr("ȡ��"),0,1))
        {
        case 0:
            {
                sendRequest("getfile", currentPath + "/" + fileList.at(r).fileName, "");
                break;
            }
        case 1:
            {//ȡ��
                break;
            }
        }
    }
}

void ControlWindow::on_enterButton_clicked()
{
    sendRequest("cd", ui->lineEdit_file->text().trimmed(), "");
}
