#ifndef CONTROLWINDOW_H
#define CONTROLWINDOW_H

#include <QMainWindow>
#include <QPixmap>
#include <QModelIndex>
#include "mytool.h"
#include <QCloseEvent>
#include "myFileInfo.h"

namespace Ui {
    class ControlWindow;
}

class ControlWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit ControlWindow(QWidget *parent = 0);
    ~ControlWindow();

    void connectionClosed();

    //���̹���
    int processrow;
    void insertwidget(QString newline);
    void updateprocess();
signals:
    void sendRequest(QString type,QString msga,QString msgb);
    void controlWindowClosed();
private:
    Ui::ControlWindow *ui;
    mytool *newtool;

    QPixmap pixmap;

    QList<myFileInfo> fileList;
    QString currentPath;
    void showFileInfoList();
private slots:

    void on_enterButton_clicked();
    void on_tableWidget_file_doubleClicked(QModelIndex index);

    void on_killButton_clicked();
    void on_refreshButton_clicked();
    void on_tableWidget_clicked(QModelIndex index);

    //ͼ����ʾ
    void paintEvent(QPaintEvent *event);
    void resizeEvent(QResizeEvent *event);

    //������
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void mouseDoubleClickEvent(QMouseEvent *event);

    //���̿���
    void keyPressEvent(QKeyEvent *event);
    void keyReleaseEvent(QKeyEvent *event);

    //�ļ���ȡ
    void getFile(bool on);


    void closeEvent(QCloseEvent *event);
public slots:
    void stopImage(bool stop);
    void fullScreen();
    void flashProcess(bool);
    void shutdown();



    void imageReceived(QPixmap pixmapReceived);
    void dirReceived(QList<myFileInfo> list);
    void killprocess();
};

#endif // CONTROLWINDOW_H
