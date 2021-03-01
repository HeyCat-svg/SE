#include "BoggleWindow.h"

#include <QFile>
#include <QHBoxLayout>
#include <QTextEdit>
#include <iostream>

BoggleWindow::BoggleWindow(QWidget *parent)
    : QMainWindow(parent)
{
    this->setWindowTitle("QBoggle!");
    this->setFixedSize(BOGGLE_WINDOW_WIDTH, BOGGLE_WINDOW_HEIGHT);

    me = new WordListWidget(this, "Me");
    computer = new WordListWidget(this, "Computer");
    board = new Board(this);
    console = new Console(this);

    me->setGeometry(20, 20, 230, 300);
    board->setGeometry(230, 0, 300, 300);
    computer->setGeometry(800 - 50 - 200, 20, 230, 300);
    console->setGeometry(30, 320, 740, 260);

    connect(console,SIGNAL(newLineWritten(QString)),this,SLOT(receiveWords(QString)));
    for(int i=0;i<board->size*board->size;++i){
        connect(board->cubes[i],SIGNAL(clicked(int,int)),this,SLOT(dealMouseEvent(int,int)));
    }


    QFile qFile(":/res/EnglishWords.txt");
    if (!qFile.open(QIODevice::ReadOnly)) {
        throw new std::runtime_error("Resource file not found!");
    }

    /*Lexicon lex(qFile);*/

    lex=new Lexicon(qFile);

    /*
    for (std::string s: lex) {
        std::cout << s << std::endl;
    }
    */
    console->write("Welcome to the game Boggle!\n"
                   "Do you like to customize the board?(Y/N)\n");
}

BoggleWindow::~BoggleWindow()
{
}

void BoggleWindow::timerEvent(QTimerEvent *event){
    if(event->timerId()==timer){
        for(int i=0;i<board->size*board->size;++i)
            if(board->word[i])
                board->cubes[i]->setStyleSheet("background-color: white; border-radius: 15px; border: 2px solid");
        killTimer(timer);
    }
}
void BoggleWindow::searchByComputer(int pos){
    if(pos==-1){
        for(int i=0;i<board->size*board->size;++i){
            if(board->book[i]==false){
                str=str+board->verList[i].ch;
                board->book[i]=true;
                std::string s=str.toStdString();
                if(str.length()>3&&!computer->check(str)&&!me->check(str)&&lex->contains(s)){
                    QString tmp=str.toLower();//输出转小写
                    computer->addScore(tmp.length()-3);//需要查阅文档看单词长度和得分关系
                    computer->addWord(tmp);
                }//如果str在词典中，检测str是否在用户单词表中，Y则无视N则加入电脑单词表
                if(lex->containsPrefix(s))
                    searchByComputer(i);
                str=str.left(str.length()-1);
                board->book[i]=false;
            }
        }
    }
    else{
        for(Board::edgeNode *p=board->verList[pos].head;p!=NULL;p=p->next){
            if(board->book[p->end]==false){
                str=str+board->verList[p->end].ch;
                board->book[p->end]=true;
                std::string s=str.toStdString();
                if(str.length()>3&&!computer->check(str)&&!me->check(str)&&lex->contains(s)){
                    QString tmp=str.toLower();//输出转小写
                    computer->addScore(tmp.length()-3);//需要查阅文档看单词长度和得分关系
                    computer->addWord(tmp);
                }//方法同上
                if(lex->containsPrefix(s))
                    searchByComputer(p->end);
                str=str.left(str.length()-1);
                board->book[p->end]=false;
            }
        }
    }
}

void BoggleWindow::searchByHuman(QString str){
    str=str.toUpper();//输入字符串转大写
    std::string s=str.toStdString();
    if(str.length()>3&&!me->check(str)&&board->search(str)&&lex->contains(s)){
        str=str.toLower();//输出字符转小写
        me->addScore(str.length()-3);
        me->addWord(str);
        highlight(1000);

        /*for(int i=0;i<5;++i){
            QString tet="";
            for(int j=0;j<5;++j){
                tet=tet+QString::number(board->word[i*5+j]);
                tet=tet+" ";
            }
            console->write(tet);
        }*/
    }
    else
        console->write("Wrong Word!");
}

void BoggleWindow::highlight(int time){
    for(int i=0;i<board->size*board->size;++i)
        if(board->word[i])
            board->cubes[i]->setStyleSheet("background-color: green; border-radius: 15px; border: 2px solid");
    timer=startTimer(time);
}

void BoggleWindow::receiveWords(QString str){
    switch(status){
    case 0:{
        if (str=="Y"||str=="y"){
            status=1;
            console->write("Please enter at least 25 characters.\n");
            return;
        }
        else if(str=="N"||str=="n"){
            status=2;
            console->write("=========================================\n"
                           "Command List:\n"
                           "_\t:played by computer\n"
                           "reset -a\t:reset all\n"
                           "reset_\t:reset without changing board\n"
                           "=========================================\n");
            return;
        }else{
            console->write("Please enter Y/N.\n");
            return;
        }
    }
    case 1:{
        if(str.length()<board->size*board->size){
            console->write("Not enough characters. Please enter again.\n");
            return;
        }
        else{
            str=str.toUpper();
            for(int i=0;i<board->size*board->size;++i){
                board->letters[i][0]=str.at(i);
                board->cubes[i]->setLetter(str.at(i));
            }
            board->clearGraph();
            board->buildGraph();
            status=3;
            console->write("=========================================\n"
                           "Command List:\n"
                           "_\t:played by computer\n"
                           "reset -a\t:reset all\n"
                           "reset_\t:reset without changing board\n"
                           "=========================================\n");
            return;
        }
    }
    case 2:
    case 3:{
            if(str=="_"){
                searchByComputer();
                return;
            }
            if(str=="reset -a"){
                me->reset();
                computer->reset();
                console->_clear();
                board->shake();
                board->clearGraph();
                board->buildGraph();
                for(int i=0;i<board->size*board->size;++i){
                    board->cubes[i]->setLetter(board->letters[i].at(0));
                }
                status=0;
                console->write("Welcome to the game Boggle!\n"
                               "Do you like to customize the board?(Y/N)\n");
                return;
            }
            if(str=="reset_"){
                me->reset();
                computer->reset();
                console->_clear();
                console->write("Welcome to the game Boggle!\n"
                               "=========================================\n"
                               "Command List:\n"
                               "_\t:played by computer\n"
                               "reset -a\t:reset all\n"
                               "reset_\t:reset without changing board\n"
                               "=========================================\n");
                status=2;
                return;
            }
            searchByHuman(str);
        }
    }
}

void BoggleWindow::dealMouseEvent(int i,int j){
    if((row==-1&&col==-1)||(i>=row-1&&i<=row+1&&j>=col-1&&j<=col+1&&!board->cubes[board->size*i+j]->flag)){
        row=i;
        col=j;
        board->cubes[board->size*i+j]->flag=true;
        board->cubes[board->size*i+j]->setStyleSheet("background-color: green; border-radius: 15px; border: 2px solid");
        str2=str2+board->verList[board->size*i+j].ch;
        std::string s=str2.toStdString();
        if(str2.length()>3&&lex->contains(s)&&!me->check(str2)){
            QString tmp=str2.toLower();
            me->addScore(tmp.length()-3);
            me->addWord(tmp);
            //重置状态
            for(int k=0;k<board->size*board->size;++k){
                board->cubes[k]->flag=false;
                board->cubes[k]->setStyleSheet("background-color: white; border-radius: 15px; border: 2px solid");
            }
            row=-1;col=-1;str2="";
            return;
        }
        if(!lex->containsPrefix(s)){
            for(int k=0;k<board->size*board->size;++k){
                board->cubes[k]->flag=false;
                board->cubes[k]->setStyleSheet("background-color: white; border-radius: 15px; border: 2px solid");
            }
            row=-1;col=-1;str2="";
            return;
        }
    }
}
