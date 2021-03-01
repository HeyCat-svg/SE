#ifndef BOGGLEWINDOW_H
#define BOGGLEWINDOW_H

#include "Board.h"
#include "Console.h"
#include "WordListWidget.h"
#include "lexicon.h"
#include <string>
#include <QTimer>
#include <QEvent>

#include <QMainWindow>

class BoggleWindow : public QMainWindow
{
    Q_OBJECT

public:
    BoggleWindow(QWidget *parent = 0);
    ~BoggleWindow();

public slots:
    void receiveWords(QString str);
    void dealMouseEvent(int i,int j);

private:
    WordListWidget *me;
    WordListWidget *computer;
    Board *board;
    Console *console;
    Lexicon *lex;
    int timer;//输入正确单词标亮时间
    int status=0;  //0:undefined 1:Y but no words 2:N 3:Y

    static const int BOGGLE_WINDOW_WIDTH = 800;
    static const int BOGGLE_WINDOW_HEIGHT = 600;

    void timerEvent(QTimerEvent *event);
    void searchByComputer(int pos=-1);
    void searchByHuman(QString str);
    void highlight(int time);
    QString str="";//记录电脑搜索的单词片段
    QString str2="";//记录用户点击方块所产生的单词
    int row=-1;int col=-1;//记录用户上一个点击的方块的行列
};

#endif // BOGGLEWINDOW_H
