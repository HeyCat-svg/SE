#ifndef TOKENIZER_H
#define TOKENIZER_H
#include <QString>

class Program;
class Buffer;
class Statement;
class Expression;
class Parser;


class Tokenizer{//一行代码被分成三个部分：行号、statement和剩余的部分
public:
    int lineNum;
    QString statement;
    QString others;
    Expression *exp=nullptr;
    Statement *_statement=nullptr;
    static Parser *parser;
    bool flag=false; //标记代码是否要被忽视

    Tokenizer(QString code);
    void furtherTokenize();//将others根据statement的不同将操作数和操作符之间做加空格处理 以便sstream导出
};


#endif // TOKENIZER_H
