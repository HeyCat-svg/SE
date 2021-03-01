#include "widget.h"

Wall::Wall(){
    R1=147;G1=112;B1=219;
    R2=221;G2=160;B2=221;
    R3=67;G3=110;B3=238;
    R4=99;G4=184;B4=255;
    timer=startTimer(400);
}

void Wall::timerEvent(QTimerEvent *event){
    if(event->timerId()==timer){
        int tmp;
        tmp=R1;R1=R2;R2=tmp;
        tmp=G1;G1=G2;G2=tmp;
        tmp=B1;B1=B2;B2=tmp;

        tmp=R3;R3=R4;R4=tmp;
        tmp=G3;G3=G4;G4=tmp;
        tmp=B3;B3=B4;B4=tmp;
    }
}
void Wall::wallGenerater(){
    bool flag;
    for(int i=0;i<6;++i){
        do{
            flag=false;
            wallX[i]=randEx()%15*40;
            wallY[i]=randEx()%15*40;
            for(int j=0;j<i;++j)
                if(wallX[j]==wallX[i]&&wallY[j]==wallY[i]){
                    flag=true;
                    break;
                }
            if(!flag)
                for(int j=0;j<widget->snake->snakeNum;++j)
                    for(int k=0;k<widget->snake->length[j];++k)
                        if(wallX[i]==widget->snake->bodyX[j][k]&&wallY[i]==widget->snake->bodyY[j][k]){
                            flag=true;
                            break;
                        }
        }while(flag);

    }
}

void Wall::portalGenerater(){
    bool flag;
    for(int i=0;i<4;++i){
        do{
           flag=false;
           switch(i){
           case 0:portalX[i]=randEx()%7*40;
                  portalY[i]=randEx()%7*40;break;//左上
           case 1:portalX[i]=(randEx()%7+8)*40;
                  portalY[i]=(randEx()%7+8)*40;break;//右下
           case 2:portalX[i]=(randEx()%7+8)*40;
                  portalY[i]=randEx()%7*40;break;//右上
           case 3:portalX[i]=randEx()%7*40;
                  portalY[i]=(randEx()%7+8)*40;break;//左下
           }
           for(int j=0;j<6;++j)
               if(portalX[i]==wallX[j]&&portalY[i]==wallY[j]){
                   flag=true;
                   break;
               }
           if(!flag)
               for(int j=0;j<widget->snake->snakeNum;++j)
                   for(int k=0;k<widget->snake->length[j];++k)
                       if(portalX[i]==widget->snake->bodyX[j][k]&&portalY[i]==widget->snake->bodyY[j][k]){
                           flag=true;
                           break;
                       }
           if(portalX[i]==widget->food->foodX&&portalY[i]==widget->food->foodY) flag=true;
           if(portalX[i]==widget->food->specialFoodX&&portalY[i]==widget->food->specialFoodY) flag=true;
           if(i==2&&portalX[2]==portalX[1]) flag=true;
        }while(flag);
    }
}

void Wall::drawWall(QPainter &painter){
    for(int i=0;i<6;++i){
        QPoint wall[4]{
            QPoint(wallX[i],wallY[i]),
            QPoint(wallX[i]+40,wallY[i]),
            QPoint(wallX[i]+40,wallY[i]+40),
            QPoint(wallX[i],wallY[i]+40)
        };
        QColor wallColor(0,0,0);
        painter.setPen(Qt::NoPen);
        painter.setBrush(wallColor);
        painter.drawConvexPolygon(wall,4);
    }
}

void Wall::drawPortal(QPainter &painter){
    for(int i=0;i<4;++i){
        QPoint portal[4]{
            QPoint(portalX[i],portalY[i]),
            QPoint(portalX[i]+40,portalY[i]),
            QPoint(portalX[i]+40,portalY[i]+40),
            QPoint(portalX[i],portalY[i]+40)
        };
        if(i==0||i==1){
            QColor portalColor(R1,G1,B1);
            painter.setPen(Qt::NoPen);
            painter.setBrush(portalColor);
            painter.drawConvexPolygon(portal,4);
        }
        else{
            QColor portalColor(R3,G3,B3);
            painter.setPen(Qt::NoPen);
            painter.setBrush(portalColor);
            painter.drawConvexPolygon(portal,4);
        }
    }

    for(int i=portalX[0]+40;i<=portalX[1];i+=40){
        QPoint track[4]{
          QPoint(i+10,portalY[0]+10),
          QPoint(i+10+20,portalY[0]+10),
          QPoint(i+10+20,portalY[0]+10+20),
          QPoint(i+10,portalY[0]+10+20),
        };
        if(i/40%2){
            QColor trackColor(R1,G1,B1);
            painter.setPen(Qt::NoPen);
            painter.setBrush(trackColor);
            painter.drawConvexPolygon(track,4);
        }
        else{
            QColor trackColor(R2,G2,B2);
            painter.setPen(Qt::NoPen);
            painter.setBrush(trackColor);
            painter.drawConvexPolygon(track,4);
        }
    }

    for(int i=portalY[0]+40;i<portalY[1];i+=40){
        QPoint track[4]{
          QPoint(portalX[1]+10,i+10),
          QPoint(portalX[1]+10+20,i+10),
          QPoint(portalX[1]+10+20,i+10+20),
          QPoint(portalX[1]+10,i+10+20),
        };
        if(i/40%2){
            QColor trackColor(R1,G1,B1);
            painter.setPen(Qt::NoPen);
            painter.setBrush(trackColor);
            painter.drawConvexPolygon(track,4);
        }
        else{
            QColor trackColor(R2,G2,B2);
            painter.setPen(Qt::NoPen);
            painter.setBrush(trackColor);
            painter.drawConvexPolygon(track,4);
        }
    }

    for(int i=portalX[3]+40;i<=portalX[2];i+=40){
        QPoint track[4]{
          QPoint(i+10,portalY[3]+10),
          QPoint(i+10+20,portalY[3]+10),
          QPoint(i+10+20,portalY[3]+10+20),
          QPoint(i+10,portalY[3]+10+20),
        };
        if(i/40%2){
            QColor trackColor(R3,G3,B3);
            painter.setPen(Qt::NoPen);
            painter.setBrush(trackColor);
            painter.drawConvexPolygon(track,4);
        }
        else{
            QColor trackColor(R4,G4,B4);
            painter.setPen(Qt::NoPen);
            painter.setBrush(trackColor);
            painter.drawConvexPolygon(track,4);
        }
    }

    for(int i=portalY[2]+40;i<portalY[3];i+=40){
        QPoint track[4]{
          QPoint(portalX[2]+10,i+10),
          QPoint(portalX[2]+10+20,i+10),
          QPoint(portalX[2]+10+20,i+10+20),
          QPoint(portalX[2]+10,i+10+20),
        };
        if(i/40%2){
            QColor trackColor(R3,G3,B3);
            painter.setPen(Qt::NoPen);
            painter.setBrush(trackColor);
            painter.drawConvexPolygon(track,4);
        }
        else{
            QColor trackColor(R4,G4,B4);
            painter.setPen(Qt::NoPen);
            painter.setBrush(trackColor);
            painter.drawConvexPolygon(track,4);
        }
    }
}

void Wall::checkWall(int x){
    bool flag=false;
    for(int i=0;i<6;++i)
        if(widget->snake->bodyX[x][0]==wallX[i]&&widget->snake->bodyY[x][0]==wallY[i]){
            flag=true;
            break;
        }
    if(flag){
        for(int i=0;i<widget->snake->length[x];++i){
            widget->snake->bodyX[x][i]=widget->snake->bodyX[x][i+1];
            widget->snake->bodyY[x][i]=widget->snake->bodyY[x][i+1];
        }
        if(widget->snake->state[x]==0||widget->snake->state[x]==1)
            widget->snake->state[x]=randEx()%2+2;
        else widget->snake->state[x]=randEx()%2;
        if(!widget->invincibility[x])
            widget->snake->length[x]-=3;
    }
}

void Wall::checkPortal(int x){
    if(widget->snake->bodyX[x][0]==portalX[0]&&widget->snake->bodyY[x][0]==portalY[0]){
        widget->snake->bodyX[x][0]=portalX[1];
        widget->snake->bodyY[x][0]=portalY[1];
    }
    else if(widget->snake->bodyX[x][0]==portalX[1]&&widget->snake->bodyY[x][0]==portalY[1]){
        widget->snake->bodyX[x][0]=portalX[0];
        widget->snake->bodyY[x][0]=portalY[0];
    }
    else if(widget->snake->bodyX[x][0]==portalX[2]&&widget->snake->bodyY[x][0]==portalY[2]){
        widget->snake->bodyX[x][0]=portalX[3];
        widget->snake->bodyY[x][0]=portalY[3];
    }
    else if(widget->snake->bodyX[x][0]==portalX[3]&&widget->snake->bodyY[x][0]==portalY[3]){
        widget->snake->bodyX[x][0]=portalX[2];
        widget->snake->bodyY[x][0]=portalY[2];
    }
}
