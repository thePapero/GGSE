/*! \file domitem.cpp
    \brief An item into the dom.

    Details.
*/
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

#include <QtXml>
#include <QUuid>

//! [0]
//! Il costruttore semplicemente inizializza le proprietà dell'oggetto
DomItem::DomItem(QDomNode &node, int row, DomItem *parent)
{
    domNode = node;
//! [0]
    // Record the item's location within its parent.
//! [1]
    rowNumber = row;
    parentItem = parent;
    //qDebug() << "Aggiungo nodo:" << node.nodeName();
    //agggiungere gli altri type quando pronti
    QString nodeName = domNode.nodeName();
    if(nodeName == QString("step"))
        type = FieldType::step;
    else if(nodeName == QString("startup") || nodeName == QString("main") || nodeName == QString("cleanup"))
        type = FieldType::section;
    else if(nodeName == QString("inputs"))
        type = FieldType::input;
    else if(nodeName == QString("outputs"))
        type = FieldType::output;
    else
        type = FieldType::unknown;
}
//! [1]

//! [2]
DomItem::~DomItem()
{
    //cancella a cascata tutti i child dell'item.
    //QHash<int,DomItem*>::iterator it;
    //for (it = childItems.begin(); it != childItems.end(); ++it)
    //    delete it.value();
    while(!childItems.empty())
        delete childItems.takeFirst();
}
//! [2]

//! [3]
QDomNode DomItem::node() const
{
    return domNode;
}
//! [3]

//! [4]
DomItem *DomItem::parent()
{
    return parentItem;
}
//! [4]

//! [5]
/**
 * @brief DomItem::child
 *        se childItems non contiene il nodo i viene generato un oggetto domItem e aggiunto allla QList dei child.
 * @param i
 * @return il puntatore al child creato.
 */
DomItem *DomItem::child(int i)
{
    //if (childItems.contains(i))
    if(childItems.size() > i)
        return childItems[i];

    if (i >= 0 && i < domNode.childNodes().count()) {
        //prende il nodo i dal DOM
        QDomNode childNode = domNode.childNodes().item(i);
        //crea un oggetto DomItem e lo inserisce nella tabella hash
        DomItem *childItem = new DomItem(childNode, i, this);
        //childItems[i] = childItem;
        childItems.append(childItem);
        return childItem;
    }
    return nullptr;
}
//! [5]

/**
 * @brief DomItem::childInsertAt
 *        Inserisce un nuovo child all'indice i. Tutti gli item successivi vengono scalati di una posizione.
 *        Per adesso non supporta l'inserimento di child alla posizione 0.
 * @param i
 * @return il puntatore al child creato.
 */
DomItem *DomItem::childInsertAt(QDomDocument &doc, int i, FieldType type)
{
    if(i > 0 && i < domNode.childNodes().count() + 1)
    {
        qDebug() << "inserisco l'elemento in posizione:" << i;
        QDomNode childAt = domNode.childNodes().item(i-1);
        QDomNode stepToAdd = createNewElement(type, doc);
        qDebug() << "nodo:" << stepToAdd.childNodes().item(0).nodeName() << "valore:" << stepToAdd.childNodes().item(0).nodeValue();
        domNode.insertAfter(stepToAdd, childAt);
        DomItem *childItem = new DomItem(stepToAdd, i, this);
        childItems.insert(i, childItem);
        //aggiorna la row degli item a seguire.
        for(int it = i+1; it < childItems.size(); it++)
            childItems[it]->setRow(it);
        return childItem;
    }
    return nullptr;
}

//! [6]
int DomItem::row()
{
    return rowNumber;
}
//! [6]

bool DomItem::setRow(int newRow)
{
    rowNumber = newRow;
    return true;
}

bool DomItem::setData(const QVariant &value)
{
    domNode.toElement().setTagName(value.toString());
    return true;

}

FieldType DomItem::getType()
{
    return type;
}

bool DomItem::insertChildren(QDomDocument &doc, int position, FieldType type)
{
    if(position < 0 || position > domNode.childNodes().count())
        return false;

    //creare un metodo simile a child che inserisce il nodo alla posizione i e scala tutti quelli dopo
    //aggiornando rowNumber e la tabella QList.
    //QDomNode childAt = domNode.childNodes().item(position-1);
    //DomItem *childAtItem = new DomItem(childAt, position-1, this);
    //qDebug() << "child a cui ficco appresso il nuovo nodo: " << childAtPos.nodeName();
    switch(type)
    {
    case FieldType::step:
        childInsertAt(doc, position, type);
        return true;
    case FieldType::input:
        return true;
    case FieldType::output:
        return true;
    case FieldType::section:
        //childInsertAt(doc, position);
        return true;
    default:
        return false;
    }
}

bool DomItem::removeChildren(int position, int count)
{
    if(position < 0 || position > domNode.childNodes().count())
        return false;
    for (int row = 0; row < count; ++row)
        delete childItems.takeAt(position);
    return true;
}

QDomElement DomItem::createEmptyStep(QDomDocument &mainDoc)
{
    QDomElement stepElement = mainDoc.createElement(QString("step"));
    QDomElement nameElement = mainDoc.createElement(QString("name"));
    QDomElement familyElement = mainDoc.createElement(QString("family"));
    QDomElement prerunElement = mainDoc.createElement(QString("prerun"));
    QDomElement runElement = mainDoc.createElement(QString("run"));
    QDomElement postrunElement = mainDoc.createElement(QString("postrun"));
    QDomElement runOpCodeElement = mainDoc.createElement(QString("runOpCode"));
    QDomElement bannerElement = mainDoc.createElement(QString("banner"));
    QDomElement skipIfElement = mainDoc.createElement(QString("skipIf"));
    QDomElement typeElement = mainDoc.createElement(QString("type"));
    QDomElement onFailElement = mainDoc.createElement(QString("onFail"));
    QDomElement onErrorElement = mainDoc.createElement(QString("onError"));
    QDomElement logWhenElement = mainDoc.createElement(QString("logWhen"));
    QDomElement inputsElement = mainDoc.createElement(QString("inputs"));
    QDomElement outputsElement = mainDoc.createElement(QString("outputs"));
    QDomElement conditionsElement = mainDoc.createElement(QString("conditions"));
    QDomElement GUUID_Element = mainDoc.createElement(QString("GUUID"));
    QDomText GUUID_Value = mainDoc.createTextNode(QString(QUuid().toString()));

    GUUID_Element.appendChild(GUUID_Value);
    stepElement.appendChild(nameElement);
    stepElement.appendChild(familyElement);
    stepElement.appendChild(prerunElement);
    stepElement.appendChild(runElement);
    stepElement.appendChild(postrunElement);
    stepElement.appendChild(runOpCodeElement);
    stepElement.appendChild(bannerElement);
    stepElement.appendChild(skipIfElement);
    stepElement.appendChild(typeElement);
    stepElement.appendChild(onFailElement);
    stepElement.appendChild(onErrorElement);
    stepElement.appendChild(logWhenElement);
    stepElement.appendChild(inputsElement);
    stepElement.appendChild(outputsElement);
    stepElement.appendChild(conditionsElement);
    stepElement.appendChild(GUUID_Element);

    return stepElement;
}

QDomElement DomItem::createEmptyInputs(QDomDocument &mainDoc)
{
    QDomElement inputsElement = mainDoc.createElement(QString("input"));
    QDomElement nameElement = mainDoc.createElement(QString("name"));
    QDomElement typeElement = mainDoc.createElement(QString("type"));
    QDomElement referenceElement = mainDoc.createElement(QString("reference"));

    inputsElement.appendChild(nameElement);
    inputsElement.appendChild(nameElement);
    inputsElement.appendChild(referenceElement);

    return inputsElement;
}

QDomElement DomItem::createEmptyOutputs(QDomDocument &mainDoc)
{
    QDomElement outputElement = mainDoc.createElement(QString("input"));
    QDomElement nameElement = mainDoc.createElement(QString("name"));
    QDomElement typeElement = mainDoc.createElement(QString("type"));
    QDomElement referenceElement = mainDoc.createElement(QString("reference"));

    outputElement.appendChild(nameElement);
    outputElement.appendChild(nameElement);
    outputElement.appendChild(referenceElement);

    return outputElement;
}

QDomElement DomItem::createNewElement(FieldType type, QDomDocument &mainDoc)
{
    QDomElement element;
    switch(type)
    {
    case FieldType::step:
        element = createEmptyStep(mainDoc);
        break;
    case FieldType::input:
        element = createEmptyInputs(mainDoc);
        break;
    case FieldType::output:
        element = createEmptyOutputs(mainDoc);
        break;
    case FieldType::section:
        break;
    default:
        break;
    }
    return element;
}
