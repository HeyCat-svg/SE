#ifndef PARSER_H
#define PARSER_H
#include <QString>

class Console;
class Tokenizer;
class Expression;
class EvalState;

class Parser{
public:
    bool checkLineNum(Tokenizer **code,int lineNum);//返回true则表示正常
    bool checkStatement(Tokenizer **code,int lineNum);
    bool checkVariable();//参数放evalstate类
    bool grammaCheck();

    Expression *buildExp(QString expStr);
    int computeExp(Expression *exp,EvalState *evalstate);
    QString dealNegative(QString exp);
};
#endif // PARSER_H
