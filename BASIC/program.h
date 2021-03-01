#ifndef PROGRAM_H
#define PROGRAM_H
#include <QObject>
#include <QWidget>
#include <QString>
#include <stack>
#include <vector>
#include <map>
#include "tokenizer.h"
#include "parser.h"
#include "statement.h"
#include "expresion.h"
#include "evalstate.h"

class Console;
class Buffer;
class Tokenizer;
class EvalState;

/*Program类中应包含tokenizer 用来将只读code的代码分解成：statement+expression
 * 另外要设置statement类，用来提供工具（输入分解好的token，做出对应的操作）
 * exoression类按照行号构建树状结构并提供计算方法。parser类负责在tokenize之后判断
 * 语法是否正确，evalstate类储存程序运行中创建的变量
 *
 * 语法正确包括statement拼写、行号有无重复、有无没有经过定义的变量出现（）
 * 参数是否正确放在statement里具体判断
 */
class Program{
    friend class Console;
    friend class Tokenizer;

private:
    int  lineNum=0; //总行数
    int pointer=0;  //当前运行到的行数
    bool hang=false;//是否挂起
    bool end=false; //程序是否结束
    bool pointerIsChanged=false;//pointer是否被改变过，即行号是否发生转跳
    QString name;  //name表示从input函数中传出来的变量名
    QString *codeStr;
    Tokenizer **code;
    static Parser *parser;
    EvalState *evalState;
    Console *console;
    stack<int> return_addr;  //函数返回地址
    map<QString,int> funList;   //存放函数名和所在行号


public:
    Program(Buffer *buffer,Console *console);
    void run(); //跑程序
    int searchLineNum(int line);
    void input(int value,QString name);
    void addFun(QString name,int addr); //加入函数名和地址 并且将pointer跳转到subEnd后
    void funReturn();   //用返回栈返回上一个调用函数的地方
    void callFun(QString funName,int currAddr=-1);  //搜索funName所在行数，将下一行行数入栈
};

#endif // PROGRAM_H
