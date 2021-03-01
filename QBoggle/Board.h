#ifndef BOARD_H
#define BOARD_H

#include "Cube.h"
#include <stdlib.h>
#include <time.h>
#include <windows.h>
#include <QWidget>

class Board : public QWidget
{
    Q_OBJECT
    friend class BoggleWindow;
public:
    explicit Board(QWidget *parent = nullptr, int size = 5, const QString *cubeLetters = BIG_BOGGLE_CUBES);
    virtual ~Board();
    void shake();

signals:

public slots:

private:
    int size;
    Cube **cubes;
    QString *letters;
    inline int index(int i, int j) { return i * size + j; }
    int randEx(int min,int max);
    static const QString STANDARD_CUBES[16];
    static const QString BIG_BOGGLE_CUBES[25];  

private:
    struct edgeNode{
        int end;
        edgeNode *next;

        edgeNode(int e,edgeNode *n=NULL){end=e;next=n;}
    };

    struct verNode{
        QChar ch;
        edgeNode *head;

        verNode(edgeNode *h=NULL){head=h;}
    };

    verNode *verList;
    bool *book;
    bool *word;//记录所查找str成功的单词序号
    bool flag=false;//判断搜索是否成功
    int find(QChar v)const;
    void insert(int start,int end);
    void buildGraph();
    void clearGraph();
    void searchHelper(int pos,QString str);
    bool search(QString str);
};

#endif // BOARD_H
