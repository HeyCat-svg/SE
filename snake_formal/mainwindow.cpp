#include"mainwindow.h"


MainWindow::MainWindow(QWidget *parent):
    QWidget(parent)
{
    setWindowTitle("Snake");
    resize(600,600);
    setStyleSheet("QWidget{background:white}");
    setWindowOpacity(0.8);

    singlePlayer=new QPushButton(this);
    singlePlayer->setText("Single Player");
    singlePlayer->setGeometry(250,150,100,50);
    connect(singlePlayer,&QPushButton::released,this,&MainWindow::buttonSingle);

    doublePlayer=new QPushButton(this);
    doublePlayer->setText("Double Players");
    doublePlayer->setGeometry(250,250,100,50);
    connect(doublePlayer,&QPushButton::released,this,&MainWindow::buttonDouble);

    autoPlay=new QPushButton(this);
    autoPlay->setText("Auto Mode");
    autoPlay->setGeometry(250,350,100,50);
    connect(autoPlay,&QPushButton::released,this,&MainWindow::buttonAuto);


    quit=new QPushButton(this);
    quit->setText("Quit");
    quit->setGeometry(250,450,100,50);
    connect(quit,SIGNAL(clicked()),this,SLOT(buttonQuit()));
}

void MainWindow::buttonSingle(){
    this->hide();
    subWidget=new Widget(1);
    connect(subWidget,&Widget::backSignal,this,&MainWindow::dealBackSignal);
    subWidget->show();
}

void MainWindow::buttonDouble(){
    this->hide();
    subWidget=new Widget(2);
    connect(subWidget,&Widget::backSignal,this,&MainWindow::dealBackSignal);
    subWidget->show();
}

void MainWindow::buttonAuto(){
    this->hide();
    subWidget=new Widget(1);
    connect(subWidget,&Widget::backSignal,this,&MainWindow::dealBackSignal);
    subWidget->_autoMove[0]=true;
    subWidget->show();
}

void MainWindow::buttonQuit(){
    this->close();
}

void MainWindow::dealBackSignal(){
    subWidget->hide();
    delete subWidget;
    this->show();
}
