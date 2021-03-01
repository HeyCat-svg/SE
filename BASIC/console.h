#ifndef CONSOLE_H
#define CONSOLE_H

#include "buffer.h"
#include "program.h"
#include <QObject>
#include <QTextEdit>
#include <QWidget>

class Console:public QTextEdit{
    Q_OBJECT
    friend class Program;
private:
    Buffer *buffer=nullptr;
    Program *program=nullptr;

    void cmdDelete(int lineNum);
    void cmdSave(const QString &filename); //used to save the code to a file
    void cmdLoad(const QString &filename); //used to load code from a file
    void cmdPrint(const QString &expression);
    void cmdRun();                        //run the code
    void cmdList();                       //correspond to the command "LIST"
    void cmdClear();                      //correspond to the command "CLEAR"
    void cmdHelp();                       //correspond to the command "HELP"


public:
    explicit Console(QWidget *parent = nullptr);
    ~Console() override;
    void write(QString msg);
    //void receiveWords(const QString &cmd);//run()的变体

signals:
    void newLineWritten(const QString newLine);

public slots:
    void dispatchCmd(const QString &cmd); //porcess commands from user

protected:
    virtual void keyPressEvent(QKeyEvent *event) override;//检测回车等键 接收命令 回车，发送信号给dispatch
};

#endif // CONSOLE_H
