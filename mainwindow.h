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
    QDataflowModelNode *sourceNode;
    QStringList classList;

private slots:
    void setupNode(QDataflowModelNode *node);
    void processData();
    void onNodeAdded(QDataflowModelNode *node);
    void onNodeTextChanged(QDataflowModelNode *node, QString text);
};

#endif // MAINWINDOW_H
