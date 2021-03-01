#ifndef BUFFER_H
#define BUFFER_H
#include <QObject>
#include <QWidget>
#include <QString>

class Console;

class Buffer:public QWidget{
    friend class Console;
    friend class Program;
private:
    struct node{
      int num;
      QString str;
      node *next=nullptr;
      node(int num,const QString str,node *next=nullptr){
          this->num=num;
          this->str=str;
          this->next=next;
      }
    };
    static const int MAX_LINE=512;
    node *head=nullptr;
    Console *console;

public:
    int lineNum=0;
    Buffer();
    ~Buffer();
    void writeTofile(const QString &filename)const;//save the code
    void readFile(const QString &filename);
    void printCode();                    //output all code
    void deleteLine(int lineNum);        //delete specified line of code
    void appendLine(const QString &text);//add one line of code to buffer
    void addLine(const QString &text);
    void clear();
};

#endif // BUFFER_H
