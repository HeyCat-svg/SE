#include "console.h"
#include <QString>
#include <string>
#include <sstream>
#include <stack>

using namespace std;

bool Parser::checkLineNum(Tokenizer **code,int lineNum){
    for(int i=0;i<lineNum-1;++i){
        if(code[i]->lineNum>=code[i+1]->lineNum){
            return false;
        }
    }
    return true;
}

bool Parser::checkStatement(Tokenizer **code,int lineNum){
    for(int i=0;i<lineNum;++i){
        QString str=code[i]->statement;
        if(str!="REM"&&str!="PRINT"&&str!="LET"&&str!="IF"
                &&str!="END"&&str!="GOTO"&&str!="INPUT"
                &&str!="Sub"&&str!="END Sub"&&str!="CALL"){
            return false;
        }
    }
    return true;
}

bool Parser::checkVariable(){
    return true;
}

bool Parser::grammaCheck(){
    return true;
}

Expression *Parser::buildExp(QString expStr){
    try {

    string str;
    stack<CompoundExp *> operators;
    stack<Expression *> operands;
    stringstream ss(expStr.toStdString());

    while(!ss.eof()){
        ss>>str;
        if(('a'<=str[0]&&str[0]<='z')||('A'<=str[0]&&str[0]<='Z')){ //Identifier
            Expression *p=new IdentifierExp(QString::fromStdString(str));
            operands.push(p);
        }
        else if('0'<=str[0]&&str[0]<='9'){  //Constant
            Expression *p=new ConstantExp(stoi(str));
            operands.push(p);
        }
        else{   //Operator
            if(operators.empty()){
                CompoundExp *p=new CompoundExp(QString::fromStdString(str));
                operators.push(p);
            }
            else{
                QString tmp=operators.top()->getOperator();
                if(str=="*"||str=="/"){
                    if(tmp!="*"&&tmp!="/"){
                        CompoundExp *p=new CompoundExp(QString::fromStdString(str));
                        operators.push(p);
                    }
                    else{
                        CompoundExp *p1=operators.top();
                        operators.pop();
                        if(operands.empty())            //exception 1
                            throw "operands is empty.";
                        p1->rhs=operands.top();
                        operands.pop();
                        if(operands.empty())            //exception 2
                            throw "operands is empty.";
                        p1->lhs=operands.top();
                        operands.pop();
                        operands.push(p1);
                        CompoundExp *p=new CompoundExp(QString::fromStdString(str));
                        operands.push(p);
                    }
                }
                else if(str=="+"||str=="-"){
                    if(tmp=="="||tmp=="("){
                        CompoundExp *p=new CompoundExp(QString::fromStdString(str));
                        operators.push(p);
                    }
                    else{
                        while(!operators.empty()&&operators.top()->getOperator()!="="&&operators.top()->getOperator()!="("){
                            CompoundExp *p1=operators.top();
                            operators.pop();
                            if(operands.empty())            //exception 3
                                throw "operands is empty.";
                            p1->rhs=operands.top();
                            operands.pop();
                            if(operands.empty())            //exception 4
                                throw "operands is empty.";
                            p1->lhs=operands.top();
                            operands.pop();
                            operands.push(p1);
                        }
                        CompoundExp *p=new CompoundExp(QString::fromStdString(str));
                        operators.push(p);
                    }
                }
                else if(str=="("){  //这里没有考虑= 因为=是作为赋值 只能在第一个入operators栈
                    CompoundExp *p=new CompoundExp(QString::fromStdString(str));
                    operators.push(p);
                }
                else if(str==")"){
                    while(operators.top()->getOperator()!="("){
                        CompoundExp *p1=operators.top();
                        operators.pop();
                        if(operands.empty())            //exception 5
                            throw "operands is empty.";
                        p1->rhs=operands.top();
                        operands.pop();
                        if(operands.empty())            //exception 6
                            throw "operands is empty.";
                        p1->lhs=operands.top();
                        operands.pop();
                        operands.push(p1);
                    }
                    if(operators.empty())            //exception 7
                        throw "operators is empty.";
                    operators.pop();
                }
            }
        }
    }
    while(!operators.empty()){
        CompoundExp *p1=operators.top();
        operators.pop();
        if(operands.empty())            //exception 8
            throw "operands is empty.";
        p1->rhs=operands.top();
        operands.pop();
        if(operands.empty())            //exception 9
            throw "operands is empty.";
        p1->lhs=operands.top();
        operands.pop();
        operands.push(p1);
    }
    return operands.top();

    } catch (...) {
        throw "Illegal expression. Parser::buildExp()";
    }
}

int Parser::computeExp(Expression *exp,EvalState *evalstate){
    try {

    if(exp->getOperator()=="="){
        int result=computeExp(exp->getRHS(),evalstate);
        evalstate->changeEval(result,exp->getLHS()->getIdentifierName());
        return result;
    }
    else{
        if(exp->type()==COMPOUND){
            int rightVal=computeExp(exp->getRHS(),evalstate);
            int leftVal=computeExp(exp->getLHS(),evalstate);
            QString op=exp->getOperator();
            if(op=="+")
                return leftVal+rightVal;
            if(op=="-")
                return leftVal-rightVal;
            if(op=="*")
                return leftVal*rightVal;
            if(op=="/")
                return leftVal/rightVal;
        }
        if(exp->type()==CONSTANT){
            return exp->getConstantValue();
        }
        if(exp->type()==IDENTIFIER){
            int identifierVal=evalstate->findEval(exp->getIdentifierName());
            return identifierVal;
        }
    }

    } catch (...) {
        throw "Illegal expression. Parser::computeExp()";
    }
}

QString Parser::dealNegative(QString exp){
    QString formalStr;
    int start=0;
    int end=0;
    for(;end<exp.length();++end){
        if(exp[end]=='-'){
            if(end==0){
                formalStr="0-";
                start=end+1;
            }
            else{
                if(exp[end-1]=='('||exp[end-1]=='='){
                    formalStr=formalStr+exp.mid(start,end-start)+"0-";
                    start=end+1;
                }
            }
        }
    }
    formalStr=formalStr+exp.mid(start,exp.length()-start);
    return formalStr;
}
