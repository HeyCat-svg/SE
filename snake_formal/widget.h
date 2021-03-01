#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QKeyEvent>
#include <QPainter>
#include <QPushButton>
#include <QPen>
#include <QBrush>
#include <QDebug>
#include <QTimer>
#include <QTime>
#include <windows.h>
#include <iostream>
#include "food.h"
#include "snake.h"
#include "wall.h"

class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(int snakeNum=1,QWidget *parent = nullptr);
    ~Widget();

    void paintEvent(QPaintEvent *);
    void keyPressEvent(QKeyEvent *event);
    void timerEvent(QTimerEvent *event);

signals:
    void backSignal();

public slots:
    void buttonPause();
    void buttonContinue();
    void sendSlot();

public:
    Snake *snake;
    Food *food;
    Wall *wall;
    QTimer *timer;      //画面刷新速度
    int m_timer[2];     //蛇移动速度
    bool keyReverse[2]={false,false};//对应特种食物的按键反转
    bool invincibility[2]={false,false};//对应特种食物无视墙壁
    bool _autoMove[2]={false,false};//AI蛇
    int abnormal[2]={0,0}; //卡循环次数超过2次就换方向

    QPushButton *pause;
    QPushButton *_continue;
    QPushButton *back;

    void move(int x);
    void invincibleMove(int x);
    void autoMove(int x);
    bool check(int x);
    void speedUp(int x,int speed);

};
int randEx();

#endif // WIDGET_H
