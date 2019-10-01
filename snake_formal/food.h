#ifndef FOOD_H
#define FOOD_H

#include <QWidget>

class Widget;

class Food : public QWidget{
public:
    int foodX,foodY;
    int specialFoodX,specialFoodY;
    int foodState;//特种食物 0：失控 1：短暂加速 2：短暂无敌
    int lastTimer[2]={0}; //效果持续时间
    int cycleTimer; //特种食物出现周期
    Widget *widget;

public:
    Food();
    void foodGenerate();
    void specialFoodGenerate();
    void drawFood(QPainter &painter);
    void checkFood(int x);
    void timerEvent(QTimerEvent *event);
};
#endif // FOOD_H
