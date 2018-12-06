#include "mytool.h"
#include <QAction>
#include <QTimer>

mytool::mytool(QObject *) :
    QToolBar()
{
    inittoolbar();
}

void mytool::inittoolbar()
{
    isin=false;
    this->setMovable(false);

    action_showimage = new QAction(this);
    action_showimage->setText(tr("��ͣ����"));
    action_showimage->setToolTip(tr("ֹͣ���洫��"));
    action_showimage->setCheckable(true);
    action_showimage->setEnabled(true);
    action_showimage->setIcon(QIcon(":/actions/icons/pause.png"));

    action_fullscreen=new QAction(this);
    action_fullscreen->setText(tr("ȫ����ʾ"));
    action_fullscreen->setToolTip(tr("��󻯽�����ʾ"));
    action_fullscreen->setIcon(QIcon(":/actions/icons/fullscreen.png"));

    action_passfile=new QAction(this);
    action_passfile->setText(tr("�ļ���ȡ"));
    action_passfile->setToolTip(tr("��ȡ�Է��ļ�"));
    action_passfile->setCheckable(true);
    action_passfile->setEnabled(true);
    action_passfile->setIcon(QIcon(":/actions/icons/sendfile.png"));

    action_process=new QAction(this);
    action_process->setText(tr("���̹���"));
    action_process->setToolTip(tr("����Զ�̼�����Ľ���"));
    action_process->setCheckable(true);
    action_process->setEnabled(true);
    action_process->setIcon(QIcon(":/actions/icons/process.png"));

    action_shutdown=new QAction(this);
    action_shutdown->setText(tr("һ���ػ�"));
    action_shutdown->setToolTip(tr("��ݹر�Զ�̼����"));
    action_shutdown->setEnabled(true);
    action_shutdown->setIcon(QIcon(":/actions/icons/shutdown.png"));

    lockbox = new QCheckBox(this);
    connect(lockbox,SIGNAL(clicked(bool)),this,SLOT(lock(bool)));
    lockbox->setIcon(QIcon(":/actions/icons/lock.png"));
    lockbox->setIconSize(QSize(25,30));

    this->addAction(action_showimage);

    this->addAction(action_passfile);
    this->addAction(action_process);
    this->addAction(action_shutdown);
    this->addSeparator();
    this->addAction(action_fullscreen);
    this->addWidget(lockbox);

    this->setIconSize(QSize(23, 30));
    this->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
}

void mytool::enterEvent(QEvent *)
{
    if(!lockbox->isChecked())
    {
        isin = true;
        this->setMaximumHeight(40);
        this->setMinimumHeight(40);
    }
}

void mytool::leaveEvent(QEvent *)
{
    if(!lockbox->isChecked())
    {
        isin = false;
        QTimer::singleShot(1000, this, SLOT(timeout()));
    }
}

void mytool::lock(bool islock)
{
    if(islock)
    {
        lockbox->setText(tr("����"));
        lockbox->setToolTip(tr("������������״̬�����Զ�����"));
        lockbox->setIcon(QIcon(":/actions/icons/unlock.png"));
    }
    else
    {
        lockbox->setText(tr("����"));
        lockbox->setToolTip(tr("������������״̬�������Զ�����"));
        lockbox->setIcon(QIcon(":/actions/icons/lock.png"));
    }
}

void mytool::timeout()
{
    if(!isin)
    {
        this->setMaximumHeight(2);
        this->setMinimumHeight(2);
    }
}
