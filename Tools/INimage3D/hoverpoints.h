#ifndef UDGHOVERPOINTS_H
#define UDGHOVERPOINTS_H

#include <QObject>

#include <QPen>
#include <QPolygonF>
#include <QWidget>


class HoverPoints : public QObject 
{
Q_OBJECT
public:
    enum PointShape 
	{
        CircleShape,
        RectangleShape,
        NoShape
    };

    enum LockType 
	{
        LockToLeft   = 0x01,
        LockToRight  = 0x02,
        LockToTop    = 0x04,
        LockToBottom = 0x08
    };

    enum SortType 
	{
        NoSort,
        XSort,
        YSort
    };

    enum ConnectionType 
	{
        NoConnection,
        LineConnection,
        CurveConnection
    };

    HoverPoints(QWidget *widget, PointShape shape);

    bool eventFilter(QObject *object, QEvent *event);

    void paintPoints();

    inline QRectF boundingRect() const;
    void setBoundingRect(const QRectF &boundingRect)
    {
        m_bounds = boundingRect;
    }

    QPolygonF points() const
    {
        return m_points;
    }
    void setPoints(const QPolygonF &points);

    QSizeF pointSize() const
    {
        return m_pointSize;
    }
    void setPointSize(const QSizeF &size)
    {
        m_pointSize = size;
    }

    SortType sortType() const
    {
        return m_sortType;
    }
    void setSortType(SortType sortType)
    {
        m_sortType = sortType;
    }

    ConnectionType connectionType() const
    {
        return m_connectionType;
    }
    void setConnectionType(ConnectionType connectionType)
    {
        m_connectionType = connectionType;
    }

    void setConnectionPen(const QPen &pen)
    {
        m_connectionPen = pen;
    }
    void setShapePen(const QPen &pen)
    {
        m_pointPen = pen;
    }
    void setShapeBrush(const QBrush &brush)
    {
        m_pointBrush = brush;
    }

    void setPointLock(int pos, LockType lock)
    {
        m_locks[pos] = lock;
    }

    void setEditable(bool editable)
    {
        m_editable = editable;
    }
    bool editable() const
    {
        return m_editable;
    }
	
	void setPointShape(PointShape shape)
	{
		m_shape = shape;
	}

public slots:
    void setEnabled(bool enabled);
    void setDisabled(bool disabled)
    {
        setEnabled(!disabled);
    }

signals:
    void pointsChanged(const QPolygonF &points);

public:
    void firePointChange();
	void setColorBar(bool flag)
	{
		m_colorBar = flag;
	}

public:
		QList<QColor> m_colors;
private:
    inline QRectF pointBoundingRect(int i) const;
    inline void movePoint(int i, const QPointF &newPos, bool emitChange = true);

    QWidget *m_widget;

    QPolygonF m_points,m_graPoints;
    QRectF m_bounds;
    PointShape m_shape;
    SortType m_sortType;
    ConnectionType m_connectionType;

    QVector<uint> m_locks;

    QSizeF m_pointSize;
    int m_currentIndex;
    bool m_editable;
    bool m_enabled;

    QPen m_pointPen;
    QBrush m_pointBrush;
    QPen m_connectionPen;
	bool m_colorBar;

    int m_width, m_height,m_fixH;
};

inline QRectF HoverPoints::pointBoundingRect(int i) const
{
    QPointF p = m_points.at(i);
    double w  = m_pointSize.width();
    double h  = m_pointSize.height();
    double x  = p.x() - w / 2;
    double y  = p.y() - h / 2;
    return QRectF(x, y, w, h);
}

inline QRectF HoverPoints::boundingRect() const
{
    if (m_bounds.isEmpty())
    {
        return m_widget->rect();
    }
    else
    {
        return m_bounds;
    }
}

#endif
