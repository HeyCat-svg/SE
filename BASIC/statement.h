#ifndef STATEMENT_H
#define STATEMENT_H

#include <string>
#include <QString>
#include <vector>

enum StmtType{RemStmt,LetStmt,PrintStmt,InputStmt,GotoStmt,IfStmt,EndStmt,
              SubStmt,EndSubStmt,CallStmt};

class Console;
class Tokenizer;
class EvalState;
class Parser;
class Expression;
class Program;

class Statement{
public:
    Statement(){}
    virtual ~Statement(){}
    virtual StmtType getType(){return RemStmt;}
    virtual void operation(){}
};

class RemStatement:public Statement{
public:
    StmtType getType();
    void operation();
};

class LetStatement:public Statement{
public:
    LetStatement(Tokenizer *token,EvalState *evalState,Parser *parser);
    StmtType getType();
    void operation();

private:
    Tokenizer *token=nullptr;
    EvalState *evalState=nullptr;
    Parser *parser=nullptr;
};

class PrintStatement:public Statement{
public:
    PrintStatement(Tokenizer *token,EvalState *evalState,Parser *parser,Console *console);
    StmtType getType();
    void operation();

private:
    Tokenizer *token=nullptr;
    EvalState *evalState=nullptr;
    Parser *parser=nullptr;
    Console *console=nullptr;
};

class InputStatement:public Statement{
    //输入的实现是在program类中加入一个flag标记是否处于input模式
    //这边只需要改flag 具体赋值交给program
    //另外没执行一行代码前要检查flag值 true为挂起
public:
    InputStatement(Tokenizer *token,Console *console,bool *flag,QString *name);
    StmtType getType();
    void operation();

private:
    Tokenizer *token=nullptr;
    Console *console=nullptr;
    bool *flag=nullptr;
    QString *name=nullptr;
};

class GotoStatement:public Statement{
public:
    GotoStatement(int *liineNum,bool *flag,Tokenizer *token);
    StmtType getType();
    void operation();

private:
    int *lineNum=nullptr;
    bool *flag=nullptr;
    Tokenizer *token=nullptr;
};

class IfStatement:public Statement{
public:
    IfStatement(int *lineNum,bool *flag,Tokenizer *token,EvalState *evalState, Parser *parser);
    StmtType getType();
    void operation();

private:
    int *lineNum=nullptr;
    bool *flag=nullptr;
    Tokenizer *token=nullptr;
    EvalState *evalState=nullptr;
    Parser *parser=nullptr;

    int exp1Start=0,exp1End=0,exp2Start=0,exp2End=0,op=0,num=0;
    QString str;
    Expression *exp1=nullptr,*exp2=nullptr;

};

class EndStatement:public Statement{
public:
    EndStatement(bool *flag);   //flag来自program类 结束则为true 外界循环break
    StmtType getType();
    void operation();

private:
    bool *flag=nullptr;
};

class SubStatement:public Statement{
public:
    SubStatement(Tokenizer *token,Program *program);
    StmtType getType();
    void operation();

private:
    Tokenizer *token=nullptr;
    Program *program=nullptr;
};

class EndSubStatement:public Statement{
public:
    EndSubStatement(Program *program);
    StmtType getType();
    void operation();

private:
    Program *program=nullptr;
};

class CallStatement:public Statement{
public:
    CallStatement(Program *program,Tokenizer *token);
    StmtType getType();
    void operation();

private:
    Program *program=nullptr;
    Tokenizer *token =nullptr;
};

#endif // STATEMENT_H
