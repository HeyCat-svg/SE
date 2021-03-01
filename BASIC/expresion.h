#ifndef EXPRESION_H
#define EXPRESION_H

#include <string>
#include <QString>

enum ExpressionType{CONSTANT,IDENTIFIER,COMPOUND};

class Expression{
public:
    Expression(){}
    virtual ~Expression()=default;
    virtual ExpressionType type(){return COMPOUND;}

    virtual int getConstantValue(){return 0;}
    virtual QString getIdentifierName(){return "";}
    virtual QString getOperator(){return "";}
    virtual Expression *getLHS(){return nullptr;}
    virtual Expression *getRHS(){return nullptr;}
};

class ConstantExp:public Expression{
public:
    ConstantExp(int val);
    ~ConstantExp()=default;
    ExpressionType type();
    int getConstantValue();

private:
    int value;
};

class IdentifierExp:public Expression{
public:
    IdentifierExp(QString name);
    ~IdentifierExp()=default;
    ExpressionType type();
    QString getIdentifierName();

private:
    QString name;
};

class CompoundExp:public Expression{
    friend class Parser;
public:
    CompoundExp(QString op,Expression *lhs=nullptr,Expression *rhs=nullptr);
    ~CompoundExp();
    ExpressionType type();
    QString getOperator();
    Expression *getLHS();
    Expression *getRHS();

private:
    QString op;
    Expression *lhs=nullptr,*rhs=nullptr;
};

#endif // EXPRESION_H
