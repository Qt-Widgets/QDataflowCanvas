#include "mainwindow.h"
#include "qdataflowcanvas.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    QDataflowCanvas *canvas = new QDataflowCanvas(this);
    setCentralWidget(canvas);

    QDataflowNode *node1 = new QDataflowNode(canvas, "node1", 1, 2);
    node1->setPos(-50, -50);
    canvas->addItem(node1);

    QDataflowNode *node2 = new QDataflowNode(canvas, "node2", 1, 1);
    node2->setPos(-50, 0);
    canvas->addItem(node2);

    QDataflowNode *node3 = new QDataflowNode(canvas, "node3", 3, 2);
    node3->setPos(50, 0);
    canvas->addItem(node3);

    canvas->addItem(new QDataflowConnection(node1->outlet(1), node2->inlet(0)));
}

MainWindow::~MainWindow()
{

}
