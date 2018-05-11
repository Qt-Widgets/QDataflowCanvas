/* QDataflowCanvas - a dataflow widget for Qt
 * Copyright (C) 2017 Federico Ferri
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
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
    void onDumpModel();
};

#endif // MAINWINDOW_H
