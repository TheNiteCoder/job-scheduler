/*
   Copyright (C) 2005 korewaisai
   korewaisai@yahoo.co.jp

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2
   of the License, or (at your option) any later version.
*/
#include <QLabel>
#include <QPushButton>
#include <QSpinBox>
#include <QVBoxLayout>

#include "Execute.h"
#include "ExecuteList.h"
#include "ExecuteView.h"
#include "ExecuteModel.h"
#include "Crontab.h"
#include "CronTime.h"

ExecuteList::ExecuteList(int maxN, int maxD, QList<Crontab*> *cron)
    :maxNum(maxN), maxDate(maxD), crontabs(cron)
{
    executeModel = new ExecuteModel(&executes);
    QHBoxLayout *h;
    QPushButton *resetButton;
    QSpinBox *numSpinBox = new QSpinBox;
    QSpinBox *dateSpinBox = new QSpinBox;

    QVBoxLayout *mainLayout = new QVBoxLayout;
    {
        mainLayout->addWidget(executeView=new ExecuteView(executeModel));
        mainLayout->addLayout((h = new QHBoxLayout));
        {
            h->addWidget(new QLabel(tr("Max Item")));
            h->addWidget(numSpinBox);
            h->addWidget(new QLabel(tr("Max Date")));
            h->addWidget(dateSpinBox);
            h->addWidget(new QLabel(tr("Select")));
            h->addWidget(countLabel=new QLabel(""));
            h->addStretch();
            h->addWidget(resetButton =
                    new QPushButton(QIcon::fromTheme("undo", QIcon(":/images/undo_small.png")), tr("&Reset")));
        }
    }
    //	mainLayout->setMargin(0);
    setLayout(mainLayout);

    numSpinBox->setRange(1,999);
    numSpinBox->setSingleStep(10);
    numSpinBox->setValue(maxNum);
    dateSpinBox->setRange(1,999);
    dateSpinBox->setValue(maxDate);
    countLabel->setFrameStyle(QFrame::Panel | QFrame::Sunken);

    connect(resetButton, &QPushButton::clicked, this, &ExecuteList::dataChanged);
    connect(numSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &ExecuteList::numChanged);
    connect(dateSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &ExecuteList::dateChanged);

    curCrontab = nullptr;
    curTCommand = nullptr;

}

void ExecuteList::dataChanged()
{

    if (!isVisible())
        return;

    QDateTime stopTime = QDateTime::currentDateTime().addDays(maxDate);

    executeView->clearSelection();
    for (auto& e : executes) delete e;
    executes.clear();
    QList<TCommand*> cmnd;
    QList<QDateTime> date;
    for (const auto& cron : *crontabs) {
        for (const auto& cc : cron->tCommands) {
            CronTime ct(cc->time);
            if (ct.isValid()) {
                cmnd << cc;
                date << ct.getNextTime(QDateTime::currentDateTime());
            } else {
                executes << new Execute(cc, "Time Format Error", -1);
            }
        }
    }
    itemCount = 0;
    if (cmnd.count() > 0) {
        for (int i = 0; i < maxNum; ++i) {
            int p = 0;
            QDateTime cur = date.at(0);
            for (int j=1; j < cmnd.count(); j++) {
                if (cur > date.at(j)) {
                    cur = date.at(j);
                    p = j;
                }
            }
            if (cur > stopTime)
                break;
            executes << new Execute( cmnd[p], cur.toString("yyyy-MM-dd(ddd) hh:mm") );
            itemCount++;
            date[p] = CronTime(cmnd[p]->time).getNextTime(cur);
        }
    }

    executeView->hideUser(crontabs->count() == 1);

    changeCurrent(curCrontab, curTCommand);
    executeView->resetView();
}

void ExecuteList::changeCurrent(Crontab *cron, TCommand *cmnd)
{
    curCrontab = cron;
    curTCommand = cmnd;
    if (!isVisible())
        return;

    for (auto& e : executes)
        e->sel = 0;

    int sel = 0;
    if (crontabs->count() > 1 && cron != nullptr)
        for (auto& e : executes)
            if (reinterpret_cast<uintptr_t>(e->tCommands->parent) == reinterpret_cast<uintptr_t>(cron))
                e->sel = 1;

    if (cmnd != nullptr)
        for (auto& e : executes)
            if (reinterpret_cast<uintptr_t>(e->tCommands) == reinterpret_cast<uintptr_t>(cmnd)) {
                e->sel = 2;
                sel++;
            }

    countLabel->setText(QString("%1/%2").arg(sel).arg(itemCount));
    executeModel->doSort();
}

void ExecuteList::numChanged(int num)
{
    maxNum = num;
}

void ExecuteList::dateChanged(int num)
{
    maxDate = num;
}

void ExecuteList::setVisible(bool flag)
{
    QWidget::setVisible(flag);

    if (flag)
        dataChanged();
}
