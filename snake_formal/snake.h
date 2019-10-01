#ifndef SNAKE_H
#define SNAKE_H

#include <QWidget>

class Widget;

class Snake : public QWidget{
    Q_OBJECT
public:
    Widget *widget;

    int snakeNum=1;
    int bodyX[2][100]={0};
    int bodyY[2][100]={0};
    int length[2]={1,1};
    int speed[2]={500,500};
    int state[2]={1,3};    //0:left 1:right 2:up 3:down

public:
    Snake(){}
    void drawSnake(QPainter &painter);
};



#endif // SNAKE_H
