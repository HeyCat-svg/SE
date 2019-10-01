#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include "widget.h"

class MainWindow: public QWidget
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent=nullptr);
    ~MainWindow(){}

public slots:
    void buttonQuit();
    void buttonSingle();
    void buttonDouble();
    void buttonAuto();
    void dealBackSignal();

protected:
    /*void keyPressEvent(QKeyEvent *event);*/
private:
    Food *food;
    Snake *snake;
    Wall *wall;
    Widget *subWidget;
    QPushButton *quit;
    QPushButton *singlePlayer;
    QPushButton *doublePlayer;
    QPushButton *autoPlay;
};
#endif // MAINWINDOW_H
