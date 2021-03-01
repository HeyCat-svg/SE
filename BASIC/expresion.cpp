#include "console.h"
#include <QString>
#include <sstream>
#include <string>

using namespace std;

ConstantExp::ConstantExp(int val){
    this->value=val;
}

ExpressionType ConstantExp::type(){
    return CONSTANT;
}

int ConstantExp::getConstantValue(){
    return value;
}

IdentifierExp::IdentifierExp(QString name){
    this->name=name;
}

ExpressionType IdentifierExp::type(){
    return IDENTIFIER;
}

QString IdentifierExp::getIdentifierName(){
    return this->name;
}

CompoundExp::CompoundExp(QString op,Expression *lhs,Expression *rhs){
    this->op=op;
    this->lhs=lhs;
    this->rhs=rhs;
}

CompoundExp::~CompoundExp(){//这边可能会有问题
    if(lhs!=nullptr)
        lhs->~Expression();
    if(rhs!=nullptr)
        rhs->~Expression();
    delete this;

    /*if(lhs!=nullptr){
        if(lhs->type()!=COMPOUND)
            delete lhs;
        else
            lhs->~Expression();
    }
    if(rhs!=nullptr){
        if(rhs->type()!=COMPOUND)
            delete lhs;
        else
            lhs->~Expression();
    }*/
}

ExpressionType CompoundExp::type(){
    return COMPOUND;
}

QString CompoundExp::getOperator(){
    return op;
}

Expression *CompoundExp::getLHS(){
    return lhs;
}

Expression *CompoundExp::getRHS(){
    return rhs;
}
