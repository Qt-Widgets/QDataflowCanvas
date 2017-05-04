#include "qdataflowcanvas.h"

#include <math.h>

#include <QDebug>

#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <QStyleOption>
#include <QApplication>
#include <QTextDocument>

QDataflowCanvas::QDataflowCanvas(QWidget *parent)
    : QGraphicsView(parent)
{
    QGraphicsScene *scene = new QGraphicsScene(this);
    scene->setItemIndexMethod(QGraphicsScene::NoIndex);
    scene->setSceneRect(-200, -200, 400, 400);
    setScene(scene);
    setCacheMode(CacheBackground);
    setViewportUpdateMode(BoundingRectViewportUpdate);
    setRenderHint(QPainter::Antialiasing, false);
    setRenderHint(QPainter::TextAntialiasing, true);
    setTransformationAnchor(AnchorUnderMouse);
    scale(0.75, 0.75);
    setMinimumSize(400, 400);

    QRadialGradient gradient(0, 0, 800);
    gradient.setColorAt(0, QColor(240,240,240));
    gradient.setColorAt(1, QColor(160,160,160));
    setBackgroundBrush(gradient);
}

void QDataflowCanvas::addItem(QGraphicsItem *item)
{
    scene()->addItem(item);
}

void QDataflowCanvas::removeItem(QGraphicsItem *item)
{
    scene()->removeItem(item);
}

void QDataflowCanvas::raiseItem(QGraphicsItem *item)
{
    qreal maxZ = 0;
    foreach(QGraphicsItem *item1, item->collidingItems(Qt::IntersectsItemBoundingRect))
        maxZ = qMax(maxZ, item1->zValue());
    item->setZValue(maxZ + 1);

    if(QDataflowNode *node = dynamic_cast<QDataflowNode*>(item))
    {
        foreach(QDataflowConnection *conn, node->connections())
        {
            raiseItem(conn);
        }
    }
}

void QDataflowCanvas::mouseDoubleClickEvent(QMouseEvent *event)
{
    QGraphicsItem *item = itemAt(event->pos());
    if(!item)
    {
        QDataflowNode *node = new QDataflowNode(this, "", 2, 2);
        scene()->addItem(node);
        node->setPos(mapToScene(event->pos()));
        node->enterEditMode();
        event->accept();
        return;
    }
    event->ignore();
    QGraphicsView::mouseDoubleClickEvent(event);
}

void QDataflowCanvas::itemTextChanged()
{
    QObject *senderParent = sender()->parent();
    if(!senderParent) return;
    QObject *senderGrandParent = senderParent->parent();
    if(!senderGrandParent) return;
    QGraphicsTextItem *txtItem = qobject_cast<QGraphicsTextItem*>(senderGrandParent);
    if(!txtItem) return;
    QGraphicsItem *item = txtItem->topLevelItem();
    if(!item) return;
    QDataflowNode *node = dynamic_cast<QDataflowNode*>(item);
    if(!node) return;
    QString txt = txtItem->document()->toPlainText();
    if(txt.contains('\n'))
    {
        txt.replace("\n", "");
        txtItem->document()->setPlainText(txt);
        node->exitEditMode();
        return;
    }
    node->adjust();
}

QDataflowNode::QDataflowNode(QDataflowCanvas *canvas, QString text, int numInlets, int numOutlets)
    : canvas_(canvas)
{
    setFlag(ItemIsMovable);
    setFlag(ItemSendsGeometryChanges);
    setFlag(ItemIsSelectable);
    setFlag(ItemIsFocusable);

    setAcceptedMouseButtons(Qt::LeftButton);

    setCacheMode(DeviceCoordinateCache);

    inputHeader_ = new QGraphicsRectItem(this);
    inputHeader_->setBrush(QBrush(Qt::lightGray));
    inputHeader_->setPen(QPen(Qt::black));

    for(int i = 0; i < numInlets; i++)
    {
        QDataflowInlet * inlet = new QDataflowInlet(this, i);
        inlet->setParentItem(inputHeader_);
        inlet->setPos(ioletWidth() / 2 + i * (ioletWidth() + ioletSpacing()), ioletHeight() / 2);
        inlets_.push_back(inlet);
    }

    objectBox_ = new QGraphicsRectItem(this);
    objectBox_->setBrush(QBrush(Qt::white));
    objectBox_->setPen(QPen(Qt::black));
    objectBox_->setFlag(QGraphicsItem::ItemStacksBehindParent);

    textItem_ = new QGraphicsTextItem(objectBox_);
    textItem_->document()->setPlainText(text);
    textItem_->setTextInteractionFlags(Qt::TextEditable);
    //textItem_->setTextInteractionFlags(Qt::TextEditorInteraction);

    QObject::connect(textItem_->document(), &QTextDocument::contentsChanged, canvas, &QDataflowCanvas::itemTextChanged);

    outputHeader_ = new QGraphicsRectItem(this);
    outputHeader_->setBrush(QBrush(Qt::lightGray));
    outputHeader_->setPen(QPen(Qt::black));

    for(int i = 0; i < numOutlets; i++)
    {
        QDataflowOutlet * outlet = new QDataflowOutlet(this, i);
        outlet->setParentItem(outputHeader_);
        outlet->setPos(ioletWidth() / 2 + i * (ioletWidth() + ioletSpacing()), ioletHeight() / 2);
        outlets_.push_back(outlet);
    }

    adjust();
}

void QDataflowNode::addConnection(QDataflowConnection *connection)
{
    connections_ << connection;
    connection->adjust();
}

void QDataflowNode::removeConnection(QDataflowConnection *connection)
{
    connections_.removeAll(connection);
}

QList<QDataflowConnection*> QDataflowNode::connections() const
{
    return connections_;
}

void QDataflowNode::setText(QString text)
{
    textItem_->setPlainText(text);
}

QString QDataflowNode::text() const
{
    return textItem_->document()->toPlainText();
}

QRectF QDataflowNode::boundingRect() const
{
    return objectBox_->boundingRect()
            .united(inputHeader_->boundingRect())
            .united(outputHeader_->boundingRect());
}

/*
QPainterPath QDataflowNode::shape() const
{
    QPainterPath path;
    path.addRect(boundingRect());
    return path;
}
*/

void QDataflowNode::adjust()
{
    QRectF r = textItem_->boundingRect();
    qreal w = std::max(r.width(), std::max(inletsWidth(), outletsWidth()));

    prepareGeometryChange();

    inputHeader_->setPos(0, 0);
    objectBox_->setPos(0, ioletHeight());
    outputHeader_->setPos(0, ioletHeight() + r.height());

    inputHeader_->setRect(0, 0, w, ioletHeight());
    objectBox_->setRect(0, 0, w, r.height());
    outputHeader_->setRect(0, 0, w, ioletHeight());

    foreach(QDataflowConnection *connection, connections_)
        connection->adjust();
}

qreal QDataflowNode::inletsWidth() const
{
    return inletCount() * (ioletWidth() + ioletSpacing()) - ioletSpacing();
}

qreal QDataflowNode::outletsWidth() const
{
    return outletCount() * (ioletWidth() + ioletSpacing()) - ioletSpacing();
}

void QDataflowNode::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *)
{
    Q_UNUSED(painter);
    Q_UNUSED(option);
}

void QDataflowNode::enterEditMode()
{
    setSelected(true);
    textItem_->setFocus();
}

void QDataflowNode::exitEditMode()
{
    setFocus();
}

bool QDataflowNode::isInEditMode() const
{
    return textItem_->hasFocus();
}

QVariant QDataflowNode::itemChange(GraphicsItemChange change, const QVariant &value)
{
    switch (change) {
    case ItemPositionHasChanged:
        foreach(QDataflowConnection *connection, connections_)
            connection->adjust();
        break;
    case ItemSelectedHasChanged:
        {
            bool selected = value.toBool();
            QPen pen(selected ? Qt::blue : Qt::black);
            inputHeader_->setPen(pen);
            objectBox_->setPen(pen);
            outputHeader_->setPen(pen);

            if(!selected && isInEditMode())
                exitEditMode();

            if(selected)
                canvas()->raiseItem(this);
        }
        break;
    default:
        break;
    };

    return QGraphicsItem::itemChange(change, value);
}

void QDataflowNode::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{
    if(!isInEditMode())
    {
        enterEditMode();
        return;
    }
    event->ignore();
    QGraphicsItem::mouseDoubleClickEvent(event);
}

void QDataflowNode::keyPressEvent(QKeyEvent *event)
{
    if(!isInEditMode())
    {
        if(event->key() == Qt::Key_Backspace)
        {
            foreach(QDataflowConnection *connection, connections_)
                scene()->removeItem(connection);
            scene()->removeItem(this);
            event->accept();
            return;
        }
        else
        {
            event->ignore();
            return;
        }
    }
}

QDataflowInlet::QDataflowInlet(QDataflowNode *node, int index)
    : node_(node), index_(index)
{
    setAcceptDrops(true);
}

QRectF QDataflowInlet::boundingRect() const
{
    QDataflowNode *n = node();
    return QRectF(-n->ioletWidth() / 2, -n->ioletHeight() / 2, n->ioletWidth(), n->ioletHeight());
}

void QDataflowInlet::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);
    QDataflowNode *n = node();
    painter->fillRect(QRect(-n->ioletWidth() / 2, -n->ioletHeight() / 2, n->ioletWidth(), n->ioletHeight()), Qt::black);
}

QDataflowOutlet::QDataflowOutlet(QDataflowNode *node, int index)
    : node_(node), index_(index), tmp_conn_(0)

{
    setCursor(Qt::CrossCursor);
    setAcceptedMouseButtons(Qt::LeftButton);
}

QRectF QDataflowOutlet::boundingRect() const
{
    QDataflowNode *n = node();
    return QRectF(-n->ioletWidth() / 2, -n->ioletHeight() / 2, n->ioletWidth(), n->ioletHeight());
}

void QDataflowOutlet::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    QDataflowNode *n = node();
    painter->fillRect(QRect(-n->ioletWidth() / 2, -n->ioletHeight() / 2, n->ioletWidth(), n->ioletHeight()), Qt::black);
}

void QDataflowOutlet::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    Q_UNUSED(event);

    tmp_conn_ = new QGraphicsLineItem(this);
    tmp_conn_->setPos(0, node()->ioletHeight() / 2);
    tmp_conn_->setZValue(10000);
    tmp_conn_->setPen(QPen(Qt::red, 1, Qt::DotLine, Qt::RoundCap, Qt::RoundJoin));
    tmp_conn_->setFlag(ItemStacksBehindParent);
    node()->canvas()->raiseItem(tmp_conn_);
    node()->canvas()->raiseItem(node());
}

void QDataflowOutlet::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    Q_UNUSED(event);

    setCursor(Qt::CrossCursor);

    node()->scene()->removeItem(tmp_conn_);
    delete tmp_conn_;
    tmp_conn_ = 0;

    if(QDataflowInlet *inlet = node()->canvas()->itemAtT<QDataflowInlet>(event->scenePos()))
    {
        QDataflowConnection *conn = new QDataflowConnection(this, inlet);
        node()->canvas()->addItem(conn);
        node()->canvas()->raiseItem(conn);
    }
}

void QDataflowOutlet::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if(tmp_conn_)
    {
        tmp_conn_->setLine(QLineF(QPointF(), tmp_conn_->mapFromScene(event->scenePos())));
        QDataflowInlet *inlet = node()->canvas()->itemAtT<QDataflowInlet>(event->scenePos());
        tmp_conn_->setPen(QPen(inlet ? Qt::black : Qt::red, 1, inlet ? Qt::SolidLine : Qt::DotLine, Qt::RoundCap, Qt::RoundJoin));
    }
}

QDataflowConnection::QDataflowConnection(QDataflowOutlet *source, QDataflowInlet *dest)
    : source_(source), dest_(dest)
{
    setFlag(ItemIsSelectable);
    setFlag(ItemIsFocusable);

    setAcceptedMouseButtons(Qt::LeftButton);

    source_->node()->addConnection(this);
    dest_->node()->addConnection(this);
    adjust();
}

void QDataflowConnection::adjust()
{
    if(!source_ || !dest_)
        return;

    prepareGeometryChange();

    sourcePoint_ = mapFromItem(source_, 0, source_->node()->ioletHeight() / 2);
    destPoint_ = mapFromItem(dest_, 0, -dest_->node()->ioletHeight() / 2);
}

QRectF QDataflowConnection::boundingRect() const
{
    if(!source_ || !dest_)
        return QRectF();

    qreal penWidth = 1;
    qreal extra = (penWidth) / 2.0;

    return QRectF(sourcePoint_, destPoint_).normalized()
        .adjusted(-extra, -extra, extra, extra);
}

QPainterPath QDataflowConnection::shape() const
{
    int k = 1;
    QPoint d[] = {QPoint(k, 0), QPoint(0, k), QPoint(k, k), QPoint(k, -k)};
    QPolygonF p;
    for(int i = 0; i < 4; i++)
        p = p.united(QPolygonF() << (sourcePoint_ - d[i]) << (sourcePoint_ + d[i]) <<
              (destPoint_ + d[i]) << (destPoint_ - d[i]));
    QPainterPath path;
    path.addPolygon(p);
    return path;
}

void QDataflowConnection::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *)
{
    if(!source_ || !dest_)
        return;

    QLineF line(sourcePoint_, destPoint_);
    if(qFuzzyCompare(line.length(), qreal(0.)))
        return;

    painter->setPen(QPen(option->state & QStyle::State_Selected ? Qt::blue : Qt::black, 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    painter->drawLine(line);
}

void QDataflowConnection::keyPressEvent(QKeyEvent *event)
{
    if(event->key() == Qt::Key_Backspace)
    {
        source()->node()->removeConnection(this);
        dest()->node()->removeConnection(this);
        source()->node()->scene()->removeItem(this);
        event->accept();
    }
    else event->ignore();
}
