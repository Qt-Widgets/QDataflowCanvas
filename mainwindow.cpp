#include "mainwindow.h"
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
            outlet(0)->sendData(reinterpret_cast<void*>(r));
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

    QObject::connect(sendButton, &QPushButton::clicked, this, &MainWindow::processData);
    QObject::connect(canvas, &QDataflowCanvas::nodeTextChanged, this, &MainWindow::onNodeTextChanged);
    QObject::connect(canvas, &QDataflowCanvas::nodeAdded, this, &MainWindow::onNodeAdded);

    // set up a small dataflow graph:
    QDataflowNode *source = canvas->add(QPoint(100, 50), "source");
    QDataflowNode *add = canvas->add(QPoint(100, 100), "add 5");
    QDataflowNode *sink = canvas->add(QPoint(100, 150), "sink");
    canvas->connect(source, 0, add, 0);
    canvas->connect(add, 0, sink, 0);
}

MainWindow::~MainWindow()
{

}

void MainWindow::complete(QString txt, QStringList &completionList)
{
    foreach(QString className, classList)
        if(className.startsWith(txt))
            completionList << className;
}

void MainWindow::setupNode(QDataflowNode *node)
{
    QStringList toks = node->text().split(QRegExp("(\\ |\\t)"));
    if(!classList.contains(toks[0]))
    {
        node->setValid(false);
        return;
    }
    if(toks[0] == "source") {sourceNode = node; node->setMetaObject(new DFSource());}
    else if(toks[0] == "sink") node->setMetaObject(new DFSink(result));
    else node->setMetaObject(new DFMathBinOp());
    node->setValid(node->metaObject()->init(toks));
}

void MainWindow::processData()
{
    long x = input->value();
    sourceNode->outlet(0)->sendData(reinterpret_cast<void*>(x));
}

void MainWindow::onNodeTextChanged(QDataflowNode *node)
{
    setupNode(node);
}

void MainWindow::onNodeAdded(QDataflowNode *node)
{
    setupNode(node);
}
