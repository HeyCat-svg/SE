#ifndef WORDLISTWIDGET_H
#define WORDLISTWIDGET_H

#include "WordTable.h"
#include <QHBoxLayout>
#include <QLabel>
#include <QTableView>

#include <QWidget>

class WordListWidget : public QWidget
{
    Q_OBJECT
public:
    explicit WordListWidget(QWidget *parent = nullptr, QString label = "");
    void addScore(int s);
    void addWord(QString word);
    void reset();
    bool check(QString str);//看str是否在单词表中
signals:

public slots:

private:
    QString label;
    QList<QString> words;
    int score;
    WordTable *wordTable;
    QLabel *scoreLabel;
};

#endif // WORDLISTWIDGET_H
