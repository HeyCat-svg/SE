#ifndef CUBE_H
#define CUBE_H

#include <QLabel>
#include <QWidget>

class Cube : public QWidget
{
    Q_OBJECT
    friend class BoggleWindow;
public:
    explicit Cube(int i,int j,QWidget *parent = nullptr);
    void setLetter(QString l);
    void mousePressEvent(QMouseEvent *event);

signals:
    void clicked(int i,int j);
public slots:

private:
    QLabel *label;
    int i=-1;//cube在board中的位置i行j列
    int j=-1;
    bool flag=false;//此方块是否被用户点击
};

#endif // CUBE_H
