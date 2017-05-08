#include "mainwindow.h"
#include "qdataflowcanvas.h"

#include <math.h>

#include <QDebug>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QSpinBox>
#include <QPushButton>
#include <QLineEdit>

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

        if(args.length() > 1)
            s = args[1].toLong();

        if(args[0] == "add") op = '+';
        else if(args[0] == "sub") op = '-';
        else if(args[0] == "mul") op = '*';
        else if(args[0] == "div") op = '/';
        else if(args[0] == "pow") op = '^';
        else return false;

        return true;
    }

    void onDataReceved(int inlet, void *data)
    {
        if(inlet == 0)
        {
            int r = reinterpret_cast<long>(data);
            if(op == '+') r = r + s;
            if(op == '-') r = r - s;
            if(op == '*') r = r * s;
            if(op == '/') r = r / s;
            if(op == '^') r = pow(r, s);
            outlet(0)->sendData(reinterpret_cast<void*>(r));
        }
        else if(inlet == 1)
        {
            s = reinterpret_cast<long>(data);
        }
    }

private:
    int s;
    QChar op;
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

class Completion : public QDataflowTextCompletion
{
public:
    Completion()
    {
        classList << "add" << "sub" << "mul" << "div" << "pow" << "source" << "sink";
    }

    void complete(QString txt, QStringList &completionList)
    {
        foreach(QString className, classList)
        {
            if(className.startsWith(txt))
                completionList << className;
        }
    }

private:
    QStringList classList;
};

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    QVBoxLayout *layout = new QVBoxLayout();
    QHBoxLayout *hbox = new QHBoxLayout();
    spinBox = new QSpinBox();
    hbox->addWidget(spinBox);
    QPushButton *btnSend = new QPushButton("Send");
    hbox->addWidget(btnSend);
    layout->addLayout(hbox);
    canvas = new QDataflowCanvas(this);
    canvas->setCompletion(new Completion);
    layout->addWidget(canvas);
    txtResult = new QLineEdit();
    txtResult->setReadOnly(true);
    layout->addWidget(txtResult);
    QWidget *w = new QWidget();
    w->setLayout(layout);
    setCentralWidget(w);

    QObject::connect(btnSend, &QPushButton::clicked, this, &MainWindow::processData);

    QObject::connect(canvas, &QDataflowCanvas::nodeTextChanged, this, &MainWindow::onNodeTextChanged);
    QObject::connect(canvas, &QDataflowCanvas::nodeAdded, this, &MainWindow::onNodeAdded);
    QObject::connect(canvas, &QDataflowCanvas::nodeRemoved, this, &MainWindow::onNodeRemoved);
    QObject::connect(canvas, &QDataflowCanvas::connectionAdded, this, &MainWindow::onConnectionAdded);
    QObject::connect(canvas, &QDataflowCanvas::connectionRemoved, this, &MainWindow::onConnectionRemoved);

    QDataflowNode *source = canvas->add(QPoint(100, 50), "source");
    source->setMetaObject(new DFSource());
    QDataflowNode *add = canvas->add(QPoint(100, 100), "add 5");
    QDataflowNode *sink = canvas->add(QPoint(100, 150), "sink");

    canvas->connect(source, 0, add, 0);
    canvas->connect(add, 0, sink, 0);

    sourceNode = source;
}

MainWindow::~MainWindow()
{

}

void MainWindow::setupNode(QDataflowNode *node)
{
    QString txt = node->text();
    QRegExp rx("(\\ |\\t)");
    QStringList toks = txt.split(rx);

    if(toks[0] == "source")
    {
        node->setMetaObject(new DFSource());
    }
    else if(toks[0] == "add" || toks[0] == "sub" || toks[0] == "mul" || toks[0] == "div" || toks[0] == "pow")
    {
        node->setMetaObject(new DFMathBinOp());
    }
    else if(toks[0] == "sink")
    {
        node->setMetaObject(new DFSink(txtResult));
    }
    else
    {
        node->setValid(false);
        return;
    }
    node->setValid(true);
    if(!node->metaObject()->init(toks))
    {
        node->setValid(false);
        node->setMetaObject(0L);
        return;
    }
}

void MainWindow::processData()
{
    long x = spinBox->value();
    sourceNode->outlet(0)->sendData(reinterpret_cast<void*>(x));
}

void MainWindow::onNodeTextChanged(QDataflowNode *node)
{
    qDebug() << "node changed:" << reinterpret_cast<void*>(node);

    setupNode(node);
}

void MainWindow::onNodeAdded(QDataflowNode *node)
{
    qDebug() << "node added:" << reinterpret_cast<void*>(node);

    setupNode(node);
}

void MainWindow::onNodeRemoved(QDataflowNode *node)
{
    qDebug() << "node removed:" << reinterpret_cast<void*>(node);
}

void MainWindow::onConnectionAdded(QDataflowConnection *conn)
{
    qDebug() << "connection added:" << reinterpret_cast<void*>(conn);
}

void MainWindow::onConnectionRemoved(QDataflowConnection *conn)
{
    qDebug() << "connection removed:" << reinterpret_cast<void*>(conn);
}
