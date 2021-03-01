#include "console.h"
#include <QString>
#include <string>
#include <sstream>

Parser *Program::parser=new Parser();
Program::Program(Buffer *buffer,Console *console){
    Buffer::node *p=buffer->head;

    this->lineNum=buffer->lineNum;
    this->console=console;
    this->codeStr=new QString[lineNum];
    this->code=new Tokenizer*[lineNum];
    this->evalState=new EvalState(console);

    for(int i=0;i<lineNum&&p!=nullptr;++i,p=p->next){
        codeStr[i]=p->str;
        code[i]=new Tokenizer(p->str);   
    }
    //这行应该先检查statement的语法 再furtherTokenize
    if(!parser->checkLineNum(code,lineNum)){
        console->write("Wrong sequence of code line.");
        end=true;
    }
    if(!parser->checkStatement(code,lineNum)){
        console->write("Can not identify the statement.");
        end=true;
    }
    for(int i=0;i<lineNum;++i){
        code[i]->furtherTokenize();
    }
}

void Program::run(){
    try {

    while(true){
        if(end||pointer>=lineNum){
            pointer=0;
            hang=false;
            break;
        }
        else if(hang){
            break;
        }
        else{
            if(code[pointer]->statement=="LET"){
                if(code[pointer]->_statement==nullptr)
                    code[pointer]->_statement=new LetStatement(code[pointer],evalState,parser);
                code[pointer]->_statement->operation();
            }
            if(code[pointer]->statement=="PRINT"){
                if(code[pointer]->_statement==nullptr)
                    code[pointer]->_statement=new PrintStatement(code[pointer],evalState,parser,console);
                code[pointer]->_statement->operation();
            }
            if(code[pointer]->statement=="INPUT"){
                if(code[pointer]->_statement==nullptr)
                    code[pointer]->_statement=new InputStatement(code[pointer],console,&hang,&name);
                code[pointer]->_statement->operation();
            }
            if(code[pointer]->statement=="GOTO"){
                if(code[pointer]->_statement==nullptr)
                    code[pointer]->_statement=new GotoStatement(&pointer,&pointerIsChanged,code[pointer]);
                code[pointer]->_statement->operation();
                if(pointerIsChanged){
                    pointer=searchLineNum(pointer);
                    pointerIsChanged=false;
                    continue;   //此操作另外改变了pointer，所以不需要++pointer
                }
            }
            if(code[pointer]->statement=="IF"){
                if(code[pointer]->_statement==nullptr)
                    code[pointer]->_statement=new IfStatement(&pointer,&pointerIsChanged,code[pointer],evalState,parser);
                code[pointer]->_statement->operation();
                if(pointerIsChanged){
                    pointer=searchLineNum(pointer);
                    pointerIsChanged=false;
                    continue;   //此操作另外改变了pointer，所以不需要++pointer
                }
            }
            if(code[pointer]->statement=="Sub"){
                if(code[pointer]->_statement==nullptr)
                    code[pointer]->_statement=new SubStatement(code[pointer],this);
                code[pointer]->_statement->operation();
                continue;   //此操作另外改变了pointer，所以不需要++pointer
            }
            if(code[pointer]->statement=="END Sub"){
                if(code[pointer]->_statement==nullptr)
                    code[pointer]->_statement=new EndSubStatement(this);
                code[pointer]->_statement->operation();
                continue;   //此操作另外改变了pointer，所以不需要++pointer
            }
            if(code[pointer]->statement=="CALL"){
                if(code[pointer]->_statement==nullptr)
                    code[pointer]->_statement=new CallStatement(this,code[pointer]);
                code[pointer]->_statement->operation();
                continue;   //此操作另外改变了pointer，所以不需要++pointer
            }
            if(code[pointer]->statement=="END"){
                if(code[pointer]->_statement==nullptr)
                    code[pointer]->_statement=new EndStatement(&end);
                code[pointer]->_statement->operation();
            }
            pointer++;
        }
    }

    } catch (const char* e) {
        QString str(e);
        console->write(str);
        end=true;
    } catch(...){
        console->write("Unknown error. Program::run().");
        end=true;
    }
}

int Program::searchLineNum(int line){
    for(int i=0;i<lineNum;++i){
        if(code[i]->lineNum==line){
            return i;
        }
    }
    console->write("Can't jump to this line.(No such lineNum)");
    end=true;
    return -1;
}

void Program::input(int value,QString name){
    evalState->changeEval(value,name);
    hang=false;
}

void Program::addFun(QString name,int addr){
    int tmp=searchLineNum(addr)+1;  //tmp是Sub语句的下一行在code[]中的实际位置
    map<QString,int>::iterator iter=funList.find(name);
    if(iter!=funList.end()){
        console->write("redefinition of the function \""+name+"\"");
        end=true;
        return;
    }
    funList[name]=tmp;
    for(int i=tmp;i<lineNum;++i){
        if(code[i]->statement=="END Sub"){
            pointer=i+1;
            return;
        }
    }
    console->write("Can't find responsive END Sub.");
}

void Program::funReturn(){
    if(!return_addr.empty()){
        pointer=return_addr.top();
        return_addr.pop();
    }
    else{
        console->write("Stack is empty.");
        end=true;
    }
}

void Program::callFun(QString funName, int currAddr){
    map<QString,int>::iterator iter=funList.find(funName);
    if(iter!=funList.end()){
        pointer=funList[funName];
    }
    else{
        console->write("The function hasn't been definited.");
        end=true;
        return;
    }
    int nextAddr=searchLineNum(currAddr)+1;
    return_addr.push(nextAddr);
}
