#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

class QDataflowCanvas;
class QDataflowNode;
class QDataflowConnection;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();

    void processNode(QDataflowNode *node);

private:
    QDataflowCanvas *canvas;

public slots:
    void onNodeTextChanged(QDataflowNode *node);
    void onNodeAdded(QDataflowNode *node);
    void onNodeRemoved(QDataflowNode *node);
    void onConnectionAdded(QDataflowConnection *conn);
    void onConnectionRemoved(QDataflowConnection *conn);
};

#endif // MAINWINDOW_H
