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

#include "domitem.h"
#include "dommodel.h"
#include <qdebug.h>

#include <QtXml>

//! [0]
DomModel::DomModel(QDomDocument document, QObject *parent) : QAbstractItemModel(parent), domDocument(document)
{
    rootItem = new DomItem(domDocument, 0);
}
//! [0]

//! [1]
DomModel::~DomModel()
{
    delete rootItem;
}
//! [1]

//! [2]
int DomModel::columnCount(const QModelIndex &/*parent*/) const
{
    return 2;
}
//! [2]

//! [3]
//! Funzione chiamata al click sul tree per aggiornare i campi
//! index è l'indice dell'item nel modello di dati
//! role deve essere DisplayRole ovvero deve essere renderizzabile come testo
QVariant DomModel::data(const QModelIndex &index, int role) const
{

    if (!index.isValid())
        return QVariant();

    DomItem *item = static_cast<DomItem*>(index.internalPointer());
    QDomNode node = item->node();

    //se e' testo non considerare il nodo come riga del tree
    if(node.isText())
        return QVariant();

    if(role == Qt::DisplayRole || role == Qt::EditRole)
    {
        QStringList attributes;
        QDomNamedNodeMap attributeMap = node.attributes();

        switch (index.column()) {
            case 0: //nome
                return node.nodeName();
            case 1: //valore
                //se ha un solo child verifica se e' di tipo text e nel caso usalo come value
                if(node.childNodes().count() == 1)
                    if(node.childNodes().item(0).isText())
                        return node.childNodes().item(0).nodeValue().split("\n").join(' ');
                return node.nodeValue().split("\n").join(' ');
            default:
                return QVariant();
        }
    }
    else {
        return QVariant();
    }
}
//! [4]

//! [5]
//! i flag impostano le proprietà dell'elemento (abilitato, selezionabile, editabile, ecc...)
Qt::ItemFlags DomModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return nullptr;
    //abilita l'edit solo per la colonna valore
    if(index.column() == 1)
        return Qt::ItemIsEditable | QAbstractItemModel::flags(index);
    else
        return QAbstractItemModel::flags(index);
}
//! [5]

//! [6]
QVariant DomModel::headerData(int section, Qt::Orientation orientation,
                              int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        switch (section) {
            case 0:
                return tr("Name");
            case 1:
                return tr("Value");
            default:
                return QVariant();
        }
    }

    return QVariant();
}
//! [6]

//! [7]
QModelIndex DomModel::index(int row, int column, const QModelIndex &parent)
            const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    DomItem *parentItem;

    if (!parent.isValid())
        parentItem = rootItem;
    else
        parentItem = static_cast<DomItem*>(parent.internalPointer());
//! [7]

//! [8]
    DomItem *childItem = parentItem->child(row);
    if (childItem)
        return createIndex(row, column, childItem);
    else
        return QModelIndex();
}
//! [8]

//! [9]
QModelIndex DomModel::parent(const QModelIndex &child) const
{
    if (!child.isValid())
        return QModelIndex();

    DomItem *childItem = static_cast<DomItem*>(child.internalPointer());
    DomItem *parentItem = childItem->parent();

    if (!parentItem || parentItem == rootItem)
        return QModelIndex();

    return createIndex(parentItem->row(), 0, parentItem);
}
//! [9]

//! [10]
int DomModel::rowCount(const QModelIndex &parent) const
{
    if (parent.column() > 0)
        return 0;

    DomItem *parentItem;

    if (!parent.isValid())
        parentItem = rootItem;
    else
        parentItem = static_cast<DomItem*>(parent.internalPointer());

    int numberOfNodes = parentItem->node().childNodes().count();
    //verifica se l'ultimo nodo e' di tipo testo e in caso positivo non considerarlo come riga del tree.
    if(numberOfNodes>0)
        if(parentItem->node().childNodes().item(numberOfNodes-1).isText())
            return numberOfNodes - 1;

    return parentItem->node().childNodes().count();
}
//! [10]

DomItem *DomModel::getItem(const QModelIndex &index) const
{
    if(index.isValid()) {
        DomItem *item = static_cast<DomItem*>(index.internalPointer());
        if(item)
            return item;
    }
    return rootItem;
}

FieldType DomModel::getItemType(const QModelIndex &index) const
{
    FieldType type = FieldType::unknown;
    if(index.isValid()) {
        DomItem *item = static_cast<DomItem*>(index.internalPointer());
        if(item)
            type = item->getType();
    }
    return type;
}

DomItem *DomModel::getTextChildItem(const QModelIndex &index) const
{
    if(index.model()->hasChildren()) {
        DomItem *item = static_cast<DomItem*>(index.child(0,1).internalPointer());
        if(item)
            return item;
    }
    return rootItem;
}

bool DomModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (role != Qt::EditRole) return false;

    bool status;

    DomItem *item = getItem(index);
    qDebug() << "valore del nodo: " << item->node().nodeValue();
    for(int i = 0; i < item->node().childNodes().count(); i++)
        qDebug() << "child " << i << "del nodo " << item->node().childNodes().item(i).nodeValue();

    switch(index.column()) {
    case 0:
        //prova per appendere il nome dello step a fianco al nome del nodo (ovvero name <nomeStep>)
//        if(item->node().childNodes().count() > 0 && item->node().childNodes().item(0).nodeName() == QString("name"))
//        {
//            qDebug() << "TROVATO CHILD DI TIPO name" << item->node().childNodes().item(0).nodeValue();
//            QString nodeValue = value.toString()
//                    + item->node().childNodes().item(0).nodeValue();
//            status = item->setData(nodeValue);
//        } else
            status = item->setData(value);
        status = true;
        break;
    case 1: /* data column */
        if(item->node().childNodes().count() > 0)
            item->node().childNodes().item(0).setNodeValue(value.toString());
        status = true;
        break;
    default:
        status = false;
    }
    emit dataChanged(index, index);
    return status;
}

bool DomModel::DomSave(QTextStream &stream, int indentation)
{
    domDocument.save(stream, indentation);
    return true;
}

bool DomModel::insertRows(int position, int rows, const QModelIndex &parent)
{
    DomItem *parentItem = getItem(parent);

    bool success;
    qDebug() << "position:" << position << " rows:" << rows;
    beginInsertRows(parent, position, position + rows - 1);
    success = parentItem->insertChildren(domDocument, position, parentItem->child(position)->getType());
    endInsertRows();

    return success;
}
