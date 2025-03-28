#include "hoverpoints.h"

#include <QApplication>
#include <QEvent>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QColorDialog>

HoverPoints::HoverPoints(QWidget *widget, PointShape shape) : QObject(widget)
{
	m_widget = widget;
	widget->installEventFilter(this);

	m_connectionType = CurveConnection;
	m_sortType       = NoSort;
	m_shape          = shape;
	m_pointPen       = QPen(QColor(255, 255, 255, 191), 1);
	m_connectionPen  = QPen(QColor(255, 255, 255, 127), 2);
	m_pointBrush     = QBrush(QColor(191, 191, 191, 127));
	m_pointSize      = QSize(11, 11);
	m_currentIndex   = -1;
	m_editable       = true;
	m_enabled        = true;

	connect(this, SIGNAL(pointsChanged(const QPolygonF &)), m_widget, SLOT(update()));

	m_width = m_height = -1;

	m_colorBar = false;

	m_graPoints.clear();

	m_fixH = m_widget->height() / 2;
}

void HoverPoints::setEnabled(bool enabled)
{
	if (m_enabled != enabled)
	{
		m_enabled = enabled;
		m_widget->update();
	}
}

bool HoverPoints::eventFilter(QObject *object, QEvent *event)
{
	if (object == m_widget && m_enabled)
	{
		switch (event->type())
		{
		case QEvent::MouseButtonPress:
		{
			QMouseEvent *me = (QMouseEvent *)event;

			QPointF clickPos = me->pos();
			int index = -1;
			for (int i = 0; i < m_points.size(); ++i)
			{
				QPainterPath path;
				if (m_shape == CircleShape)
				{
					path.addEllipse(pointBoundingRect(i));
				}
				else
				{
					if (m_shape == RectangleShape)
					{
						path.addRect(pointBoundingRect(i));
					}
				}

				if (path.contains(clickPos))
				{
					index = i;
					break;
				}
			}

			if (me->button() == Qt::LeftButton)
			{
				if (m_colorBar)
					break;
				if (index == -1)
				{
					if (!m_editable)
					{
						return false;
					}
					int pos = 0;
					// Insert sort for x or y
					if (m_sortType == XSort)
					{
						for (int i = 0; i < m_points.size(); ++i)
						{
							if (m_points.at(i).x() > clickPos.x())
							{
								pos = i;
								break;
							}
						}
					}
					else if (m_sortType == YSort)
					{
						for (int i = 0; i < m_points.size(); ++i)
						{
							if (m_points.at(i).y() > clickPos.y())
							{
								pos = i;
								break;
							}
						}
					}

					m_points.insert(pos, clickPos);
					m_locks.insert(pos, 0);
					m_currentIndex = pos;
					firePointChange();
				}
				else
				{
					m_currentIndex = index;
				}
				return true;

			}
			else if (me->button() == Qt::RightButton)
			{
				if (index >= 0 && m_editable)
				{
					if (m_locks[index] == 0)
					{
						m_locks.remove(index);
						m_points.remove(index);
						if (m_colorBar && m_colors.size() > 0)
						{
							m_colors.removeAt(index);
						}
					}
					firePointChange();
					return true;
				}
			}

		}
		break;
		case QEvent::MouseButtonDblClick:
		{
			QMouseEvent *me = (QMouseEvent *)event;
			if (m_colorBar && me->button() == Qt::LeftButton)
			{
				QColor color;
				bool ok;
				color = QColor::fromRgba(QColorDialog::getRgba(color.rgba(), &ok));
				if (ok)
				{
					int index = -1;
					QPointF clickPos = me->pos();
					for (int i = 0; i < m_points.size(); ++i)
					{
						QPainterPath path;
						if (m_shape == CircleShape)
						{
							path.addEllipse(pointBoundingRect(i));
						}
						else
						{
							if (m_shape == RectangleShape)
							{
								path.addRect(pointBoundingRect(i));
							}
						}

						if (path.contains(clickPos))
						{
							index = i;
							break;
						}
					}
					if (index == -1)
					{
						if (!m_editable)
						{
							return false;
						}
						int pos = 0;
						// Insert sort for x or y
						if (m_sortType == XSort)
						{
							for (int i = 0; i < m_points.size(); ++i)
							{
								if (m_points.at(i).x() > clickPos.x())
								{
									pos = i;
									break;
								}
							}
						}
						else if (m_sortType == YSort)
						{
							for (int i = 0; i < m_points.size(); ++i)
							{
								if (m_points.at(i).y() > clickPos.y())
								{
									pos = i;
									break;
								}
							}
						}
						clickPos.setY(m_height / 2);
						m_points.insert(pos, clickPos);
						m_colors.insert(pos, color);
						m_locks.insert(pos, 0);
						m_currentIndex = pos;
						firePointChange();
					}
					else
					{
						m_currentIndex = index;
						m_colors.replace(m_currentIndex, color);
					}
				}

				return true;
			}
		}

		case QEvent::MouseButtonRelease:
			m_currentIndex = -1;
			break;

		case QEvent::MouseMove:
			if (m_currentIndex >= 0)
			{
				if (m_colorBar)
					break;
				movePoint(m_currentIndex, ((QMouseEvent *)event)->pos());
			}
			break;

		case QEvent::Resize:
		{
			if (m_widget->isVisible())
			{
				QResizeEvent *e = (QResizeEvent *)event;
				double stretch_x = e->size().width() / double(e->oldSize().width());
				double stretch_y = e->size().height() / double(e->oldSize().height());
				for (int i = 0; i < m_points.size(); ++i)
				{
					QPointF p = m_points[i];
					movePoint(i, QPointF(p.x() * stretch_x, p.y() * stretch_y), false);
				}

				firePointChange();

				m_width = e->size().width();
				m_height = e->size().height();
				m_fixH = m_height / 2;
			}
			break;
		}

		case QEvent::Show:
		{
			if (m_width != m_widget->width() || m_height != m_widget->height())
			{
				if (m_width == -1 && m_height == -1)
				{
					m_width = m_widget->width();
					m_height = m_widget->height();
				}
				double stretch_x = m_widget->width() / double(m_width);
				double stretch_y = m_widget->height() / double(m_height);
				for (int i = 0; i < m_points.size(); ++i)
				{
					QPointF p = m_points[i];
					movePoint(i, QPointF(p.x() * stretch_x, p.y() * stretch_y), false);
				}

				firePointChange();

				m_width = m_widget->width();
				m_height = m_widget->height();
				m_fixH = m_height / 2;
			}
			break;
		}

		case QEvent::Paint:
		{
			QWidget *that_widget = m_widget;
			m_widget = 0;
			QApplication::sendEvent(object, event);
			m_widget = that_widget;
			paintPoints();
			return true;
		}
		default:
			break;
		}
	}

	return false;
}

void HoverPoints::paintPoints()
{
	QPainter painter(m_widget), grdP(m_widget);

	painter.setRenderHint(QPainter::Antialiasing);
	grdP.setRenderHint(QPainter::Antialiasing);

	if (m_connectionPen.style() != Qt::NoPen && m_connectionType != NoConnection)
	{
		painter.setPen(m_connectionPen);
		grdP.setPen(m_connectionPen);

		if (m_connectionType == CurveConnection)
		{
			if (!m_colorBar)
			{
				QPainterPath path;
				path.moveTo(m_points.at(0));
				for (int i = 1; i < m_points.size(); ++i)
				{
					QPointF p1 = m_points.at(i - 1);
					QPointF p2 = m_points.at(i);
					double distance = p2.x() - p1.x();
					path.cubicTo(p1.x() + distance / 2, p1.y(), p1.x() + distance / 2, p2.y(), p2.x(), p2.y());
				}
				painter.drawPath(path);
			}

		}
		else
		{
			if (!m_colorBar)
			{
				painter.drawPolyline(m_points);
				m_graPoints.clear();
				for (int i = 0; i < m_points.size(); ++i)
				{
					QPointF p2 = m_points.at(i);
					m_graPoints.append(QPointF(p2.x(), p2.y() + 12));
				}
				grdP.drawPolyline(m_graPoints);
			}
		}
	}

	painter.setPen(m_pointPen);
	painter.setBrush(m_pointBrush);

	for (int i = 0; i < m_points.size(); ++i)
	{
		QRectF bounds = pointBoundingRect(i);
		if (m_shape == CircleShape)
		{
			painter.drawEllipse(bounds);
		}
		else
		{
			if (m_shape == RectangleShape)
			{
				if (m_colorBar)
				{
					painter.fillRect(bounds, QBrush(m_colors.at(i)));
				}
				else
				{
					painter.drawRect(bounds);
				}
			}
		}
	}
}

static QPointF bound_point(const QPointF &point, const QRectF &bounds, int lock)
{
	QPointF p = point;
	double left = bounds.left();
	double right = bounds.right();
	double top = bounds.top();
	double bottom = bounds.bottom();

	if (p.x() < left || (lock & HoverPoints::LockToLeft))
	{
		p.setX(left);
	}
	else if (p.x() > right || (lock & HoverPoints::LockToRight))
	{
		p.setX(right);
	}

	if (p.y() < top || (lock & HoverPoints::LockToTop))
	{
		p.setY(top);
	}
	else if (p.y() > bottom || (lock & HoverPoints::LockToBottom))
	{
		p.setY(bottom);
	}

	return p;
}

void HoverPoints::setPoints(const QPolygonF &points)
{
	m_points.clear();
	m_graPoints.clear();
	for (int i = 0; i < points.size(); ++i)
	{
		QPointF point(points.at(i));
		m_points << bound_point(point, boundingRect(), 0);
	}


	m_locks.clear();
	if (m_points.size() > 0)
	{
		m_locks.resize(m_points.size());
		m_locks.fill(0);
	}
}

void HoverPoints::movePoint(int index, const QPointF &point, bool emitUpdate)
{
	m_points[index] = bound_point(point, boundingRect(), m_locks.at(index));
	if (emitUpdate)
	{
		firePointChange();
	}
}

inline static bool x_less_than(const QPointF &p1, const QPointF &p2)
{
	return p1.x() < p2.x();
}

inline static bool y_less_than(const QPointF &p1, const QPointF &p2)
{
	return p1.y() < p2.y();
}

void HoverPoints::firePointChange()
{
	if (m_sortType != NoSort)
	{
		QPointF oldCurrent;
		if (m_currentIndex != -1)
		{
			oldCurrent = m_points[m_currentIndex];
		}

		if (m_sortType == XSort)
		{
			qSort(m_points.begin(), m_points.end(), x_less_than);
		}
		else if (m_sortType == YSort)
		{
			qSort(m_points.begin(), m_points.end(), y_less_than);
		}

		if (m_currentIndex != -1)
		{
			for (int i = 0; i < m_points.size(); ++i)
			{
				if (m_points[i] == oldCurrent)
				{
					m_currentIndex = i;
					break;
				}
			}
		}

	}

	emit pointsChanged(m_points);
}
