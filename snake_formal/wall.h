#ifndef WALL_H
#define WALL_H

#include <QWidget>

class Widget;

class Wall : public QWidget{
public:
    Widget *widget;

    int wallX[6]={0};
    int wallY[6]={0};
    int portalX[4]={0};
    int portalY[4]={0};
    int timer;  //传送门闪烁计时器
    int R1,G1,B1,R2,G2,B2;//传送门RGB
    int R3,G3,B3,R4,G4,B4;

public:
    Wall();
    void timerEvent(QTimerEvent *event);
    void wallGenerater();
    void portalGenerater();
    void drawWall(QPainter &painter);
    void drawPortal(QPainter &painter);
    void checkWall(int x);
    void checkPortal(int x);
};

#endif // WALL_H
