#ifndef QDATAFLOWCANVAS_H
#define QDATAFLOWCANVAS_H

#include <QGraphicsScene>
#include <QGraphicsItem>
#include <QGraphicsView>
#include <QGraphicsSceneMouseEvent>
#include <QMouseEvent>
#include <QLineEdit>

class QDataflowNode;
class QDataflowInlet;
class QDataflowOutlet;
class QDataflowConnection;
class QDataflowTextCompletion;
class QDataflowNodeTextLabel;
class QDataflowMetaObject;

enum QDataflowItemType {
    QDataflowItemTypeNode = QGraphicsItem::UserType + 1,
    QDataflowItemTypeConnection = QGraphicsItem::UserType + 2,
    QDataflowItemTypeInlet = QGraphicsItem::UserType + 3,
    QDataflowItemTypeOutlet = QGraphicsItem::UserType + 4
};

class QDataflowCanvas : public QGraphicsView
{
    Q_OBJECT
public:
    QDataflowCanvas(QWidget *parent);
    virtual ~QDataflowCanvas();

    QDataflowNode * add(QPoint pos, const QString &txt, int numInlets = 0, int numOutlets = 0);
    void remove(QDataflowNode *node);

    void connect(QDataflowNode *source, int outletIndex, QDataflowNode *dest, int inletIndex);
    void disconnect(QDataflowNode *source, int outletIndex, QDataflowNode *dest, int inletIndex);

    QDataflowTextCompletion * completion() const {return completion_;}
    void setCompletion(QDataflowTextCompletion *completion) {completion_ = completion;}

protected:
    void addNode(QDataflowNode *node);
    void addConnection(QDataflowConnection *conn);
    void removeNode(QDataflowNode *node);
    void removeConnection(QDataflowConnection *conn);

public:
    void raiseItem(QGraphicsItem *item);

protected:
    template<typename T>
    T * itemAtT(const QPointF &point);

    void notifyNodeTextChanged(QDataflowNode *node);
    void notifyNodeAdded(QDataflowNode *node);
    void notifyNodeRemoved(QDataflowNode *node);
    void notifyConnectionAdded(QDataflowConnection *conn);
    void notifyConnectionRemoved(QDataflowConnection *conn);

    void mouseDoubleClickEvent(QMouseEvent *event) override;

public slots:
    void itemTextEditorTextChange();

signals:
    void nodeTextChanged(QDataflowNode *node);
    void nodeAdded(QDataflowNode *node);
    void nodeRemoved(QDataflowNode *node);
    void connectionAdded(QDataflowConnection *conn);
    void connectionRemoved(QDataflowConnection *conn);

    friend class QDataflowNode;
    friend class QDataflowIOlet;
    friend class QDataflowInlet;
    friend class QDataflowOutlet;
    friend class QDataflowConnection;

private:
    QDataflowTextCompletion *completion_;
    QSet<QDataflowNode*> ownedNodes_;
    QSet<QDataflowConnection*> ownedConnections_;
};

class QDataflowNode : public QGraphicsItem
{
protected:
    QDataflowNode(QDataflowCanvas *canvas_, QString text = "", int numInlets = 0, int numOutlets = 0, bool valid = true);

public:
    QDataflowInlet * inlet(int index) const {return inlets_.at(index);}
    int inletCount() const {return inlets_.size();}
    void setInletCount(int count, bool skipAdjust = false);
    QDataflowOutlet * outlet(int index) const {return outlets_.at(index);}
    int outletCount() const {return outlets_.size();}
    void setOutletCount(int count, bool skipAdjust = false);

    QDataflowMetaObject * metaObject() const {return metaObject_;}
    void setMetaObject(QDataflowMetaObject *metaObject);

    int type() const override {return QDataflowItemTypeNode;}

    void onDataReceved(int inlet, void *data);

    void setText(QString text);
    QString text() const;

    void setValid(bool valid);
    bool isValid() const;

    void adjustConnections() const;

    QRectF boundingRect() const override;

    void adjust();

    QDataflowCanvas * canvas() const {return canvas_;}

    qreal ioletWidth() const {return 10;}
    qreal ioletHeight() const {return 3;}
    qreal ioletSpacing() const {return 13;}
    qreal inletsWidth() const;
    qreal outletsWidth() const;
    QPen objectPen() const;
    QBrush objectBrush() const;
    QBrush headerBrush() const;

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

    void enterEditMode();
    void exitEditMode(bool revertText);
    bool isInEditMode() const;

protected:
    QVariant itemChange(GraphicsItemChange change, const QVariant &value) override;

    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event) override;

    void keyPressEvent(QKeyEvent *event) override;

private:
    QDataflowCanvas *canvas_;
    QList<QDataflowInlet*> inlets_;
    QList<QDataflowOutlet*> outlets_;
    QGraphicsRectItem *inputHeader_;
    QGraphicsRectItem *objectBox_;
    QGraphicsRectItem *outputHeader_;
    QDataflowNodeTextLabel *textItem_;
    bool valid_;
    QString oldText_;
    QDataflowMetaObject *metaObject_;

    friend class QDataflowCanvas;
};

class QDataflowIOlet : public QGraphicsItem
{
protected:
    QDataflowIOlet(QDataflowNode *node, int index);

public:
    virtual int type() const override = 0;

    QDataflowNode * node() const {return node_;}
    int index() const {return index_;}

    void addConnection(QDataflowConnection *connection);
    void removeConnection(QDataflowConnection *connection);
    QList<QDataflowConnection*> connections() const;
    void adjustConnections() const;

    QDataflowCanvas * canvas() const {return canvas_;}

protected:
    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

private:
    QDataflowCanvas *canvas_;
    QList<QDataflowConnection*> connections_;
    QDataflowNode *node_;
    int index_;

    friend class QDataflowCanvas;
    friend class QDataflowNode;
};

class QDataflowInlet : public QDataflowIOlet
{
protected:
    QDataflowInlet(QDataflowNode *node, int index);

public:
    int type() const override {return QDataflowItemTypeInlet;}

    void onDataRecevied(void *data);

    friend class QDataflowCanvas;
    friend class QDataflowNode;
};

class QDataflowOutlet : public QDataflowIOlet
{
protected:
    QDataflowOutlet(QDataflowNode *node, int index);

public:
    int type() const override {return QDataflowItemTypeOutlet;}

    void sendData(void *data);

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;

private:
    QGraphicsLineItem *tmpConn_;

    friend class QDataflowCanvas;
    friend class QDataflowNode;
};

class QDataflowConnection : public QGraphicsItem
{
protected:
    QDataflowConnection(QDataflowOutlet *source, QDataflowInlet *dest);

public:
    QDataflowOutlet * source() const {return source_;}
    QDataflowInlet * dest() const {return dest_;}

    void adjust();

    QDataflowCanvas * canvas() const {return canvas_;}

    int type() const override {return QDataflowItemTypeConnection;}

protected:
    QRectF boundingRect() const override;
    QPainterPath shape() const override;

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

    void keyPressEvent(QKeyEvent *event) override;

private:
    QDataflowCanvas *canvas_;
    QDataflowOutlet *source_;
    QDataflowInlet *dest_;
    QPointF sourcePoint_;
    QPointF destPoint_;

    friend class QDataflowCanvas;
    friend class QDataflowInlet;
    friend class QDataflowOutlet;
};

class QDataflowNodeTextLabel : public QGraphicsTextItem
{
protected:
    QDataflowNodeTextLabel(QDataflowNode *node, QGraphicsItem *parent);

public:
    bool sceneEvent(QEvent *event);
    void setCompletion(QStringList list);
    void clearCompletion();
    void acceptCompletion();
    void cycleCompletion(int d);
    void updateCompletion();
    void complete();

private:
    QDataflowNode *node_;
    QList<QGraphicsSimpleTextItem*> completionItems_;
    QList<QGraphicsRectItem*> completionRectItems_;
    int completionIndex_;
    bool completionActive_;

    friend class QDataflowNode;
};

class QDataflowTextCompletion
{
public:
    virtual void complete(QString nodeText, QStringList &completionList);
};

class QDataflowMetaObject
{
public:
    virtual ~QDataflowMetaObject() {}

    virtual bool init(QStringList args);
    QDataflowNode * node() {return node_;}
    QDataflowInlet * inlet(int index) {return node_->inlet(index);}
    QDataflowOutlet * outlet(int index) {return node_->outlet(index);}
    int inletCount() {return node_->inletCount();}
    void setInletCount(int c) {node_->setInletCount(c);}
    int outletCount() {return node_->outletCount();}
    void setOutletCount(int c) {node_->setOutletCount(c);}
    virtual void onDataReceved(int inlet, void *data);

private:
    QDataflowNode *node_;

    friend class QDataflowNode;
};

template<typename T>
T * QDataflowCanvas::itemAtT(const QPointF &point)
{
    foreach(QGraphicsItem *item, scene()->items(point, Qt::IntersectsItemShape, Qt::DescendingOrder, transform()))
    {
         if(T *itemT = dynamic_cast<T*>(item))
             return itemT;
    }
    return 0;
}


#endif // QDATAFLOWCANVAS_H
