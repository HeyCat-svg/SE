#include "console.h"

StmtType RemStatement::getType(){
    return RemStmt;
}

void RemStatement::operation(){
    return;
}

LetStatement::LetStatement(Tokenizer *token,EvalState *evalState,Parser *parser){
    this->token=token;
    this->evalState=evalState;
    this->parser=parser;
}

StmtType LetStatement::getType(){
    return LetStmt;
}

void LetStatement::operation(){
    if(token->exp==nullptr)
        token->exp=parser->buildExp(token->others);
    parser->computeExp(token->exp,evalState);
}

PrintStatement::PrintStatement(Tokenizer *token,EvalState *evalState,Parser *parser,Console *console){
    this->token=token;
    this->evalState=evalState;
    this->parser=parser;
    this->console=console;
}

StmtType PrintStatement::getType(){
    return PrintStmt;
}

void PrintStatement::operation(){
    if(token->exp==nullptr)
        token->exp=parser->buildExp(token->others);
    int value=parser->computeExp(token->exp,evalState);
    console->write(QString::number(value));

    /*ExpressionType type=token->exp->type();
    if(type==CONSTANT){
        console->write(QString::number(token->exp->getConstantValue()));
    }
    if(type==IDENTIFIER){
        QString name=token->exp->getIdentifierName();
        if(evalState->isDefined(name)){
            int value=evalState->findEval(name);
            console->write(QString::number(value));
        }
        else{
            this->console->write("Error! No such variable.");
        }
    }
    if(type==COMPOUND){
        int value=parser->computeExp(token->exp,evalState);
        console->write(QString::number(value));
    }*/
}


InputStatement::InputStatement(Tokenizer *token,Console *console,bool *flag,QString *name){
    this->token=token;
    this->console=console;
    this->flag=flag;
    this->name=name;
}

StmtType InputStatement::getType(){
    return InputStmt;
}

void InputStatement::operation(){
    console->write(" ? ");  //先不考虑输入文本要和？在同一行
    *flag=true;
    *name=token->others;
}

GotoStatement::GotoStatement(int *lineNum,bool *flag,Tokenizer *token){
    this->lineNum=lineNum;
    this->token=token;
    this->flag=flag;
}

StmtType GotoStatement::getType(){
    return GotoStmt;
}

void GotoStatement::operation(){
    int num=token->others.toInt();
    *lineNum=num;
    *flag=true;
}

IfStatement::IfStatement(int *lineNum,bool *flag,Tokenizer *token,EvalState *evalState, Parser *parser){
    this->lineNum=lineNum;
    this->parser=parser;
    this->evalState=evalState;
    this->token=token;
    this->flag=flag;
}

StmtType IfStatement::getType(){
    return IfStmt;
}

void IfStatement::operation(){
    if(op==0){
        str=token->others.simplified();
        for(int i=0;i<str.length();++i){
            if(str[i]=='>'||str[i]=='<'||str[i]=='='){
                exp1End=i-1;    //真正字符的后一位
                exp2Start=i+2;  //式2开头所在的字符
                op=i;
            }
            if(i<=str.length()-4&&str.mid(i,4)=="THEN"){
                exp2End=i-1;    //真正字符的后一位
                num=i+5;
            }
        }
        QString intnum=str.mid(num);
        num=intnum.toInt();     //num现在为转跳的行数
        exp1=parser->buildExp(str.mid(exp1Start,exp1End-exp1Start));
        exp2=parser->buildExp(str.mid(exp2Start,exp2End-exp2Start));
    }

    int result1=parser->computeExp(exp1,evalState);
    int result2=parser->computeExp(exp2,evalState);
    if(str[op]==">"){
        if(result1>result2){
            *lineNum=num;  //lineNum是code[]的index，num是行号，需要转换
            *flag=true;
        }
        else
            return;
    }
    if(str[op]=="<"){
        if(result1<result2){
            *lineNum=num;
            *flag=true;
        }
        else
            return;
    }
    if(str[op]=="="){
        if(result1==result2){
            *lineNum=num;
            *flag=true;
        }
        else
            return;
    }
}

EndStatement::EndStatement(bool *flag){
    this->flag=flag;
}

StmtType EndStatement::getType(){
    return EndStmt;
}

void EndStatement::operation(){
    *flag=true;
}

SubStatement::SubStatement(Tokenizer *token,Program *program){
    this->token=token;
    this->program=program;
}

StmtType SubStatement::getType(){
    return SubStmt;
}

void SubStatement::operation(){
    program->addFun(token->others,token->lineNum);
}

EndSubStatement::EndSubStatement(Program *program){
    this->program=program;
}

StmtType EndSubStatement::getType(){
    return EndSubStmt;
}

void EndSubStatement::operation(){
    program->funReturn();
}

CallStatement::CallStatement(Program *program,Tokenizer *token){
    this->program=program;
    this->token=token;
}

StmtType CallStatement::getType(){
    return CallStmt;
}

void CallStatement::operation(){
    program->callFun(token->others,token->lineNum);
}
