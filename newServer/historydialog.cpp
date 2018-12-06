#include "historydialog.h"
#include "ui_historydialog.h"

HistoryDialog::HistoryDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::HistoryDialog)
{
    ui->setupUi(this);
    initHistoryFile();
    readFile();
    this->setWindowTitle(tr("�����¼"));
    this->setWindowIcon(QIcon(":/actions/icons/windowicon.png"));
}

HistoryDialog::~HistoryDialog()
{
    delete ui;
}

void HistoryDialog::initHistoryFile()
{
    file = new QFile("user.dat");
    MagicNumber = 0x12345678;

    out.setDevice(file);
    out.setVersion(QDataStream::Qt_4_7);
    in.setDevice(file);
    in.setVersion(QDataStream::Qt_4_7);

    if (!file->exists())
    {
        file->open(QIODevice::WriteOnly);
        out << MagicNumber;
        file->close();
    }

}

void HistoryDialog::readFile()
{
     QString msg;
     quint32 magic;

     if (!file->open(QIODevice::ReadOnly))
     {
         ui->label->setText(tr("�����¼��ȡʧ��"));
         return;
     }

     in >> magic;

     if (magic != MagicNumber)
     {
         ui->label->setText(tr("�����¼��ʽ����"));
         return;
     }

     while (!in.atEnd())
     {
         in >> msg;
         ui->listWidget->addItem(msg);
     }
     file->close();
}

void HistoryDialog::appendMsg(QString msg)
{
    if (!file->open(QIODevice::Append))
    {
       ui->label->setText(tr("�����¼����ʧ��"));
       return;
    }

    ui->listWidget->addItem(msg);
    out << msg;

    file->close();
}
