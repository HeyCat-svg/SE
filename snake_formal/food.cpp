#include "widget.h"

Food::Food(){}

void Food::foodGenerate(){
    bool flag;
    do{
        flag=false;
        foodX=randEx()%15*40;
        foodY=randEx()%15*40;
        for(int i=0;i<widget->snake->snakeNum;++i){
            for(int j=0;j<widget->snake->length[i];++j)
                if(foodX==widget->snake->bodyX[i][j]&&foodY==widget->snake->bodyY[i][j]){
                    flag=true;
                    break;
                }
            if(flag) break;
        }
        if(!flag)
            for(int i=0;i<6;++i)
                if(foodX==widget->wall->wallX[i]&&foodY==widget->wall->wallY[i]){
                    flag=true;
                    break;
                }
    }while(flag);
}

void Food::specialFoodGenerate(){
    bool flag;
    do{
        flag=false;
        specialFoodX=randEx()%15*40;
        specialFoodY=randEx()%15*40;
        for(int i=0;i<widget->snake->snakeNum;++i){
            for(int j=0;j<widget->snake->length[i];++j)
                if(foodX==widget->snake->bodyX[i][j]&&foodY==widget->snake->bodyY[i][j]){
                    flag=true;
                    break;
                }
            if(flag) break;
        }
        if(!flag)
            for(int i=0;i<6;++i)
                if(foodX==widget->wall->wallX[i]&&foodY==widget->wall->wallY[i]){
                    flag=true;
                    break;
                }
        if(!flag&&specialFoodX==foodX&&specialFoodY==foodY)
            flag=true;

    }while(flag);
    foodState=randEx()%3;
}

void Food::drawFood(QPainter &painter){
    QPoint food[4]={
        QPoint(foodX,foodY),
        QPoint(foodX+40,foodY),
        QPoint(foodX+40,foodY+40),
        QPoint(foodX,foodY+40)
    };
    QColor foodColor(255,0,0);
    painter.setPen(Qt::NoPen);
    painter.setBrush(foodColor);
    painter.drawConvexPolygon(food,4);

    QPoint specialFood[4]={
        QPoint(specialFoodX,specialFoodY),
        QPoint(specialFoodX+40,specialFoodY),
        QPoint(specialFoodX+40,specialFoodY+40),
        QPoint(specialFoodX,specialFoodY+40)
    };
    QColor specialFoodColor(128,0,128);
    painter.setPen(Qt::NoPen);
    painter.setBrush(specialFoodColor);
    painter.drawConvexPolygon(specialFood,4);
}

void Food::checkFood(int x){
    if(widget->snake->bodyX[x][0]==foodX&&widget->snake->bodyY[x][0]==foodY){
        ++(widget->snake->length[x]);
        widget->wall->wallGenerater();
        foodGenerate();
        widget->wall->portalGenerater();
        if(widget->snake->speed[x]>80)
            (widget->snake->speed[x])-=20;
        widget->speedUp(x,widget->snake->speed[x]);
    }
    else if(widget->snake->bodyX[x][0]==specialFoodX&&widget->snake->bodyY[x][0]==specialFoodY){
         switch(foodState){
         case 0:widget->keyReverse[x]=true;break;
         case 1:widget->speedUp(x,widget->snake->speed[x]/2);break;
         case 2:widget->invincibility[x]=true;break;
         }
         specialFoodX=600;
         specialFoodY=600;
         lastTimer[x]=startTimer(4000);
         cycleTimer=startTimer(15000);
         }
}

void Food::timerEvent(QTimerEvent *event){
    if(event->timerId()==lastTimer[0]){
        switch(foodState){
        case 0:widget->keyReverse[0]=false;break;
        case 1:widget->speedUp(0,widget->snake->speed[0]);break;
        case 2:widget->invincibility[0]=false;break;
        }
        killTimer(lastTimer[0]);
    }
    if(event->timerId()==lastTimer[1]){
        switch(foodState){
        case 0:widget->keyReverse[1]=false;break;
        case 1:widget->speedUp(1,widget->snake->speed[1]);break;
        case 2:widget->invincibility[1]=false;break;
        }
        killTimer(lastTimer[1]);
    }
    if(event->timerId()==cycleTimer){
        specialFoodGenerate();
        killTimer(cycleTimer);
    }
}
