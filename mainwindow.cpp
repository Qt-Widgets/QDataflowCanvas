/* QDataflowCanvas - a dataflow widget for Qt
 * Copyright (C) 2017 Federico Ferri
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "mainwindow.h"
#define _USE_MATH_DEFINES
#include <math.h>
#include <QDebug>

class DFSource : public QDataflowMetaObject
{
public:
    bool init(QStringList args)
    {
        Q_UNUSED(args);

        setInletCount(0);
        setOutletCount(1);

        return true;
    }
};

class DFMathBinOp : public QDataflowMetaObject
{
public:
    bool init(QStringList args)
    {
        s = 0;

        setInletCount(2);
        setOutletCount(1);

        op = args[0];

        if(args.length() > 1)
            s = args[1].toLong();

        return true;
    }

    void onDataReceved(int inlet, void *data)
    {
        if(inlet == 0)
        {
            int r = reinterpret_cast<long>(data);
            if(op == "add") r = r + s;
            if(op == "sub") r = r - s;
            if(op == "mul") r = r * s;
            if(op == "div") r = r / s;
            if(op == "pow") r = pow(r, s);
            sendData(0, reinterpret_cast<void*>(r));
        }
        else if(inlet == 1)
        {
            s = reinterpret_cast<long>(data);
        }
    }

private:
    QString op;
    int s;
};

class DFSink : public QDataflowMetaObject
{
public:
    DFSink(QLineEdit *e) : e_(e) {}

    bool init(QStringList args)
    {
        Q_UNUSED(args);

        setInletCount(1);
        setOutletCount(0);

        return true;
    }

    void onDataReceved(int inlet, void *data)
    {
        if(inlet == 0)
        {
            e_->setText(QString::number(reinterpret_cast<long>(data)));
        }
    }

private:
    QLineEdit *e_;
};

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setupUi(this);

    classList << "add" << "sub" << "mul" << "div" << "pow" << "source" << "sink";
    canvas->setCompletion(this);

    QDataflowModel *model = canvas->model();

    new QDataflowModelDebugSignals(model);

    QObject::connect(sendButton, &QPushButton::clicked, this, &MainWindow::processData);
    QObject::connect(model, &QDataflowModel::nodeTextChanged, this, &MainWindow::onNodeTextChanged);
    QObject::connect(model, &QDataflowModel::nodeAdded, this, &MainWindow::onNodeAdded);

    // set up a small dataflow graph:
    QDataflowModelNode *source = model->create(QPoint(100, 50), "source", 0, 0);
    QDataflowModelNode *add = model->create(QPoint(100, 100), "add 5", 0, 0);
    QDataflowModelNode *sink = model->create(QPoint(100, 150), "sink", 0, 0);
    model->connect(source, 0, add, 0);
    model->connect(add, 0, sink, 0);
}

MainWindow::~MainWindow()
{

}

void MainWindow::complete(QString txt, QStringList &completionList)
{
    foreach(QString className, classList)
        if(className.startsWith(txt) && className.length() > txt.length())
            completionList << className;
}

void MainWindow::setupNode(QDataflowModelNode *node)
{
    QStringList toks = node->text().split(QRegExp("(\\ |\\t)"));
    if(!classList.contains(toks[0]))
    {
        node->setValid(false);
        return;
    }
    if(toks[0] == "source") {sourceNode = node; node->setDataflowMetaObject(new DFSource());}
    else if(toks[0] == "sink") node->setDataflowMetaObject(new DFSink(result));
    else node->setDataflowMetaObject(new DFMathBinOp());
    bool ok = node->dataflowMetaObject()->init(toks);
    if(!ok)
    {
        qDebug() << "initialization failed for:" << node->text();
    }
    node->setValid(ok);
}

void MainWindow::processData()
{
    long x = input->value();
    sourceNode->dataflowMetaObject()->sendData(0, reinterpret_cast<void*>(x));
}

void MainWindow::onNodeAdded(QDataflowModelNode *node)
{
    setupNode(node);
}

void MainWindow::onNodeTextChanged(QDataflowModelNode *node, QString text)
{
    Q_UNUSED(text);
    setupNode(node);
}
