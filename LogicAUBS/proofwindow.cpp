#include "proofwindow.h"

#include <QMessageBox>
#include <QCloseEvent>

ProofWindow::ProofWindow(QWidget *parent, QString filename) : QMdiSubWindow(parent), filename(filename), modified(false)
{
    setAttribute(Qt::WA_DeleteOnClose);
    if (filename.isEmpty()) {
        setWindowTitle("[New Proof]");
    } else {
        filename.replace('\\', '/');
        dispname = filename.mid(filename.lastIndexOf('/') + 1);
        setWindowTitle(dispname);
    }
    view = new QWebView(this);
    view->setUrl(QUrl("qrc:/res/page.html"));
    setWidget(view);
}

bool ProofWindow::prepareClose() const
{
    if (!modified)
        return true;
    int result;
    if (filename.isEmpty())
        result = QMessageBox::question(const_cast<ProofWindow*>(this), tr("Save changes?"),
                                       tr("Would you like to save this new proof?"), QMessageBox::Yes, QMessageBox::No, QMessageBox::Cancel);
    else
        result = QMessageBox::question(const_cast<ProofWindow*>(this), tr("Save changes?"),
                                       tr("Would you like to save the proof \"%1\"?").arg(dispname), QMessageBox::Yes, QMessageBox::No, QMessageBox::Cancel);
    if (result == QMessageBox::Cancel)
        return false;
    if (result == QMessageBox::Yes)
            return save();
    return true;
}

bool ProofWindow::save() const
{
    // TODO
    return true;
}

void ProofWindow::closeEvent(QCloseEvent *closeEvent)
{
    if (!prepareClose()) {
        closeEvent->ignore();
        return;
    }
    emit closed();
    closeEvent->accept();
}
