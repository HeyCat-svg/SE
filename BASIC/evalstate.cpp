#include "console.h"

EvalState::EvalState(Console *console){
    head=new node(0,"null");
    this->console=console;
}

EvalState::~EvalState(){
    node *tmp;
    while(head!=nullptr){
        tmp=head;
        head=head->next;
        delete tmp;
    }
}

void EvalState::changeEval(int val,QString str){
    if(this->isDefined(str)){
        node *p=head->next;
        for(;p->evalName!=str&&p!=nullptr;p=p->next);
        p->value=val;
    }
    else{
        head->next=new node(val,str,head->next);
    }

}

void EvalState::delEval(QString str){
    bool flag=false;
    for(node *p=head;p->next!=nullptr;p=p->next){
        if(p->next->evalName==str){
            node *tmp=p->next;
            p->next=p->next->next;
            delete tmp;
            flag=true;
            break;
        }
    }
    if(!flag){
        throw "No such variable. Evalstate::delEval()";
    }
}

int EvalState::findEval(QString str){   //遇到没有定义的变量的报错还要再优化一下

    for(node *p=head;p->next!=nullptr;p=p->next){
        if(p->next->evalName==str){
            return p->next->value;
        }
    }

    throw "No such variable. Evalstate::findEval()";
}

bool EvalState::isDefined(QString str){
    node *p=head->next;
    for(;p!=nullptr;p=p->next){
        if(str==p->evalName)
            return true;
    }
    return false;
}
