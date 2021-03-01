#include "Cube.h"

#include <QHBoxLayout>

Cube::Cube(int i,int j,QWidget *parent) : QWidget(parent)
{
    this->i=i;
    this->j=j;
    label = new QLabel();
    label->setText("");
    label->setAlignment(Qt::AlignCenter);
    QFont font = label->font();
    font.setPointSize(20);
    label->setFont(font);

    this->setFixedSize(75, 75);
    this->setStyleSheet("background-color: white; border-radius: 15px; border: 2px solid");

    QHBoxLayout *layout = new QHBoxLayout();
    layout->addWidget(label);
    setLayout(layout);
}

void Cube::setLetter(QString l)
{
    label->setText(l);
}

void Cube::mousePressEvent(QMouseEvent *event){
    emit clicked(i,j);
}
