#ifndef EVALSTATE_H
#define EVALSTATE_H
#include <QString>
#include <string>

using namespace std;

class Console;
class EvalState{
private:
    struct node{
        QString evalName;
        int value;
        node *next;
        node(int val,QString str,node *next=nullptr){
            this->value=val;
            this->evalName=str;
            this->next=next;
        }
    };

    node *head;
    Console *console;
public:
    EvalState(Console *console);
    ~EvalState();
    void changeEval(int val,QString str);
    void delEval(QString str);
    int findEval(QString str);
    bool isDefined(QString str);
};

#endif // EVALSTATE_H
