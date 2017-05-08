#include "mainwindow.h"
#include "qdataflowcanvas.h"

#include <QDebug>

class Completion : public QDataflowTextCompletion
{
public:
    Completion()
    {
        classList << "add";
        classList << "node";
        classList << "nodebuffer";
        classList << "norm";
        classList << "trigger";
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
    canvas = new QDataflowCanvas(this);
    canvas->setCompletion(new Completion);
    setCentralWidget(canvas);

    QObject::connect(canvas, &QDataflowCanvas::nodeTextChanged, this, &MainWindow::onNodeTextChanged);
    QObject::connect(canvas, &QDataflowCanvas::nodeAdded, this, &MainWindow::onNodeAdded);
    QObject::connect(canvas, &QDataflowCanvas::nodeRemoved, this, &MainWindow::onNodeRemoved);
    QObject::connect(canvas, &QDataflowCanvas::connectionAdded, this, &MainWindow::onConnectionAdded);
    QObject::connect(canvas, &QDataflowCanvas::connectionRemoved, this, &MainWindow::onConnectionRemoved);

    QDataflowNode *node1 = canvas->add(QPoint(-50, -50), "node 1 2");
    QDataflowNode *node2 = canvas->add(QPoint(-50, 0), "node 1 1");
    QDataflowNode *node3 = canvas->add(QPoint(50, 0), "node 3 2");

    canvas->connect(node1, 1, node2, 0);

    Q_UNUSED(node3);
}

MainWindow::~MainWindow()
{

}

void MainWindow::processNode(QDataflowNode *node)
{
    QString txt = node->text();
    QRegExp rx("(\\ |\\t)");
    QStringList toks = txt.split(rx);
    bool valid = toks[0] == "node";
    int numIn = valid && toks.length() >= 2 ? toks[1].toInt() : node->inletCount();
    int numOut = valid && toks.length() >= 3 ? toks[2].toInt() : node->outletCount();
    node->setInletCount(numIn);
    node->setOutletCount(numOut);
    node->setValid(valid);
}

void MainWindow::onNodeTextChanged(QDataflowNode *node)
{
    qDebug() << "node changed:" << reinterpret_cast<void*>(node);

    processNode(node);
}

void MainWindow::onNodeAdded(QDataflowNode *node)
{
    qDebug() << "node added:" << reinterpret_cast<void*>(node);

    processNode(node);
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
