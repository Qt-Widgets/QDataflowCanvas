#ifndef QDATAFLOWMODEL_H
#define QDATAFLOWMODEL_H

#include <QObject>
#include <QSet>
#include <QList>
#include <QPoint>
#include <QString>
#include <QDebug>

class QDataflowModelNode;
class QDataflowModelIOlet;
class QDataflowModelInlet;
class QDataflowModelOutlet;
class QDataflowModelConnection;
class QDataflowMetaObject;

class QDataflowModel : public QObject
{
    Q_OBJECT
public:
    explicit QDataflowModel(QObject *parent = 0);

    void addNode(QDataflowModelNode *node);
    void removeNode(QDataflowModelNode *node);
    void addConnection(QDataflowModelConnection *conn);
    void addConnection(QDataflowModelOutlet *outlet, QDataflowModelInlet *inlet);
    void addConnection(QDataflowModelNode *sourceNode, int sourceOutlet, QDataflowModelNode *destNode, int destInlet);
    void removeConnection(QDataflowModelConnection *conn);
    void removeConnection(QDataflowModelOutlet *outlet, QDataflowModelInlet *inlet);
    void removeConnection(QDataflowModelNode *sourceNode, int sourceOutlet, QDataflowModelNode *destNode, int destInlet);
    QList<QDataflowModelConnection*> findConnections(QDataflowModelConnection *conn) const;
    QList<QDataflowModelConnection*> findConnections(QDataflowModelNode *sourceNode, int sourceOutlet, QDataflowModelNode *destNode, int destInlet) const;
    QList<QDataflowModelConnection*> findConnections(QDataflowModelOutlet *source, QDataflowModelInlet *dest) const;

signals:
    void nodeAdded(QDataflowModelNode *node);
    void nodeRemoved(QDataflowModelNode *node);
    void nodeValidChanged(QDataflowModelNode *node, bool valid);
    void nodePosChanged(QDataflowModelNode *node, QPoint pos);
    void nodeTextChanged(QDataflowModelNode *node, QString text);
    void nodeInletCountChanged(QDataflowModelNode *node, int count);
    void nodeOutletCountChanged(QDataflowModelNode *node, int count);
    void connectionAdded(QDataflowModelConnection *conn);
    void connectionRemoved(QDataflowModelConnection *conn);

public slots:

private slots:
    void onValidChanged(bool valid);
    void onPosChanged(QPoint pos);
    void onTextChanged(QString text);
    void onInletCountChanged(int count);
    void onOutletCountChanged(int count);

private:
    QSet<QDataflowModelNode*> nodes_;
    QSet<QDataflowModelConnection*> connections_;
};

class QDataflowModelNode : public QObject
{
    Q_OBJECT
public:
    explicit QDataflowModelNode(QDataflowModel *parent, QPoint pos, QString text, int inletCount, int outletCount);

    QDataflowModel * model();

    QDataflowMetaObject * dataflowMetaObject() const;
    void setDataflowMetaObject(QDataflowMetaObject *dataflowMetaObject);

    bool isValid() const;
    void setValid(bool valid);
    QPoint pos() const;
    QString text() const;
    QList<QDataflowModelInlet*> inlets() const;
    QDataflowModelInlet * inlet(int index) const;
    int inletCount() const;
    QList<QDataflowModelOutlet*> outlets() const;
    QDataflowModelOutlet * outlet(int index) const;
    int outletCount() const;

signals:
    void validChanged(bool valid);
    void posChanged(QPoint pos);
    void textChanged(QString text);
    void inletCountChanged(int count);
    void outletCountChanged(int count);

public slots:
    void setPos(QPoint pos);
    void setText(const QString &text);
    void addInlet();
    void removeLastInlet();
    void setInletCount(int count);
    void addOutlet();
    void removeLastOutlet();
    void setOutletCount(int count);

protected slots:
    void addInlet(QDataflowModelInlet *inlet);
    void addOutlet(QDataflowModelOutlet *outlet);

private:
    bool valid_;
    QPoint pos_;
    QString text_;
    QList<QDataflowModelInlet*> inlets_;
    QList<QDataflowModelOutlet*> outlets_;
    QDataflowMetaObject *dataflowMetaObject_;
};

QDebug operator<<(QDebug debug, const QDataflowModelNode &node);
QDebug operator<<(QDebug debug, const QDataflowModelNode *node);

class QDataflowModelIOlet : public QObject
{
    Q_OBJECT
public:
    explicit QDataflowModelIOlet(QDataflowModelNode *parent, int index);

    QDataflowModel * model();

    QDataflowModelNode * node() const;
    int index() const;

    void addConnection(QDataflowModelConnection *conn);
    QList<QDataflowModelConnection*> connections() const;

signals:

public slots:

private:
    QList<QDataflowModelConnection*> connections_;
    int index_;
};

class QDataflowModelInlet : public QDataflowModelIOlet
{
    Q_OBJECT
public:
    explicit QDataflowModelInlet(QDataflowModelNode *parent, int index);

signals:

public slots:

private:
};

QDebug operator<<(QDebug debug, const QDataflowModelInlet &inlet);
QDebug operator<<(QDebug debug, const QDataflowModelInlet *inlet);

class QDataflowModelOutlet : public QDataflowModelIOlet
{
    Q_OBJECT
public:
    explicit QDataflowModelOutlet(QDataflowModelNode *parent, int index);

signals:

public slots:

private:
};

QDebug operator<<(QDebug debug, const QDataflowModelOutlet &outlet);
QDebug operator<<(QDebug debug, const QDataflowModelOutlet *outlet);

class QDataflowModelConnection : public QObject
{
    Q_OBJECT
public:
    explicit QDataflowModelConnection(QDataflowModel *parent, QDataflowModelOutlet *source, QDataflowModelInlet *dest);

    QDataflowModel * model();

    QDataflowModelOutlet * source() const;
    QDataflowModelInlet * dest() const;

signals:

public slots:

private:
    QDataflowModelOutlet *source_;
    QDataflowModelInlet *dest_;
};

QDebug operator<<(QDebug debug, const QDataflowModelConnection &conn);
QDebug operator<<(QDebug debug, const QDataflowModelConnection *conn);

class QDataflowMetaObject
{
public:
    virtual ~QDataflowMetaObject() {}

    virtual bool init(QStringList args);
    QDataflowModelNode * node() {return node_;}
    QDataflowModelInlet * inlet(int index) {return node_->inlet(index);}
    QDataflowModelOutlet * outlet(int index) {return node_->outlet(index);}
    int inletCount() {return node_->inletCount();}
    void setInletCount(int c) {node_->setInletCount(c);}
    int outletCount() {return node_->outletCount();}
    void setOutletCount(int c) {node_->setOutletCount(c);}
    virtual void onDataReceved(int inlet, void *data);
    void sendData(int outlet, void *data);

private:
    QDataflowModelNode *node_;

    friend class QDataflowModelNode;
};

class QDataflowModelDebugSignals : public QObject
{
    Q_OBJECT
public:
    explicit QDataflowModelDebugSignals(QDataflowModel *parent);

private:
    QDebug debug() const;

private slots:
    void onNodeAdded(QDataflowModelNode *node);
    void onNodeRemoved(QDataflowModelNode *node);
    void onNodePosChanged(QDataflowModelNode *node, QPoint pos);
    void onNodeTextChanged(QDataflowModelNode *node, QString text);
    void onNodeInletCountChanged(QDataflowModelNode *node, int count);
    void onNodeOutletCountChanged(QDataflowModelNode *node, int count);
    void onConnectionAdded(QDataflowModelConnection *conn);
    void onConnectionRemoved(QDataflowModelConnection *conn);
};

#endif // QDATAFLOWMODEL_H
