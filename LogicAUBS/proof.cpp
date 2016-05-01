#include "proof.h"

#include <QStringList>
#include <QMap>
#include <QMutex>
#include <QObject>
#include <QFile>
#include <QTextStream>

static QMap<QString, QSharedPointer<Rule> > basicRules;
static QMutex basicRulesLock;

QString Expression::getLevelCompliantStr(int maxLevel, bool bracketized) const
{
    if (getLevel() <= maxLevel)
        return getStr(bracketized);
    return QStringLiteral("(") + getStr(bracketized) + QStringLiteral(")");
}

Expression *Expression::fromStr(const QString &str)
{
    int end;
    Expression *result = fromStrAux(str, 0, 3, end);
    if (end != str.length())
        return NULL;
    return result;
}

Expression *Expression::fromStrAux(const QString &str, int start, int maxLevel, int &end)
{
    if (maxLevel >= 2) {
        if (maxLevel >= 3) {
            Expression *e1 = fromStrAux(str, start, 2, end);
            if (!e1)
                return NULL;
            if ((end >= str.length()) || ((str[end] != '>') && (str[end] != '=')))
                return e1;
            char mem = str[end].toLatin1();
            Expression *e2 = fromStrAux(str, ++end, 2, end);
            if (!e2)
                return NULL;
            if (mem == '=')
                return new ExprEquiv(e1, e2);
            return new ExprImply(e1, e2);
        }
        Expression *e1 = fromStrAux(str, start, 1, end);
        if (!e1)
            return NULL;
        if ((end >= str.length()) || ((str[end] != '|') && (str[end] != '&')))
            return e1;
        char mem = str[end].toLatin1();
        Expression *e2 = fromStrAux(str, ++end, 1, end);
        if (!e2)
            return NULL;
        if (mem == '|')
            return new ExprOR(e1, e2);
        return new ExprAND(e1, e2);
    }
    if (start >= str.length())
        return NULL;
    if (maxLevel >= 1) {
        int mid = start;
        while (str[mid] == '~') {
            if (++mid >= str.length())
                return NULL;
        }
        Expression *result = fromStrAux(str, mid, 0, end);
        while (start < mid) {
            result = new ExprNOT(result);
            ++start;
        }
        return result;
    }
    if (str[start] == '(') {
        Expression *e = fromStrAux(str, start + 1, 3, end);
        if ((end >= str.length()) || (str[end] != ')'))
            return NULL;
        ++end;
        return e;
    }
    end = start;
    while ((end < str.length()) && str[end].isLetter())
        ++end;
    if (end == start)
        return NULL;
    return new ExprVar(str.mid(start, end - start));
}

ExprVar::ExprVar(QString variableName) : varName(variableName)
{
    for (int i = variableName.length(); i-- > 0;) {
        if (!variableName[i].isLetter()) {
            varName = "?";
            return;
        }
    }
}

QString ExprVar::getStr(bool bracketized) const
{
    if (bracketized)
        return QStringLiteral("[") + varName + QStringLiteral("]");
    return varName;
}

QSet<QString> ExprVar::getVariables() const
{
    QSet<QString> result;
    result.insert(varName);
    return result;
}

Expression *ExprVar::getCopy() const
{
    return new ExprVar(varName);
}

QSharedPointer<Expression> ExprVar::replaceVariableNames(QMap<QString, QSharedPointer<Expression> > renaming) const
{
    return renaming.value(varName);
}

int ExprVar::getLevel() const
{
    return 0;
}

ExprNOT::ExprNOT(Expression *e) : e(e) {}

QString ExprNOT::getStr(bool bracketized) const
{
    return QStringLiteral("~") + e->getLevelCompliantStr(1, bracketized);
}

QSet<QString> ExprNOT::getVariables() const
{
    return e->getVariables();
}

Expression *ExprNOT::getCopy() const
{
    return new ExprNOT(e->getCopy());
}

QSharedPointer<Expression> ExprNOT::replaceVariableNames(QMap<QString, QSharedPointer<Expression> > renaming)
{
    e->replaceVariableNames(renaming);
}

int ExprNOT::getLevel() const
{
    return 1;
}

ExprOR::ExprOR(Expression *e1, Expression *e2) : e1(e1), e2(e2) {}

QString ExprOR::getStr(bool bracketized) const
{
    return e1->getLevelCompliantStr(1, bracketized) + QStringLiteral("|") + e2->getLevelCompliantStr(1, bracketized);
}

QSet<QString> ExprOR::getVariables() const
{
    return e1->getVariables() | e2->getVariables();
}

Expression *ExprOR::getCopy() const
{
    return new ExprOR(e1->getCopy(), e2->getCopy());
}

void ExprOR::replaceVariableNames(QMap<QString, QSharedPointer<Expression> > renaming)
{
    e1->replaceVariableNames(renaming);
    e2->replaceVariableNames(renaming);
}

int ExprOR::getLevel() const
{
    return 2;
}

ExprAND::ExprAND(Expression *e1, Expression *e2) : e1(e1), e2(e2) {}

QString ExprAND::getStr(bool bracketized) const
{
    return e1->getLevelCompliantStr(1, bracketized) + QStringLiteral("&") + e2->getLevelCompliantStr(1, bracketized);
}

QSet<QString> ExprAND::getVariables() const
{
    return e1->getVariables() | e2->getVariables();
}

Expression *ExprAND::getCopy() const
{
    return new ExprAND(e1->getCopy(), e2->getCopy());
}

void ExprAND::replaceVariableNames(QMap<QString, QSharedPointer<Expression> > renaming)
{
    e1->replaceVariableNames(renaming);
    e2->replaceVariableNames(renaming);
}

int ExprAND::getLevel() const
{
    return 2;
}

ExprImply::ExprImply(Expression *e1, Expression *e2) : e1(e1), e2(e2) {}

QString ExprImply::getStr(bool bracketized) const
{
    return e1->getLevelCompliantStr(2, bracketized) + QStringLiteral(">") + e2->getLevelCompliantStr(2, bracketized);
}

QSet<QString> ExprImply::getVariables() const
{
    return e1->getVariables() | e2->getVariables();
}

Expression *ExprImply::getCopy() const
{
    return new ExprImply(e1->getCopy(), e2->getCopy());
}

void ExprImply::replaceVariableNames(QMap<QString, QSharedPointer<Expression> > renaming)
{
    e1->replaceVariableNames(renaming);
    e2->replaceVariableNames(renaming);
}

int ExprImply::getLevel() const
{
    return 3;
}

ExprEquiv::ExprEquiv(Expression *e1, Expression *e2) : e1(e1), e2(e2) {}

QString ExprEquiv::getStr(bool bracketized) const
{
    return e1->getLevelCompliantStr(2, bracketized) + QStringLiteral("=") + e2->getLevelCompliantStr(2, bracketized);
}

QSet<QString> ExprEquiv::getVariables() const
{
    return e1->getVariables() | e2->getVariables();
}

Expression *ExprEquiv::getCopy() const
{
    return new ExprEquiv(e1->getCopy(), e2->getCopy());
}

void ExprEquiv::replaceVariableNames(QMap<QString, QSharedPointer<Expression> > renaming)
{
    e1->replaceVariableNames(renaming);
    e2->replaceVariableNames(renaming);
}

int ExprEquiv::getLevel() const
{
    return 3;
}

Rule::Rule(QList<QSharedPointer<Expression> > premises, QList<QSharedPointer<Expression> > conclusions) : premises(premises), conclusions(conclusions) {}

QString Rule::getStr(bool bracketized) const
{
    QString result;
    if (conclusions.isEmpty())
        return QString();
    if (premises.isEmpty()) {
        result = ": ";
    } else {
        result = premises.first()->getStr(bracketized);
        for (int i = premises.size(); --i > 0;)
            result += QStringLiteral(", ") + premises[i]->getStr(bracketized);
        result += " : ";
    }
    result += conclusions.first()->getStr(bracketized);
    for (int i = conclusions.size(); --i > 0;)
        result += QStringLiteral(", ") + conclusions[i]->getStr(bracketized);
    return result;
}

QList<QSharedPointer<Expression> > Rule::getPremises() const
{
    return premises;
}

QList<QSharedPointer<Expression> > Rule::getConclusions() const
{
    return conclusions;
}

QSet<QString> Rule::getInputVariables() const
{
    QSet<QString> result;
    foreach (const QSharedPointer<Expression> &e, premises)
        result |= e->getVariables();
    return result;
}

QSet<QString> Rule::getOutputVariables() const
{
    QSet<QString> result;
    foreach (const QSharedPointer<Expression> &e, conclusions)
        result |= e->getVariables();
    return result;
}

Rule *Rule::adapt(QMap<QString, QSharedPointer<Expression> > renaming) const
{
    QList< QSharedPointer<Expression> > o_premises, o_conclusions;
    o_premises.reserve(premises.size());
    foreach (const QSharedPointer<Expression> &prem, premises)
        o_premises.append(prem->replaceVariableNames(renaming));
    o_conclusions.reserve(conclusions.size());
    foreach (const QSharedPointer<Expression> &cl, conclusions)
        o_conclusions.append(cl->replaceVariableNames(renaming));
    return new Rule(o_premises, o_conclusions);
}

Rule *Rule::fromStr(const QString &str)
{
    QList< QSharedPointer<Expression> > premises, conclusions;
    int colon = str.indexOf(':');
    if (colon < 0)
        return NULL;
    QStringList prems = str.left(colon).trimmed().split(',', QString::SkipEmptyParts);
    foreach (const QString &prem, prems) {
        Expression *ptr = Expression::fromStr(prem.trimmed());
        if (!ptr)
            return NULL;
        premises.append(QSharedPointer<Expression>(ptr));
    }
    QStringList cls = str.mid(colon + 1).trimmed().split(',', QString::SkipEmptyParts);
    if (cls.isEmpty())
        return NULL;
    foreach (const QString &cl, cls) {
        Expression *ptr = Expression::fromStr(cl.trimmed());
        if (!ptr)
            return NULL;
        conclusions.append(QSharedPointer<Expression>(ptr));
    }
    return new Rule(premises, conclusions);
}

Proof::Proof(QSharedPointer<Rule> rule) : rule(rule), ok(true), finished(false)
{
    basicRulesLock.lock();
    if (basicRules.isEmpty()) {
        basicRules[":ElimAnd"] = QSharedPointer<Rule>(Rule::fromStr("X&Y : X, Y"));
        basicRules[":IntroAnd"] = QSharedPointer<Rule>(Rule::fromStr("X, Y : X&Y"));
        basicRules[":ElimOr1"] = QSharedPointer<Rule>(Rule::fromStr("X|Y, ~X : Y"));
        basicRules[":ElimOr2"] = QSharedPointer<Rule>(Rule::fromStr("X|Y, ~Y : X"));
        basicRules[":IntroOr"] = QSharedPointer<Rule>(Rule::fromStr("X : X|Y, Y|X"));
        basicRules[":ElimArrow"] = QSharedPointer<Rule>(Rule::fromStr("X, X>Y : Y"));
        // Note: ":Assume", ":IntroArrow" and ":RAA" are also handled separately for correctness.
        basicRules[":Assume"] = QSharedPointer<Rule>(Rule::fromStr(": X"));
        basicRules[":IntroArrow"] = QSharedPointer<Rule>(Rule::fromStr("Y : X>Y"));
        basicRules[":RAA"] = QSharedPointer<Rule>(Rule::fromStr("Y, ~Y : X"));
        // Note: ":Assume", ":IntroArrow" and ":RAA" are also handled separately for correctness.
        basicRules[":ElimEquiv"] = QSharedPointer<Rule>(Rule::fromStr("X=Y : X>Y, Y>X"));
        basicRules[":IntroEquiv"] = QSharedPointer<Rule>(Rule::fromStr("X>Y, Y>X : X=Y"));
        basicRules[":Conclusion"] = QSharedPointer<Rule>(Rule::fromStr("X : X"));
    }
    basicRulesLock.unlock();
    if (rule->getConclusions().isEmpty()) {
        ok = false;
        return;
    }
    QList< QSharedPointer<Expression> > premises = rule->getPremises();
    steps.reserve(2 * premises.size());
    for (int i = 0; i < premises.size(); ++i) {
        Step step;
        step.indentation = 0;
        step.output = premises[i]->getStr();
        step.rule = "-";
        step.clIndex = 0;
        steps.append(step);
    }
}

Proof::Proof(QString filename) : ok(false)
{
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly)) {
        lastError = "Could not open file for reading";
        return;
    }
    QTextStream in(&file);
    rule = QSharedPointer<Rule>(Rule::fromStr(in.readLine()));
    QString s;
    int n, i;
    while (in >> s, s != "IDX") {
        Step step;
        step.output = s;
        in >> s;
        s.replace("%20", " ");
        step.rule = s;
        in >> n;
        step.usedInputs.reserve(n);
        for (int j = n; j--;) {
            in >> i;
            step.usedInputs.append(i);
        }
        in >> step.clIndex >> step.indentation;
        while (in >> s, s != "END_STEP") {
            if (in.atEnd()) {
                lastError = "Unexpected end of file";
                file.close();
                return;
            }
            if ((i = s.indexOf(':')) < 0) {
                lastError = "Wrong renaming rule";
                file.close();
                return;
            }
            step.renaming[s.left(i)] = s.mid(i + 1);
        }
        steps.append(step);
    }
    in >> n;
    stepIndexes.reserve(n);
    for (int j = n; j--;) {
        in >> i;
        stepIndexes.append(i);
    }
    file.close();
    if (ok = verifyCorrect())
        finished = verifyFinished();
}

bool Proof::saveToFile(QString filename) const
{
    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly)) {
        lastError = "Could not open file for writing";
        return false;
    }
    file.write((rule->getStr() + "\n").toLocal8Bit());
    for (int i = 0; i < steps.size(); ++i) {
        const Step &currentStep = steps[i];
        file.write(currentStep.output.toLocal8Bit());
        file.write(" ");
        QString ruleModif = currentStep.rule;
        ruleModif.replace(" ", "%20");
        file.write(ruleModif.toLocal8Bit());
        file.write((QStringLiteral(" ") + QString::number(currentStep.usedInputs.size()) + " ").toLocal8Bit());
        for (int j = 0; j < currentStep.usedInputs.size(); ++j)
            file.write((QString::number(currentStep.usedInputs[j]) + " ").toLocal8Bit());
        file.write((QString::number(currentStep.clIndex) + " " + QString::number(currentStep.indentation) + " ").toLocal8Bit());
        QMap<QString,QString>::const_iterator it = currentStep.renaming.constBegin();
        while (it != currentStep.renaming.constEnd()) {
            file.write((it.key() + ":" + it.value() + " ").toLocal8Bit());
            ++it;
        }
        file.write("END_STEP\n");
    }
    file.write((QStringLiteral("IDX ") + QString::number(stepIndexes.size())).toLocal8Bit());
    for (int i = 0; i < stepIndexes.size(); ++i)
        file.write((QStringLiteral(" ") + QString::number(stepIndexes[i])).toLocal8Bit());
    file.write("\n");
    file.close();
}

bool Proof::isCorrect() const
{
    return ok;
}

bool Proof::isFinished() const
{
    return finished;
}

QSharedPointer<Rule> Proof::getRule() const
{
    return rule;
}

bool Proof::verifyCorrect() const
{
    QList< QSharedPointer<Expression> > premises = rule->getPremises();
    QList<bool> accessible;
    accessible.reserve(steps.size());
    int indent = 0;
    for (int i = premises.size(); i-- > 0;) {
        const Step &currentStep = steps[i];
        if (currentStep.indentation != indent)
            return false;
        if (premises[i]->getStr() != currentStep.output)
            return false;
        accessible.append(true);
    }
    for (int i = premises.size(); i < steps.size(); ++i) {
        const Step &currentStep = steps[i];
        if (currentStep.rule == ":Assume") {
            ++indent;
        } else if ((currentStep.rule == ":IntroArrow") || (currentStep.rule == ":RAA")) {
            --indent;
        }
        if (currentStep.indentation != indent)
            return false;
        QSharedPointer<Rule> subRule;
        if (currentStep.rule[0] == ':') {
            subRule = basicRules.value(currentStep.rule);
            if (subRule.isNull()) {
                lastError = QObject::tr("Unrecognized rule \"%1\".").arg(currentStep.rule);
                return false;
            }
        } else {
            Proof *proof = new Proof(currentStep.rule);
            if (!proof->isCorrect()) {
                delete proof;
                lastError = QObject::tr("Lemma \"%1\" is not found or not correct.").arg(currentStep.rule);
                return false;
            }
            if (!proof->isFinished()) {
                delete proof;
                lastError = QObject::tr("Lemma \"%1\" is not finished.").arg(currentStep.rule);
                return false;
            }
            subRule = proof->getRule();
            delete proof;
        }
        subRule = QSharedPointer<Rule>(subRule->adapt(currentStep.renaming));
        premises = subRule->getPremises();
        if (premises.size() != currentStep.usedInputs.size())
            return false;
        for (int j = premises.size(); j-- > 0;) {
            if ((currentStep.usedInputs[j] < 0) || (currentStep.usedInputs[j] >= i))
                return false;
            if (steps[currentStep.usedInputs[j]].output != premises[j]->getStr())
                return false;
            if (!accessible[currentStep.usedInputs[j]])
                return false;
        }
        if ((currentStep.clIndex < 0) || (currentStep.clIndex > subRule->getConclusions().size()))
            return false;
        if (output != subRule->getConclusions().at(currentStep.clIndex)->getStr())
            return false;
        if ((currentStep.rule == ":IntroArrow") || (currentStep.rule == ":RAA")) {
            int j = i - 1;
            while ((j >= 0) && (steps[j].indentation > indent))
                accessible[j--] = false;
            if (steps[++j].renaming["X"] != currentStep.renaming["X"])
                return false;
        }
    }
    return true;
}

bool Proof::verifyFinished() const
{
    QList< QSharedPointer<Expression> > conclusions = rule->getConclusions();
    if (steps.last().indentation)
        return false;
    if (stepIndexes.size() != conclusions.size())
        return false;
    for (int i = conclusions.size(); i-- > 0;) {
        if ((stepIndexes[i] < 0) || (stepIndexes[i] >= steps.size()))
            return false;
        if (steps[stepIndexes[i]].output != conclusions[i]->getStr())
            return false;
    }
    return true;
}
