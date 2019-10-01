#include "widget.h"
#include <stdlib.h>
#include <time.h>

Widget::Widget(int snakeNum,QWidget *parent)
    : QWidget(parent)
{
    this->setParent(parent);
    this->setWindowTitle(QString("snake"));
    this->resize(760,600);
    setStyleSheet("QWidget{background:white}");
    setWindowOpacity(0.8);

    //链接各对象
    food=new Food;
    snake=new Snake;
    wall=new Wall;
    food->widget=this;
    wall->widget=this;
    snake->widget=this;

    //define the button
    pause=new QPushButton(this);
    pause->setText("Pause");
    pause->setGeometry(620,350,100,50);
    connect(pause,&QPushButton::released,this,&Widget::buttonPause);

    _continue=new QPushButton(this);
    _continue->setText("Continue");
    _continue->setGeometry(620,350,100,50);
    connect(_continue,&QPushButton::released,this,&Widget::buttonContinue);
    _continue->hide();

    back=new QPushButton(this);
    back->setText("Back");
    back->setGeometry(620,450,100,50);
    connect(back,&QPushButton::released,this,&Widget::sendSlot);

    //启动定时器
    this->snake->snakeNum=snakeNum;
    for(int i=0;i<snake->snakeNum;++i)
        m_timer[i]=startTimer(500);
    QTimer *timer=new QTimer(this);
    connect(timer,SIGNAL(timeout()),this,SLOT(update()));
    timer->start(20);

    wall->wallGenerater();
    food->foodGenerate();
    food->specialFoodGenerate();
    wall->portalGenerater();
}

Widget::~Widget(){}

void Widget::paintEvent(QPaintEvent *){
    //calculate the score
    QPainter painter(this);
    QPen pen;
    QFont font("微软雅黑",12,QFont::ExtraLight,false);
    pen.setColor(Qt::black);
    painter.setPen(pen);
    painter.setFont(font);
    for(int i=0;i<snake->snakeNum;++i){
        painter.drawText(620,20+20*i,QString("蛇")+QString("%1").arg(i+1)+QString("当前的分：")+QString("%1").arg(snake->length[i]));
    }

    //draw the map
    painter.setRenderHint(QPainter::Antialiasing);
    for(int j=0;j<=600;j+=40){
        painter.drawLine(0,j,600,j);
        painter.drawLine(j,0,j,600);
    }

    //draw the snakes
    snake->drawSnake(painter);

    //draw the food
    food->drawFood(painter);

    //draw the wall
    wall->drawWall(painter);

    //draw the portal
    wall->drawPortal(painter);

    //output "GAME OVER"
    if(check(0)&&check(1)){
        QFont font("微软雅黑",30,QFont::ExtraLight,false);
        pen.setColor(Qt::black);
        painter.setPen(pen);
        painter.setFont(font);
        painter.drawText(200,300,QString("GAME OVER!"));

        pause->hide();
    }
}

void Widget::timerEvent(QTimerEvent *event){
    if(event->timerId()==m_timer[0])
        move(0);
    if(event->timerId()==m_timer[1])
        move(1);
}

void Widget::keyPressEvent(QKeyEvent *event){
    if(event->key()==Qt::Key_W&&snake->state[0]!=2&&snake->state[0]!=3){
        if(keyReverse[0]) snake->state[0]=3;
        else snake->state[0]=2;
    }
    if(event->key()==Qt::Key_A&&snake->state[0]!=1&&snake->state[0]!=0){
        if(keyReverse[0]) snake->state[0]=1;
        else snake->state[0]=0;
    }
    if(event->key()==Qt::Key_S&&snake->state[0]!=2&&snake->state[0]!=3){
        if(keyReverse[0]) snake->state[0]=2;
        else snake->state[0]=3;
    }
    if(event->key()==Qt::Key_D&&snake->state[0]!=1&&snake->state[0]!=0){
        if(keyReverse[0]) snake->state[0]=0;
        else snake->state[0]=1;
    }
    if(event->key()==Qt::Key_8&&snake->state[1]!=2&&snake->state[1]!=3){
        if(keyReverse[1]) snake->state[1]=3;
        else snake->state[1]=2;
    }
    if(event->key()==Qt::Key_4&&snake->state[1]!=0&&snake->state[1]!=1){
        if(keyReverse[1]) snake->state[1]=1;
        else snake->state[1]=0;
    }
    if(event->key()==Qt::Key_5&&snake->state[1]!=2&&snake->state[1]!=3){
        if(keyReverse[1]) snake->state[1]=2;
        else snake->state[1]=3;
    }
    if(event->key()==Qt::Key_6&&snake->state[1]!=0&&snake->state[1]!=1){
        if(keyReverse[1]) snake->state[1]=0;
        else snake->state[1]=1;
    }
}

void Widget::buttonPause(){
    for(int i=0;i<snake->snakeNum;++i)
        killTimer(m_timer[i]);
    pause->hide();
    _continue->show();
}

void Widget::buttonContinue(){
    for(int i=0;i<snake->snakeNum;++i)
        m_timer[i]=startTimer(snake->speed[i]);
    _continue->hide();
    pause->show();
}

void Widget::sendSlot(){
    emit backSignal();
}

void Widget::move(int x){
    if(check(x)){
        killTimer(m_timer[x]);
        return;
    }
    int tmpX=snake->bodyX[x][0];
    int tmpY=snake->bodyY[x][0];
    for(int j=snake->length[x];j>0;--j){
        snake->bodyX[x][j]=snake->bodyX[x][j-1];
        snake->bodyY[x][j]=snake->bodyY[x][j-1];
    }
    switch(snake->state[x]){
    case 0:snake->bodyX[x][0]-=40;break;
    case 1:snake->bodyX[x][0]+=40;break;
    case 2:snake->bodyY[x][0]-=40;break;
    case 3:snake->bodyY[x][0]+=40;break;
    }

    wall->checkPortal(x);
    food->checkFood(x);
    if(_autoMove[x]) autoMove(x);
    else wall->checkWall(x);

    if(tmpX==snake->bodyX[x][0]&&tmpY==snake->bodyY[x][0])
        ++abnormal[x];
    else abnormal[x]=0;
    if(abnormal[x]==2){
        snake->state[x]=randEx()%4;
        abnormal[x]=0;
    }
}

void Widget::invincibleMove(int x){
        for(int i=0;i<snake->length[x];++i){
            snake->bodyX[x][i]=snake->bodyX[x][i+1];
            snake->bodyY[x][i]=snake->bodyY[x][i+1];
        }
        if(snake->state[x]==0||snake->state[x]==1)
            snake->state[x]=randEx()%2+2;
        else snake->state[x]=randEx()%2;
}

bool Widget::check(int x){
    if(x+1>snake->snakeNum) return true;
    if(snake->bodyX[x][0]/40<0||snake->bodyX[x][0]/40>14||snake->bodyY[x][0]/40<0||snake->bodyY[x][0]/40>14){
        if(invincibility[x]){
            invincibleMove(x);
            return false;
        }
        else return true;
    }
    for(int j=1;j<snake->length[x];++j){
        if(snake->bodyX[x][j]==snake->bodyX[x][0]&&snake->bodyY[x][j]==snake->bodyY[x][0]){
            if(invincibility[x])
                return false;
            else return true;
        }
    }
    if(snake->length[x]<1) return true;
    return false;
}

void Widget::speedUp(int x,int speed){
    killTimer(m_timer[x]);
    m_timer[x]=startTimer(speed);
}

void Widget::autoMove(int x){
    bool flag=false;
    bool flag1=false;
    for(int i=0;i<6;++i)
        if(snake->bodyX[x][0]==wall->wallX[i]&&snake->bodyY[x][0]==wall->wallY[i]){
            flag=true;
            break;
        }
    for(int i=1;i<snake->length[x];++i)
        if(snake->bodyX[x][i]==snake->bodyX[x][0]&&snake->bodyY[x][i]==snake->bodyY[x][0]){
            flag1=true;
            break;
        }
    if(flag){
        for(int i=0;i<snake->length[x];++i){
            snake->bodyX[x][i]=snake->bodyX[x][i+1];
            snake->bodyY[x][i]=snake->bodyY[x][i+1];
        }
        if(snake->state[x]==2||snake->state[x]==3){
            if(food->foodX>snake->bodyX[x][0]) snake->state[x]=1;
            else snake->state[x]=0;
        }
        else{
            if(food->foodY>snake->bodyY[x][0]) snake->state[x]=3;
            else snake->state[x]=2;
        }
    }
    else if(flag1){
        for(int i=0;i<snake->length[x];++i){
            snake->bodyX[x][i]=snake->bodyX[x][i+1];
            snake->bodyY[x][i]=snake->bodyY[x][i+1];
        }
        if(snake->state[x]==2||snake->state[x]==3){
            int i=1;
            for(;i<snake->length[x];++i)
                if(snake->bodyY[x][i]==snake->bodyY[x][0])
                    break;
            if(snake->bodyX[x][i]>snake->bodyX[x][0]) snake->state[x]=0;
            else snake->state[x]=1;
        }
        else{
            int i=1;
            for(;i<snake->length[x];++i)
                if(snake->bodyX[x][i]==snake->bodyX[x][0])
                    break;
            if(snake->bodyY[x][i]>snake->bodyY[x][0]) snake->state[x]=2;
            else snake->state[x]=3;
        }
    }else{
        if(food->foodY>snake->bodyY[x][0]){
            if(snake->state[x]==2){
                if(food->foodX>snake->bodyX[x][0]) snake->state[x]=1;
                else snake->state[x]=0;
            }
            else if(snake->state[x]==0||snake->state[x]==1) snake->state[x]=3;
        }

        else if(food->foodY<snake->bodyY[x][0]){
            if(snake->state[x]==3){
                if(food->foodX>snake->bodyX[x][0]) snake->state[x]=1;
                else snake->state[x]=0;
            }
            else if(snake->state[x]==0||snake->state[x]==1) snake->state[x]=2;
        }
        else if(food->foodY==snake->bodyY[x][0]){
            if(food->foodX>snake->bodyX[x][0]){
                if(snake->state[x]==2||snake->state[x]==3) snake->state[x]=1;
                //以下部分会有蛇与自身相撞的风险
                else if(snake->state[x]==0){
                    if(snake->bodyY[x][0]==0) snake->state[x]=3;
                    else snake->state[x]=2;
                }
            }
            else{
                if(snake->state[x]==2||snake->state[x]==3) snake->state[x]=0;
                //以下部分会有蛇与自身相撞的风险
                else if(snake->state[x]==1){
                    if(snake->bodyY[x][0]==0) snake->state[x]=3;
                    else snake->state[x]=2;
                }
            }
        }
    }
}

int randEx(){
    LARGE_INTEGER seed;
    QueryPerformanceFrequency(&seed);
    QueryPerformanceCounter(&seed);
    srand(seed.QuadPart);
    return rand();
}



