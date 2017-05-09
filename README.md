# QDataflowCanvas

A Qt widget for displaying and editing a dataflow graph.

It is based on the [Graphics View Framework](http://doc.qt.io/qt-5/graphicsview.html) of [Qt](https://www.qt.io), and it used the [Model/View architecture](http://doc.qt.io/qt-5/model-view-programming.html).

# Building a Dataflow application

![screenshot](/screenshot.png?raw=true)

Start by creating a UI with a QSpinBox and a QPushButton which will be used to send input to one of the nodes (the `source` object). Create also a disabled QLineEdit which will be used to display the data received by the `sink` object.

The `dataflowMetaObject` is a volatile field used to attach the dataflow logic to the graph.

We create a subclass of `QDataflowMetaObject` for the `DFSource` object. In the `bool init(QStringList args)` method we validate the object and set up the number of inlets/outlets according to object's functionality:

```C++
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
```

Similarily, we create a subclass of `QDataflowMetaObject` for the `DFSink` object. The `void onDataReceved(int inlet, void *data)` method will be called when data is received on an inlet. In the case of the `sink` object, we want to display the data in the `QLineEdit` object:

```C++
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
```

Now we create an object to perform mathematical operations. Instead of creating many classes, we create just one subclass performing various math operations, depending on the value of the `op` field:

```C++
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
```

The `DFMathBinOp` object has two inlets, because it implements binary mathematical operators. If we want to compute `2 + 3`, we first send `3` to the right inlet, which will store `3` in its internal status variable, and then send `2` to the left inlet, which will trigger the computation and output the result on the first outlet.

This pattern is common in dataflow programming environments: the leftmost inlet (which will trigger the output) is the "hot" inlet, and the other inlets are "cold" inlets.

Now we need to attach the correct `QDataflowMetaObject` to the newly created instances of `QDataflowModelNode`. To do so, we create the `void setupNode(QDataflowModelNode *node)` method, which will parse (tokenize) the object creation arguments, assign dataflowMetaObject, and mark the object valid:

```C++
void MainWindow::setupNode(QDataflowModelNode *node)
{
    QStringList toks = node->text().split(QRegExp("(\\ |\\t)"));
    if(!classList.contains(toks[0]))
    {
```
classList is a `QStringList` with all the available dataflow object classes. It will be used also for completion.
```C++
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
```

The next thing we need is a method to feed data to the `source` object:

```C++
void MainWindow::processData()
{
    long x = input->value();
    sourceNode->dataflowMetaObject()->sendData(0, reinterpret_cast<void*>(x));
}
```

And now proceed to the creation of the `QDataflowCanvas`:

```C++
QDataflowCanvas *canvas = new QDataflowCanvas(mainWindow);
```

initialize completion:

```C++
QStringList classList;
classList << "add" << "sub" << "mul" << "div" << "pow" << "source" << "sink";
canvas->setCompletion(this);
```

where `this` inherits also from `QDataflowTextCompletion` and implements the `void complete(QString txt, QStringList &list)` method:

```C++
void MainWindow::complete(QString txt, QStringList &completionList)
{
    foreach(QString className, classList)
        if(className.startsWith(txt))
            completionList << className;
}
```

and connect signals:

```C++
QObject::connect(sendButton, &QPushButton::clicked, this, &MainWindow::processData);
QObject::connect(model, &QDataflowModel::nodeTextChanged, this, &MainWindow::onNodeTextChanged);
QObject::connect(model, &QDataflowModel::nodeAdded, this, &MainWindow::onNodeAdded);
```

when a node is created or changed, `void setupNode(QDataflowModelNode *node)` is called (again):

```C++
void MainWindow::onNodeAdded(QDataflowModelNode *node)
{
    setupNode(node);
}

void MainWindow::onNodeTextChanged(QDataflowModelNode *node, QString text)
{
    Q_UNUSED(text);
    setupNode(node);
}
```

See mainwindow.ui/h/cpp for a complete example.


