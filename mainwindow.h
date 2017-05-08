#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

class QDataflowCanvas;
class QDataflowNode;

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
};

#endif // MAINWINDOW_H
