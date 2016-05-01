#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QMessageBox>
#include <QFileDialog>
#include <QCloseEvent>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);
#ifdef Q_OS_LINUX
    /* Prevent the Ubuntu bug with the shortcuts not working with appmenu-qt5: */
    addActions(ui->menuBar->actions());
#endif
    /* No shortcut overload with MdiArea: */
    ui->actionClose->setShortcutContext(Qt::WidgetShortcut);
    noWindow();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::addWindow(ProofWindow *newProof)
{
    ui->mdiArea->addSubWindow(newProof);
    newProof->show();
    if (!ui->actionSave->isEnabled())
        someWindow();
    connect(newProof, SIGNAL(closed()), this, SLOT(winClosed()));
}

void MainWindow::noWindow()
{
    ui->actionSave->setEnabled(false);
    ui->actionSave_As->setEnabled(false);
    ui->actionClose->setEnabled(false);
    ui->action_ElimAnd->setEnabled(false);
    ui->action_IntroAnd->setEnabled(false);
    ui->action_ElimOr->setEnabled(false);
    ui->action_IntroOr->setEnabled(false);
    ui->action_ElimArrow->setEnabled(false);
    ui->action_Assume->setEnabled(false);
    ui->action_IntroArrow->setEnabled(false);
    ui->action_RAA->setEnabled(false);
    ui->action_ElimEquiv->setEnabled(false);
    ui->action_IntroEquiv->setEnabled(false);
    ui->action_Lemma->setEnabled(false);
}

void MainWindow::someWindow()
{
    ui->actionSave->setEnabled(true);
    ui->actionSave_As->setEnabled(true);
    ui->actionClose->setEnabled(true);
    ui->action_ElimAnd->setEnabled(true);
    ui->action_IntroAnd->setEnabled(true);
    ui->action_ElimOr->setEnabled(true);
    ui->action_IntroOr->setEnabled(true);
    ui->action_ElimArrow->setEnabled(true);
    ui->action_Assume->setEnabled(true);
    ui->action_IntroArrow->setEnabled(true);
    ui->action_RAA->setEnabled(true);
    ui->action_ElimEquiv->setEnabled(true);
    ui->action_IntroEquiv->setEnabled(true);
    ui->action_Lemma->setEnabled(true);
}

void MainWindow::closeEvent(QCloseEvent *evt)
{
    foreach (const QMdiSubWindow *win, ui->mdiArea->subWindowList()) {
        if (!reinterpret_cast<const ProofWindow*>(win)->prepareClose()) {
            evt->ignore();
            return;
        }
    }
    evt->accept();
}

void MainWindow::winClosed()
{
    if (ui->mdiArea->subWindowList().size() <= 1)
        noWindow();
}

void MainWindow::on_actionExit_triggered()
{
    close();
}

void MainWindow::on_actionNew_proof_triggered()
{
    addWindow(new ProofWindow(ui->mdiArea));
}

void MainWindow::on_actionClose_triggered()
{
    ui->mdiArea->activeSubWindow()->close();
}

void MainWindow::on_actionOpen_triggered()
{
    QString filename = QFileDialog::getOpenFileName(this, tr("Select proof file"), QString(), tr("Proof files (*.aubs)"));
    if (filename.isEmpty())
        return;
    addWindow(new ProofWindow(ui->mdiArea, filename));
}
