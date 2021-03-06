/*
   Copyright (C) 2005 korewaisai
   korewaisai@yahoo.co.jp

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2
   of the License, or (at your option) any later version.
*/
#include <QtGui>
#include <QScrollBar>

#include "Crontab.h"
#include "VariableView.h"
#include "VariableModel.h"

VariableView::VariableView(VariableModel *model)
{
    variableModel = model;
    setModel(variableModel);
    resizeColumnToContents(0);
    resizeColumnToContents(1);
    setRootIsDecorated(false);

    connect(selectionModel(), &QItemSelectionModel::currentChanged, this, &VariableView::varSelected);
}

void VariableView::resetView()
{
    clearSelection();
    reset();
    resizeColumnToContents(0);
    resizeColumnToContents(1);
    if (variableModel->rowCount(QModelIndex()) > 0)
        setCurrentIndex(variableModel->index(0, 0, QModelIndex()));
    else
        emit changeVar(nullptr);
}

void VariableView::varSelected(const QModelIndex &cur, const QModelIndex &)
{
    if (cur.isValid())
        emit changeVar(variableModel->getVariable(cur));
    else
        emit changeVar(nullptr);
}

void VariableView::varDataChanged()
{
    QModelIndex index = currentIndex();
    if (index.isValid())
        variableModel->varDataChanged(index);
}

void VariableView::insertVariable()
{
    Variable *v = new Variable("*","*","");
    QModelIndex index = currentIndex();
    int pos;
    if (index.isValid())
        pos = index.row();
    else
        pos = 0;
    variableModel->insertVariable(pos, v);
    setCurrentIndex(variableModel->index(pos, 0, QModelIndex()));
}

void VariableView::removeVariable()
{
    QModelIndex index = currentIndex();
    if (index.isValid()) {
        variableModel->removeVariable(index.row());
        if (variableModel->rowCount(QModelIndex()) > 0)
            setCurrentIndex(variableModel->index(0, 0, QModelIndex()));
        else
            emit changeVar(nullptr);
    }
}

void VariableView::scrollTo(const QModelIndex &idx, ScrollHint)
{
    QRect area = viewport()->rect();
    QRect rect = visualRect(idx);
    if (rect.height() == 0) return;
    double step = static_cast<double>(verticalStepsPerItem()) / rect.height();
    if (rect.top() < 0)
        verticalScrollBar()->setValue(
                    verticalScrollBar()->value() + static_cast<int>(rect.top() * step));
    else if (rect.bottom() > area.bottom())
        verticalScrollBar()->setValue(
                    verticalScrollBar()->value() +
                    static_cast<int>((rect.bottom() - area.bottom()) * step) + 5);
}
