#include "console.h"
#include <sstream>
#include <string>
using namespace std;

Parser *Tokenizer::parser=Program::parser;
Tokenizer::Tokenizer(QString code){
    string str,str1;
    stringstream ss(code.toStdString());
    ss>>lineNum;
    ss>>str;        //statement
    statement=QString::fromStdString(str);
    while(!ss.eof()){
        ss>>str1;   //others
        others=others+QString::fromStdString(str1);
    }
}

void Tokenizer::furtherTokenize(){
    if(this->statement=="REM"){
        flag=true;
        return;
    }
    if(this->statement=="END"){
        if(this->others=="Sub"){
            this->statement="END Sub";
            return;
        }
        return;
    }
    if(this->statement=="LET"||this->statement=="PRINT"){//得保证others最后一位不是空格
        others=parser->dealNegative(others);
        QString formalStr;//承载最终加入空格的语句
        int start,end,length;
        start=end=0;
        length=others.length();
        for(;end<length;++end){
            if(others[end]=='+'||others[end]=='-'||others[end]=='*'
                    ||others[end]=='/'||others[end]=='='
                    ||others[end]=='('||others[end]==')'){//有问题 明天调试
                QString tmp=others.mid(start,end-start);
                formalStr=formalStr+tmp+" "+others.mid(end,1)+" ";
                start=end+1;
            }
        }
        if(start==length)
            formalStr=formalStr.mid(0,formalStr.length()-1);
        else
            formalStr=formalStr+others.mid(start,length-start);
        others=formalStr;
        return;
    }
    if(this->statement=="IF"){
        others=parser->dealNegative(others);
        QString formalStr;
        int start,end,length;
        start=end=0;
        length=others.length();
        for(;end<length;++end){
            if(others[end]=='<'||others[end]=='>'||others[end]=='='
                   ||others[end]=='+'||others[end]=='-'||others[end]=='*'
                   ||others[end]=='/'||others[end]=='('||others[end]==')'){
                QString tmp=others.mid(start,end-start);
                formalStr=formalStr+tmp+" "+others.mid(end,1)+" ";
                start=end+1;
            }
            if(end<=length-4&&others.mid(end,4)=="THEN"){
                QString tmp=others.mid(start,end-start);
                formalStr=formalStr+tmp+" "+others.mid(end,4)+" ";
                start=end+4;
            }
        }
        formalStr=formalStr+others.mid(start,length-start);
        others=formalStr;
        return;
    }
}
