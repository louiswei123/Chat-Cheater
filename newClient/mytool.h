#ifndef MYTOOL_H
#define MYTOOL_H

#include <QToolBar>
#include <QCheckBox>
#include <QProgressBar>

class mytool : public QToolBar
{
    Q_OBJECT
public:
    explicit mytool(QObject *);
    void enterEvent(QEvent *); // �ض���
    void leaveEvent(QEvent *);

    QCheckBox *lockbox;
    QAction *action_showimage;
    QAction *action_fullscreen;
    QAction *action_passfile;
    QAction *action_shutdown;
    QAction *action_process;

    bool isin;//��ʶ��굱ǰ�Ƿ��ڹ�������

public slots:
    void inittoolbar();
    void lock(bool islock);
    void timeout();

};

#endif // MYTOOL_H
