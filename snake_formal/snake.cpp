#include "widget.h"


void Snake::drawSnake(QPainter &painter){
    int R,G,B;
    for(int i=0;i<snakeNum;++i){
        for(int j=0;j<length[i];++j){
            QPoint snakeBody[4]={
                QPoint(bodyX[i][j],bodyY[i][j]),
                QPoint(bodyX[i][j]+40,bodyY[i][j]),
                QPoint(bodyX[i][j]+40,bodyY[i][j]+40),
                QPoint(bodyX[i][j],bodyY[i][j]+40)

            };
            if(i==0){
                if(j%2) {R=102;G=255;B=0;}
                else{ R=0;G=220;B=0;}
            }
            else{
                if(j%2){R=127;G=255;B=212;}
                else {R=100;G=149;B=237;}
            }
            QColor snakeColor(R,G,B);
            painter.setPen(Qt::NoPen);
            painter.setBrush(snakeColor);
            painter.drawConvexPolygon(snakeBody,4);
        }
    }

}
