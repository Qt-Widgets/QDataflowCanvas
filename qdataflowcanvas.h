#ifndef QDATAFLOWCANVAS_H
#define QDATAFLOWCANVAS_H

#include <QGraphicsScene>
#include <QGraphicsItem>
#include <QGraphicsView>
#include <QGraphicsSceneMouseEvent>
#include <QMouseEvent>
#include <QLineEdit>

enum QDataflowType {
    QDataflowTypeNode = QGraphicsItem::UserType + 1,
    QDataflowTypeConnection = QGraphicsItem::UserType + 2,
    QDataflowTypeInlet = QGraphicsItem::UserType + 3,
    QDataflowTypeOutlet = QGraphicsItem::UserType + 4
};

class QDataflowNode;
class QDataflowInlet;
class QDataflowOutlet;
class QDataflowConnection;

class QDataflowCanvas : public QGraphicsView
{
    Q_OBJECT
public:
    QDataflowCanvas(QWidget *parent);

    void addItem(QGraphicsItem *item);
    void removeItem(QGraphicsItem *item);
    void raiseItem(QGraphicsItem *item);

    template<typename T>
    T * itemAtT(const QPointF &point);

protected:
    void mouseDoubleClickEvent(QMouseEvent *event) override;

public slots:
    void itemTextChanged();
};

class QDataflowNode : public QGraphicsItem
{
public:
    QDataflowNode(QDataflowCanvas *canvas_, QString text = "", int numInlets = 0, int numOutlets = 0);

    void addConnection(QDataflowConnection *connection);
    void removeConnection(QDataflowConnection *connection);
    QList<QDataflowConnection*> connections() const;

    QDataflowInlet * inlet(int index) const {return inlets_.at(index);}
    int inletCount() const {return inlets_.size();}
    QDataflowOutlet * outlet(int index) const {return outlets_.at(index);}
    int outletCount() const {return outlets_.size();}

    int type() const override {return QDataflowTypeNode;}

    void setText(QString text);
    QString text() const;

    QRectF boundingRect() const override;
    //QPainterPath shape() const override;

    void adjust();

    QDataflowCanvas * canvas() {return canvas_;}

    qreal ioletWidth() const {return 10;}
    qreal ioletHeight() const {return 3;}
    qreal ioletSpacing() const {return 13;}
    qreal inletsWidth() const;
    qreal outletsWidth() const;

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

    void enterEditMode();
    void exitEditMode();
    bool isInEditMode() const;

protected:
    QVariant itemChange(GraphicsItemChange change, const QVariant &value) override;

    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event) override;

    void keyPressEvent(QKeyEvent *event) override;

private:
    QDataflowCanvas *canvas_;
    QList<QDataflowConnection*> connections_;
    QList<QDataflowInlet*> inlets_;
    QList<QDataflowOutlet*> outlets_;
    QGraphicsRectItem *inputHeader_;
    QGraphicsRectItem *objectBox_;
    QGraphicsRectItem *outputHeader_;
    QGraphicsTextItem *textItem_;
};

class QDataflowInlet : public QGraphicsItem
{
public:
    QDataflowInlet(QDataflowNode *node, int index);

    int type() const override {return QDataflowTypeInlet;}

    QDataflowNode * node() const {return node_;}
    int index() const {return index_;}

protected:
    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

private:
    QDataflowNode *node_;
    int index_;
};

class QDataflowOutlet : public QGraphicsItem
{
public:
    QDataflowOutlet(QDataflowNode *node, int index);

    int type() const override {return QDataflowTypeOutlet;}

    QDataflowNode * node() const {return node_;}
    int index() const {return index_;}

protected:
    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;

private:
    QDataflowNode *node_;
    int index_;
    QGraphicsLineItem *tmp_conn_;
};

class QDataflowConnection : public QGraphicsItem
{
public:
    QDataflowConnection(QDataflowOutlet *source, QDataflowInlet *dest);

    QDataflowOutlet * source() const {return source_;}
    QDataflowInlet * dest() const {return dest_;}

    void adjust();

    int type() const override {return QDataflowTypeConnection;}

protected:
    QRectF boundingRect() const override;
    QPainterPath shape() const override;

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

    void keyPressEvent(QKeyEvent *event) override;

private:
    QDataflowOutlet *source_;
    QDataflowInlet *dest_;

    QPointF sourcePoint_;
    QPointF destPoint_;
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
