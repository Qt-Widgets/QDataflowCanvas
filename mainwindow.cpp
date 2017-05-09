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
    QDataflowModelNode *source = new QDataflowModelNode(model, QPoint(100, 50), "source", 0, 0);
    QDataflowModelNode *add = new QDataflowModelNode(model, QPoint(100, 100), "add 5", 0, 0);
    QDataflowModelNode *sink = new QDataflowModelNode(model, QPoint(100, 150), "sink", 0, 0);
    model->addNode(source);
    model->addNode(add);
    model->addNode(sink);
    model->addConnection(source->outlet(0), add->inlet(0));
    model->addConnection(add->outlet(0), sink->inlet(0));
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
