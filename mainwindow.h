#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

class QDataflowCanvas;
class QDataflowNode;
class QDataflowConnection;

class QSpinBox;
class QLineEdit;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();

    void setupNode(QDataflowNode *node);
    void processData();

private:
    QDataflowCanvas *canvas;
    QDataflowNode *sourceNode;
    QSpinBox *spinBox;
    QLineEdit *txtResult;

public slots:
    void onNodeTextChanged(QDataflowNode *node);
    void onNodeAdded(QDataflowNode *node);
    void onNodeRemoved(QDataflowNode *node);
    void onConnectionAdded(QDataflowConnection *conn);
    void onConnectionRemoved(QDataflowConnection *conn);
};

#endif // MAINWINDOW_H
