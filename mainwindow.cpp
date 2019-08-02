/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** BSD License Usage
** Alternatively, you may use this file under the terms of the BSD license
** as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "dommodel.h"
#include "mainwindow.h"

#include <QDomDocument>
#include <QTreeView>
#include <QMenuBar>
#include <QFileDialog>

#include <qmessagebox.h>
#include <QtWidgets>

#include <qdebug.h>

MainWindow::MainWindow() : QMainWindow(), model(nullptr)
{
    model = new DomModel(QDomDocument(), this);
    view = new QTreeView(this);
    view->setModel(model);

    setCentralWidget(view);
    setWindowTitle(tr("GGSE - A Ground Ground Sequence Editor"));

    //set custom context menu
    contextMenu = new QMenu(view);
    view->setContextMenuPolicy(Qt::CustomContextMenu);

    createActions();
    createStatusBar();

    //connect signals
    connect(view, &QTreeView::expanded, this, &MainWindow::setToSize);
    connect(view, &QTreeView::collapsed, this, &MainWindow::setToSize);
    connect(view, &QTreeView::customContextMenuRequested, this, &MainWindow::onCustomContextMenu);
    setSignals();
}

void MainWindow::setSignals()
{
    connect(view->selectionModel(), &QItemSelectionModel::selectionChanged, this, &MainWindow::updateActions);
}

void MainWindow::openFile()
{
    QString filePath = QFileDialog::getOpenFileName(this, tr("Open File"), xmlPath, tr("XML files (*.xml)"));
    if(!filePath.isEmpty())
    {
        if(openFileByName(filePath))
        {
            //seems that update the model cause a signal reset.
            setSignals();
        }
    }
}

void MainWindow::setToSize(const QModelIndex &index)
{
    for (int column = 0; column < model->columnCount(); ++column)
        view->resizeColumnToContents(column);
}

void MainWindow::newFile()
{
    QFileDevice::FileError err = QFileDevice::NoError;
    QString filePath = ":/res/template.xml";
    if(!openFileByName(filePath))
        QMessageBox::information(this,tr("Info"), tr("An error occurred while creating a new File."));
    else
    {
        //seems that update the model cause a signal reset.
        setSignals();
    }
}

bool MainWindow::save()
{
    QMessageBox::information(this,tr("Info"), tr("This function is not yet implemented."));
    return true;
}

bool MainWindow::saveAs()
{
    QFileDevice::FileError err = QFileDevice::NoError;
    QString filePath = QFileDialog::getSaveFileName(this, tr("Save As"), xmlPath, tr("XML files (*.xml)"));
    if(!filePath.isEmpty())
    {
        QFile file(filePath);
        if(file.open(QIODevice::WriteOnly | QIODevice::Text))
        {
            //TODO: salvare su un file temporaneo. Verificare la correttezza.
            //salvare sul file richiesto. Eliminare il file temporaneo.
            QTextStream stream;
            stream.setDevice(&file);
            model->DomSave(stream, 4);
            file.close();
            statusBar()->showMessage("File saved correctly");
        } else {
            err = file.error();
            QMessageBox::warning(this, "Error", file.errorString());
        }
    }
    return true;
}

void MainWindow::addElement()
{
    //recupera l'item del tree attivo. Verifica se è un  nodo a cui si può aggiungere uno step. Chiama addRows.
    //NOTA: dovrebbe verificare se il parent è step o se è startup, main, clenaup
    QModelIndex index = view->selectionModel()->currentIndex();

    //insertRow calls the virtual method insertRows (overridden in dommodel)
    //per coerenza con l'origine della funzione lascio il +1 e lo risottraggo in insertRow
    if (!model->insertRow(index.row()+1, index.parent()))
        return;

    //updateActions();
}

void MainWindow::expandAll()
{
    view->expandAll();
    for (int column = 0; column < model->columnCount(); ++column)
        view->resizeColumnToContents(column);
}

void MainWindow::collapseAll()
{
    view->collapseAll();
}

void MainWindow::about()
{
    QMessageBox::about(this, tr("About Application"),
                       tr("An application with no reason."));
}

void MainWindow::createActions()
{
    fileMenu = menuBar()->addMenu(tr("&File"));
    fileToolBar = addToolBar(("File"));

    const QIcon newIcon = QIcon::fromTheme("document-new", QIcon(":/images/new.png"));
    QAction *newAct = new QAction(newIcon, tr("&New"), this);
    newAct->setShortcut(QKeySequence::New);
    newAct->setStatusTip(tr("Create a new sequence"));
    connect(newAct, &QAction::triggered, this, &MainWindow::newFile); //connette il pulsante alla funzione
    fileMenu->addAction(newAct);
    fileToolBar->addAction(newAct);

    //fileMenu->addAction(tr("&Open..."), this, &MainWindow::openFile, QKeySequence::Open);
    const QIcon openIcon = QIcon::fromTheme("document-open", QIcon(":/images/open.png"));
    QAction *openAct = new QAction(openIcon, tr("&Open..."), this);
    openAct->setShortcut(QKeySequence::Open);
    openAct->setStatusTip(tr("Open An Existing File"));
    connect(openAct, &QAction::triggered, this, &MainWindow::openFile);
    fileMenu->addAction(openAct);
    fileToolBar->addAction(openAct);

    const QIcon saveAsIcon = QIcon::fromTheme("document-save-as");
    QAction *saveAsAct = new QAction(saveAsIcon, tr("Save &As..."), this);
    saveAsAct->setShortcut((QKeySequence::SaveAs));
    saveAsAct->setStatusTip(tr("Save sequence under new name"));
    connect(saveAsAct, &QAction::triggered, this, &MainWindow::saveAs);
    fileMenu->addAction(saveAsAct);
    fileToolBar->addAction(saveAsAct);

    fileMenu->addSeparator();
    fileToolBar->addSeparator();

    const QIcon addStepIcon = QIcon::fromTheme("addStep");
    addStepAct = new QAction(addStepIcon, tr("Add Step"), this);
    addStepAct->setStatusTip(tr("Add a new element"));
    connect(addStepAct, &QAction::triggered, this, &MainWindow::addElement);
    fileToolBar->addAction(addStepAct);
    addStepAct->setEnabled(false);
    contextMenu->addAction(addStepAct); //add to context menu

    fileToolBar->addSeparator();

    const QIcon expandAllIcon = QIcon::fromTheme("expandAll");
    QAction *expandAllAct = new QAction(expandAllIcon, tr("Expand all"), this);
    expandAllAct->setStatusTip(tr("Expand all items"));
    connect(expandAllAct, &QAction::triggered, this, &MainWindow::expandAll);
    fileToolBar->addAction(expandAllAct);

    const QIcon collapseAllIcon = QIcon::fromTheme("collapseAll");
    QAction *collapseAllAct = new QAction(collapseAllIcon, tr("Collapse all"), this);
    collapseAllAct->setStatusTip(tr("Collapse all items"));
    connect(collapseAllAct, &QAction::triggered, this, &MainWindow::collapseAll);
    fileToolBar->addAction(collapseAllAct);

    fileMenu->addSeparator();

    //fileMenu->addAction(tr("E&xit"), this, &QWidget::close, QKeySequence::Quit);
    const QIcon exitIcon = QIcon::fromTheme("application-exit");
    QAction *exitAct = fileMenu->addAction(exitIcon, tr("E&xit"), this, &QWidget::close);
    exitAct->setShortcut(tr("Exit the application"));

    //addStepAct = new QAction("Add Step",contextMenu);
    addConditionAct = new QAction("Add Condition", contextMenu);
    addInputAct = new QAction("Add Input", contextMenu);
    addOutputAct = new QAction("Add Output", contextMenu);
}

void MainWindow::createStatusBar()
{
    //il widget è creato automaticamente alla prima chiamata
    statusBar()->showMessage(tr("Ready"));
}

void MainWindow::onCustomContextMenu(const QPoint &point)
{
    QModelIndex index = view->indexAt(point);

    showContextMenu(index, view->viewport()->mapToGlobal(point));
}

void MainWindow::showContextMenu(QModelIndex index, const QPoint& globalPos)
{
    QMenu menu;
    FieldType type = model->getItemType(index);
    if (index.isValid())
    {
        switch(type) {
        case FieldType::step:
            menu.addAction(addStepAct);
            menu.addAction(addConditionAct);
            menu.addAction(addInputAct);
            menu.addAction(addOutputAct);
            break;
        case FieldType::section:
            menu.addAction(addStepAct);
            break;
        case FieldType::input:
            break;
        case FieldType::output:
            break;
        default:
            break;
        }
        menu.exec(globalPos);
    }
}

//verificare il tipo di step selezionato e in base a quello cambiare etichetta al pulsante
void MainWindow::updateActions()
{
    bool hasSelection = !view->selectionModel()->selection().isEmpty();

    if(hasSelection) {
        QModelIndex index = view->selectionModel()->currentIndex();
        FieldType type = model->getItemType(index);
        switch(type) {
        case FieldType::step:
        case FieldType::section:
            addStepAct->setText(tr("Add Step"));
            addStepAct->setEnabled(true);
            break;
        case FieldType::input:
            addStepAct->setText(tr("Add Input"));
            addStepAct->setEnabled(true);
            break;
        case FieldType::output:
            addStepAct->setText(tr("Add Output"));
            addStepAct->setEnabled(true);
            break;
        default:
            addStepAct->setEnabled(false);
        }
    }

    bool hasCurrent = view->selectionModel()->currentIndex().isValid();
    if (hasCurrent) {
        int row = view->selectionModel()->currentIndex().row();
        int column = view->selectionModel()->currentIndex().column();
        if (view->selectionModel()->currentIndex().parent().isValid())
            statusBar()->showMessage(tr("Position: (%1,%2)").arg(row).arg(column));
        else
            statusBar()->showMessage(tr("Position: (%1,%2) in top level").arg(row).arg(column));
    }
}

bool MainWindow::openFileByName(QString fileName)
{
    QFile file(fileName);
    //apro il file read only in quanto lo leggo solamente
    //quando vado a salvarlo deve essere riaperto write
    if (file.exists() && file.open(QFile::ReadOnly | QFile::Text)) {
        QDomDocument document;
        QString error;
        int line, column;
        if (document.setContent(&file, &error, &line, &column))
        {
            //crea un oggetto DomModel derivato da QabstractItemModel
            DomModel *newModel = new DomModel(document, this);
            //lega il modello creato al tree
            view->setModel(newModel);
            //svuota i vecchi dati
            delete model;
            //popola con nuovi dati. Viene invocato il costruttore di assegnazione che in questo caso
            //effettua una copia 1:1. NOTA: se presenti vengono copiati anche i puntatori, di conseguenza
            //se si distrugge l'oggetto da cui è partita la copia, gli attributi puntatore di quelli copiati
            //punteranno a locazioni di memoria inesistenti (Nell'ipotesi che il distruttore liberi correttamente
            //la memoria allocata dai membri della classe).
            model = newModel;
            for (int column = 0; column < model->columnCount(); ++column)
                view->resizeColumnToContents(column);
            if(!fileName.contains(":/res"))
                xmlPath = fileName;
            statusBar()->showMessage("File opened correctly");
        }
        else
        {
            QMessageBox::warning(this, "Error", QStringLiteral("Error: %1 in line %2, column %3").arg(error));
        }
        file.close();
        return true;
    }
    return false;
}
