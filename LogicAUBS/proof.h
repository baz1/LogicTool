#ifndef PROOF_H
#define PROOF_H

#include <QString>
#include <QSharedPointer>
#include <QSet>

class Expression
{
public:
    virtual QString getStr(bool bracketized = false) const = 0;
    virtual QSet<QString> getVariables() const = 0;
    virtual QSharedPointer<Expression> replaceVariableNames(QMap<QString, QSharedPointer<Expression> > renaming) const = 0;
    virtual int getLevel() const = 0;
    QString getLevelCompliantStr(int maxLevel, bool bracketized = false) const;
public:
    static Expression *fromStr(const QString &str);
private:
    static Expression *fromStrAux(const QString &str, int start, int maxLevel, int &end);
};

class ExprVar : public Expression
{
public:
    ExprVar(QString variableName);
    QString getStr(bool bracketized = false) const;
    QSet<QString> getVariables() const;
    QSharedPointer<Expression> replaceVariableNames(QMap<QString, QSharedPointer<Expression> > renaming) const;
    int getLevel() const;
private:
    QString varName;
};

class ExprNOT : public Expression
{
public:
    ExprNOT(Expression *e);
    QString getStr(bool bracketized = false) const;
    QSet<QString> getVariables() const;
    QSharedPointer<Expression> replaceVariableNames(QMap<QString, QSharedPointer<Expression> > renaming) const;
    int getLevel() const;
private:
    QSharedPointer<Expression> e;
};

class ExprOR : public Expression
{
public:
    ExprOR(Expression *e1, Expression *e2);
    QString getStr(bool bracketized = false) const;
    QSet<QString> getVariables() const;
    QSharedPointer<Expression> replaceVariableNames(QMap<QString, QSharedPointer<Expression> > renaming) const;
    int getLevel() const;
private:
    QSharedPointer<Expression> e1, e2;
};

class ExprAND : public Expression
{
public:
    ExprAND(Expression *e1, Expression *e2);
    QString getStr(bool bracketized = false) const;
    QSet<QString> getVariables() const;
    QSharedPointer<Expression> replaceVariableNames(QMap<QString, QSharedPointer<Expression> > renaming) const;
    int getLevel() const;
private:
    QSharedPointer<Expression> e1, e2;
};

class ExprImply : public Expression
{
public:
    ExprImply(Expression *e1, Expression *e2);
    QString getStr(bool bracketized = false) const;
    QSet<QString> getVariables() const;
    QSharedPointer<Expression> replaceVariableNames(QMap<QString, QSharedPointer<Expression> > renaming) const;
    int getLevel() const;
private:
    QSharedPointer<Expression> e1, e2;
};

class ExprEquiv : public Expression
{
public:
    ExprEquiv(Expression *e1, Expression *e2);
    QString getStr(bool bracketized = false) const;
    QSet<QString> getVariables() const;
    QSharedPointer<Expression> replaceVariableNames(QMap<QString, QSharedPointer<Expression> > renaming) const;
    int getLevel() const;
private:
    QSharedPointer<Expression> e1, e2;
};

class Rule
{
public:
    Rule(QList< QSharedPointer<Expression> > premises, QList< QSharedPointer<Expression> > conclusions);
    QString getStr(bool bracketized = false) const;
    QList< QSharedPointer<Expression> > getPremises() const;
    QList< QSharedPointer<Expression> > getConclusions() const;
    QSet<QString> getInputVariables() const;
    QSet<QString> getOutputVariables() const;
    Rule *adapt(QMap<QString, QSharedPointer<Expression> > renaming) const;
public:
    static Rule *fromStr(const QString &str);
private:
    QList< QSharedPointer<Expression> > premises, conclusions;
};

struct Step
{
    QString rule;
    QList<int> usedInputs;
    QMap<QString, QString> renaming;
    int clIndex;
    QString output;
    int indentation;
};

class Proof
{
public:
    Proof(QSharedPointer<Rule> rule);
    Proof(QString filename);
    bool saveToFile(QString filename) const;
    bool isCorrect() const;
    bool isFinished() const;
    QSharedPointer<Rule> getRule() const;
private:
    bool verifyCorrect() const;
    bool verifyFinished() const;
private:
    QSharedPointer<Rule> rule;
    QList<Step> steps;
    QList<int> stepIndexes;
    bool ok, finished;
    QString lastError;
};

#endif // PROOF_H
