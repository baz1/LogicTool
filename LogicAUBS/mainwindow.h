#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QList>

#include "proofwindow.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
private:
    void addWindow(ProofWindow *newProof);
    void noWindow();
    void someWindow();
protected:
    void closeEvent(QCloseEvent *evt);
public slots:
    void winClosed();
private slots:
    void on_actionExit_triggered();
    void on_actionNew_proof_triggered();
    void on_actionClose_triggered();
    void on_actionOpen_triggered();
private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
