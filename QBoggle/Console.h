#ifndef CONSOLE_H
#define CONSOLE_H

#include <QObject>
#include <QTextEdit>
#include <QWidget>

class Console : public QTextEdit
{
    Q_OBJECT
    friend class BoggleWindow;
public:
    explicit Console(QWidget *parent = nullptr);

signals:
    void newLineWritten(QString newline);

public slots:
    void _clear();
    void write(QString msg);

protected:
    virtual void keyPressEvent(QKeyEvent *event) override;
};

#endif // CONSOLE_H