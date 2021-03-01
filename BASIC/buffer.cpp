#include "console.h"
#include <sstream>
#include <QFile>
#include <QTextStream>

using namespace std;

Buffer::Buffer() {}

Buffer::~Buffer() {}

void Buffer::writeTofile(const QString &filename) const{
    QFile file("./"+filename);
    if(!file.open(QIODevice::WriteOnly|QIODevice::Text)){
        console->write("error! Fliename is unspecified.");
        return;
    }
    QTextStream in(&file);
    for(node *p=head;p!=nullptr;p=p->next){
        in<<p->str<<"\n";
    }
    file.flush();
    file.close();
    console->write("All Code is stored to "+filename);
}

void Buffer::readFile(const QString &filename){
    QFile file("./"+filename);
    if(!file.open(QIODevice::ReadOnly|QIODevice::Text)){
        console->write("error! Fliename is unspecified.");
        return;
    }
    QTextStream in(&file);
    while(!in.atEnd()){
        QString line=in.readLine();
        appendLine(line);
    }

}

void Buffer::printCode(){
    for(node *p=head;p!=nullptr;p=p->next)
        console->write(p->str);
}

void Buffer::deleteLine(int lineNum){
    node *p, *pre;
    pre=head;
    for(p= head;p!=nullptr&&p->num!=lineNum;pre=p,p=p->next);
    if(p==nullptr){
        console->write("No such line in code.");
        return;
    }
    else if(p==head){
        head=head->next;
        delete p;
    }else{
        pre->next=pre->next->next;
        delete p;
    }
    this->lineNum--;
}

void Buffer::appendLine(const QString &text){
    int num;
    stringstream ss(text.toStdString());
    ss>>num;
    for(node *p=head;p!=nullptr;p=p->next){
        if(p->num==num){
            deleteLine(num);
            addLine(text);
            return;
        }
    }
    addLine(text);
}

void Buffer::addLine(const QString &text){
    int num;
    stringstream ss(text.toStdString());
    ss>>num;
    if(head==nullptr||head->num>num){
        this->head=new node(num,text,head);
    }
    else{
        for(node *p=head;p!=nullptr;p=p->next){
            if(p->next==nullptr||p->next->num>num){
                p->next=new node(num,text,p->next);
                break;
            }
        }
    }
    lineNum++;
}

void Buffer::clear(){
    node *p=head;
    while(p!=nullptr){
        node *tmp=p;
        p=p->next;
        delete tmp;
    }
    head=nullptr;
}
