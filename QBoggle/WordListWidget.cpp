#include "WordListWidget.h"


WordListWidget::WordListWidget(QWidget *parent, QString label) : QWidget(parent)
{
    this->score=0;
    this->words.clear();

    QVBoxLayout *layout = new QVBoxLayout();
    QHBoxLayout *headLayout = new QHBoxLayout();

    QLabel *nameLabel = new QLabel(this);
    scoreLabel = new QLabel(this);
    QFont font = nameLabel->font();
    font.setPointSize(20);
    nameLabel->setFont(font);
    scoreLabel->setFont(font);
    nameLabel->setText(label);
    scoreLabel->setText(QString::number(score));

    headLayout->addWidget(nameLabel, 0, Qt::AlignmentFlag::AlignLeft);
    headLayout->addWidget(scoreLabel, 0, Qt::AlignmentFlag::AlignRight);
    layout->addLayout(headLayout);

    QFrame *hline = new QFrame();
    hline->setFrameShape(QFrame::HLine);
    hline->setFrameShadow(QFrame::Sunken);
    layout->addWidget(hline);

    wordTable = new WordTable();
    layout->addWidget(wordTable);

    setLayout(layout);
}

void WordListWidget::addScore(int score)
{
    this->score += score;
    scoreLabel->setText(QString::number(this->score));

}
void WordListWidget::addWord(QString word)
{
    this->words.append(word);
    this->wordTable->addWord(word);
}
void WordListWidget::reset()
{
    this->score = 0;
    scoreLabel->setText(QString::number(this->score));
    this->words.clear();
    this->wordTable->clearContents();
}

bool WordListWidget::check(QString str){
    str=str.toLower();
    for(int i=0;i<words.length();++i){
        if(str==words.at(i))
            return true;
    }
    return false;
}

