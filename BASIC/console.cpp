#include "console.h"
#include <QKeyEvent>
#include <QTextLine>
#include <QTextCursor>
#include <sstream>

using namespace std;

Console::Console(QWidget *parent):QTextEdit(parent){
    buffer=new Buffer();
    buffer->console=this;
    this->setFont(QFont(tr("Consolas"), 14));
    connect(this,SIGNAL(newLineWritten(const QString)),this,SLOT(dispatchCmd(const QString)));
}

Console::~Console(){
    delete buffer;
}

void Console::write(QString msg){
    this->append(msg);
}

void Console::keyPressEvent(QKeyEvent *event){
    QTextCursor cursor = this->textCursor();
    cursor.movePosition(QTextCursor::End);
    setTextCursor(cursor);
    ensureCursorVisible();

    if (event->key() == Qt::Key_Backspace){//控制在当前行编辑
        if(cursor.atBlockStart())
            return;
    }
    if (event->key() == Qt::Key_Delete){
        if(cursor.atBlockStart())
            return;
    }
    if (this->textCursor().hasSelection())
        return;
    if (event->key() == Qt::Key_Return&&!cursor.atBlockStart()) {
        QTextCursor cursor = this->textCursor();
        cursor.movePosition(QTextCursor::End);
        cursor.select(QTextCursor::LineUnderCursor);
        QString lastLine = cursor.selectedText();
        newLineWritten(lastLine);
    }
    QTextEdit::keyPressEvent(event);
}

void Console::dispatchCmd(const QString &cmd){
    if(program!=nullptr&&program->hang){
        QString str=cmd.simplified();
        for(int i=0;i<str.length();++i){
            if(cmd[i]<'0'||cmd[i]>'9'){
                this->write("Wrong input.");
                this->write(" ? ");
                return;
            }
        }
        int value=str.toInt();
        program->input(value,program->name);
        program->run();
        return;
    }
    if('0'<=cmd[0]&&cmd[0]<='9'){//对于输入如果为数字开头的，则判定为代码然后加入buffer
        buffer->appendLine(cmd);
        return;
    }
    if(cmd[0]=='s'&&cmd[1]==' '){//save format: s filename.txt
        QString filename;
        filename=cmd.mid(2); //suffix should be included into the filename
        cmdSave(filename);
        return;
    }
    if(cmd[0]=='l'&&cmd[1]==' '){//load format: l filename.txt
        QString filename;
        filename=cmd.mid(2);
        cmdLoad(filename);
        return;
    }
    if(cmd[0]=='d'&&cmd[1]==' '){//delete format: d lineNum
        QString str=cmd.mid(2);
        stringstream ss(str.toStdString());
        int lineNum;
        if(ss.good()){
            ss>>lineNum;
            cmdDelete(lineNum);
        }
        return;
    }
    if(cmd.length()>6&&cmd.mid(0,5)=="PRINT"){
        QString expresion=cmd.mid(6);
        expresion=expresion.simplified();
        cmdPrint(expresion);
        return;
    }
    if(cmd=="RUN"){
        cmdRun();
        return;
    }
    if(cmd=="LIST"){
        cmdList();
        return;
    }
    if(cmd=="CLEAR"){
        cmdClear();
        return;
    }
    if(cmd=="HELP"){
        cmdHelp();
        return;
    }
    this->write("Bad/Unknown command");
}

void Console::cmdDelete(int lineNum){
    buffer->deleteLine(lineNum);
}

void Console::cmdSave(const QString &filename){
    buffer->writeTofile(filename);
}

void Console::cmdLoad(const QString &filename){
    buffer->readFile(filename);
}

void Console::cmdPrint(const QString &expression){
    try {

    QString code="1";
    code=code+" PRINT "+expression;
    Expression *exp;
    Parser *parser=new Parser();
    Tokenizer *token=new Tokenizer(code);
    if(!program){
        for(int i=0;i<expression.length();++i){
            QChar ch=expression[i];
            if((ch>='0'&&ch<='9')||ch=='+'||ch=='-'||ch=='*'
                    ||ch=='/'||ch==' '||ch=='('||ch==')')
                continue;
            else{
                this->write("Illegal character in the expression.");
                return;
            }
        }
        token->furtherTokenize();
        exp=parser->buildExp(token->others);
        int result=parser->computeExp(exp,nullptr);
        this->write(QString::number(result));
        return;
    }
    else{
        token->furtherTokenize();
        exp=parser->buildExp(token->others);
        int result=parser->computeExp(exp,program->evalState);
        this->write(QString::number(result));
        return;
    }

    } catch (const char* e) {
        QString str(e);
        this->write(str);
        if(program)
            program->end=true;
    } catch (...){
        this->write("Unknown error. Console::printCmd()");
    }

}

void Console::cmdRun(){
    //TODO: 建立program类 调用里面的run()
    static bool flag=false;
    if(!flag){
         program=new Program(buffer,this);
         flag=true;
    }
    else{
        delete program;
        program=new Program(buffer,this);
    }
    program->run();

}

void Console::cmdList(){
    buffer->printCode();
}

void Console::cmdClear(){
    buffer->clear();
}

void Console::cmdHelp(){
    //TODO: 输出帮助信息 如命令
    this->write("s <filename.txt>: save the code to filename.txt");
    this->write("l <filename.txt>: Load the code from filename.txt");
    this->write("d <lineNum>: Delete line with specified lineNum");
    this->write("PRINT <exp>: Print result of following expression");
    this->write("RUN: Run the code");
    this->write("LIST: Show the code stored in buffer");
    this->write("CLEAR: Clear the buffer");
}
