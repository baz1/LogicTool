#ifndef PROOF_H
#define PROOF_H

#include <QMdiSubWindow>
#include <QWebView>

class ProofWindow : public QMdiSubWindow
{
    Q_OBJECT
public:
    explicit ProofWindow(QWidget *parent = 0, QString filename = QString());
    bool prepareClose() const;
    bool save() const;
signals:
    void closed();
public slots:
protected:
    void closeEvent(QCloseEvent *closeEvent);
private:
    QWebView *view;
    QString filename, dispname;
    mutable bool modified;
};

#endif // PROOF_H
