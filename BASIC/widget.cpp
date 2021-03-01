#include "widget.h"

Widget::Widget(QWidget *parent)
    : QWidget(parent)
{
    this->setWindowTitle("BASIC");
    this->setFixedSize(800,550);

    console=new Console(this);
    console->setGeometry(0,0,800,550);
    console->write("Minimal BASIC -- Type \"HELP\" for help.");
    console->write("\n");
}

Widget::~Widget(){}
