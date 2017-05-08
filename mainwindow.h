#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "ui_mainwindow.h"
#include "qdataflowcanvas.h"

class MainWindow : public QMainWindow, private Ui::MainWindow, public QDataflowTextCompletion
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();

    void complete(QString txt, QStringList &completionList) override;

private:
    QDataflowNode *sourceNode;
    QStringList classList;

private slots:
    void setupNode(QDataflowNode *node);
    void processData();
    void onNodeTextChanged(QDataflowNode *node);
    void onNodeAdded(QDataflowNode *node);
};

#endif // MAINWINDOW_H
