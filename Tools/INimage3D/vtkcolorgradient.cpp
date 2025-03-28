#include "vtkcolorgradient.h"
#include "ui_vtkcolorgradient.h"

#include"qfourpaneviewer.h"

#include<QEvent>
#include<QMouseEvent>
#include<QPainter>
#include<QPainterPath>
#include<QColorDialog>
#include<QMessageBox>

GradientShape::GradientShape(QWidget *widget, ShapeStyle style)
{
    if (widget)
    {
        m_parent = widget;
        m_parent->installEventFilter(this);
        widget->setStyleSheet("background-color:rgb(255,255,255)}");
        m_maxH = m_parent->height();
        m_maxW = m_parent->width();
    }
    m_shapeStyle = style;
    m_linePen = QPen(QColor(5, 5, 5, 255), 2 /*SolidLine*/);
    if (style == ShapeStyle::Default)
    {
        m_points.clear();
        int deltaX = 30, deltaY = 0;
        int delta = (m_maxH - 2 * deltaY) / 12;
        m_points << QPointF(35, m_maxH - 7) << QPointF(m_maxW - 35, m_maxH - deltaY - 10 * delta);
        //左上点 右下点
        m_xyRect = QRectF(QPointF(35, m_maxH - deltaY - 10 * delta), QPointF(m_maxW - 35, m_maxH));
    }
    else
    {
        m_points.clear();
        int deltaX = 35, deltaY = 0, deltaH = 20;
        int delta = (m_maxW - 2 * deltaX) / 10;
        int Y = (m_maxH - deltaY - deltaH) / 2;
        m_points << QPointF(deltaX, Y) << QPointF(deltaX + 10 * delta, Y);
        QColor color(255, 255, 255);
        m_colors.push_back(color);
        m_colors.push_back(color);
        m_xyRect = QRectF(QPointF(deltaX, Y), QPointF(deltaX + 10 * delta, Y));
    }
    m_slope = 80;
    m_synData = false;

}

inline QRectF GradientShape::PointInRectX(int i)const
{
    QPointF point = m_points.at(i);
    double w = 10;
    double x = point.x() - w / 2;
    return QRectF(x, 0, w, m_maxH);
}
inline QRectF GradientShape::getPointRect(int i)const
{
    QPointF point = m_points.at(i);
    double w = 10, h = 10;
    double x = point.x() - w / 2;
    double y = point.y() - h / 2;
    return QRectF(x, y, w, h);
}

void GradientShape::movePoints(int index, const QPointF &point, bool update)
{
    int len = m_points.size();
    if (index >= 0 && index < len)
    {
        QPointF pt    = point;
        double left   = m_xyRect.left();
        double right  = m_xyRect.right();
        double top    = m_xyRect.top();
        double bottom = m_xyRect.bottom();

        if (pt.x() < left)
        {
            pt.setX(left);
        }
        else if (pt.x() > right)
        {
            pt.setX(right);
        }
        if (index > 0 && index < len - 1)
        {
            if (pt.x() >= m_points[index + 1].x())
            {
                pt.setX(m_points[index + 1].x() - 5);
            }
            else if (pt.x() <= m_points[index - 1].x())
            {
                pt.setX(m_points[index -1].x() + 5);
            }
        }
        if (index == 0 || index == len - 1)
        {
            pt.setX(m_points[index].x());
        }
        if (pt.y() < top)
        {
            pt.setY(top);
        }
        else if (pt.y() > bottom)
        {
            pt.setY(bottom);
        }

        m_points[index] = pt;
        m_parent->update();
    }
    
}

void GradientShape::updateslope(int vl)
{
    if (m_slope != vl)
    {
        m_slope = vl;
        m_parent->update();
    }
}
void GradientShape::paintPointsLines()
{
    QPainter painter(m_parent);
    painter.setRenderHint(QPainter::Antialiasing);
    if (m_shapeStyle == ShapeStyle::Default)
    {
        // draw curve
        //QPainterPath path;
        //path.moveTo(m_points.at(0));
        //int size = m_points.size();
        //for (int i = 0; i < size; i++)
        //{
        //    QPointF pt1 = m_points.at(i - 1);
        //    QPointF pt2 = m_points.at(i);
        //
        //    double distance = pt2.x() - pt1.x();
        //    path.cubicTo(pt1.x() + distance / 2, pt1.y(), pt1.x() + distance / 2, pt2.y(), pt2.x(), pt2.y());
        //}
        //painter.drawPath(path);
        painter.drawPolyline(m_points);

        int len = m_points.size();
        if (m_slope <= 0)
        {
            m_slope = 0;
        }
        else if (m_slope > 100)
        {
            m_slope = 100;
        }

        m_gradientPoints.clear();
        for (int i = 0; i < len; i++)
        {
            QRectF bounds = getPointRect(i);
            painter.drawEllipse(bounds);

            QPointF ptH = m_points.at(i);
            int deltH = m_xyRect.bottomRight().y() - ptH.y();
            if (deltH >= 0)
            {
                ptH.setY(m_xyRect.bottomRight().y() - deltH*m_slope/100);
            }
            m_gradientPoints.append(ptH);
        }
        painter.drawPolyline(m_gradientPoints);
    }
    else
    {
        int len = m_points.size();
        int y = m_points.at(0).y();
        int colorH = 20;
        int colorW = 6;
        for (int i = 0; i < len; i++)
        {
            if (i < len - 1)
            {
                QLinearGradient gradient(m_points.at(i), m_points.at(i+1));
                gradient.setColorAt(0.0, m_colors.at(i));
                gradient.setColorAt(1.0, m_colors.at(i+1));
                QPoint top(m_points.at(i).x() + colorW, y- colorH);
                QPoint bottom(m_points.at(i+1).x() - colorW, y+ colorH);
                painter.fillRect(QRect(top, bottom), gradient);
            }
            QRectF bounds = getPointRect(i);
            painter.drawRect(bounds);
            painter.fillRect(bounds, QBrush(m_colors.at(i)));
        }
    }
    // test 和绘制XY坐标系的范围
    //QColor red(255, 0, 0);
    //QPen pen(red);
    //painter.setPen(red);
    //painter.drawRect(m_xyRect);

}

void GradientShape::paintRuler()
{
    QPainter painter(m_parent);
    painter.setRenderHint(QPainter::Antialiasing);

    //XY刻度
    //Y
    if (m_shapeStyle == ShapeStyle::Default)
    {
        //XY轴线
        QPolygonF points;
        int deltaX = 30, deltaY = 0;
        int delta = (m_maxH - 2 * deltaY) / 12;
        QPointF pointO(deltaX, m_maxH - deltaY), pointX(m_maxW - deltaX, m_maxH - deltaY), pointY(deltaX, deltaY + delta);
        points << pointY << pointO << pointX;
        painter.setPen(m_linePen);
        painter.drawPolyline(points);
        for (int i = 0; i < 11; i++)
        {
            painter.drawLine(QPointF(deltaX, m_maxH - deltaY - i * delta), QPointF(deltaX / 2, m_maxH - deltaY - i * delta));
            painter.drawText(QPointF(deltaX / 2 - 10, m_maxH - deltaY - i * delta - delta / 2 + 7), QString::number(i / 10.00, 'f', 2));
        }
    }
    else
    {
        //color bar
        QPolygonF points;
        int deltaX = 35, deltaY = 0, deltaH = 20;
        QPointF point0(deltaX, deltaY), point1(deltaX, m_maxH - deltaY - deltaH), point2(m_maxW - deltaX, m_maxH - deltaY - deltaH), point3(m_maxW - deltaX, deltaY);
        points << point0 << point1 << point2 << point3 << point0;
        painter.setPen(m_linePen);
        painter.drawPolyline(points);

        int delta = (m_maxW - 2 * deltaX) / 10;
        int j = m_grayMin, dis = (m_grayMax - m_grayMin) / 10;
        QLinearGradient gradient();
        for (int i = 0; i < 11; i++)
        {
            painter.drawLine(QPointF(deltaX + i * delta, m_maxH - deltaY - deltaH), QPointF(deltaX + i * delta, m_maxH - deltaY - deltaH + 7));
            painter.drawText(QPointF(deltaX + i * delta - 3, m_maxH - deltaY - deltaH + 18), QString::number(j));
            j += dis;
        }
    }

}

bool GradientShape::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == m_parent)
    {
        QMouseEvent *mouseEvent = (QMouseEvent *)event;
        int index = -1, removeindex = -1, len = m_points.size();
        QPointF clickPoint = mouseEvent->pos();
        switch (event->type())
        {
            case QEvent::MouseButtonDblClick:
            {
                //if (m_shapeStyle == ShapeStyle::ColorStyle){//}
                for (int i = 0; i < len; i++)
                {
                    QPainterPath path, pathPointX;
                    path.addRect(getPointRect(i));
                    pathPointX.addRect(PointInRectX(i));
                    if (path.contains(clickPoint))
                    {
                        index = i;
                        removeindex = i;
                    }
                    else if (pathPointX.contains(clickPoint))
                    {
                        index = i;
                    }
                    if (index > -1)
                    {                  
                        break;
                    }
                }
                if (mouseEvent->button() == Qt::LeftButton)
                {
                    if (index == -1)
                    {
                        int pos = 0;
                        for (int i = 0; i < len; i++)
                        {
                            if (m_points.at(i).x() > clickPoint.x())
                            {
                                pos = i;
                                break;
                            }
                        }
                        if (m_shapeStyle == ShapeStyle::ColorStyle)
                        {
                            bool bok;
                            QColor color = QColor::fromRgba(QColorDialog::getRgba(color.rgba(), &bok));
                            if (bok)
                            {
                                color.setAlpha(255);
                                m_colors.insert(pos, color);
                            }
                            else
                            {
                                return false;
                            }
                            m_points.insert(pos, QPointF(clickPoint.x(), m_points.at(0).y()));
                        }
                        else
                        {
                            if (clickPoint.rx() <= m_xyRect.topLeft().rx())
                            {
                                clickPoint.setX(m_xyRect.topLeft().rx() + 5);
                            }
                            else if (clickPoint.rx() >= m_xyRect.bottomRight().rx())
                            {
                                clickPoint.setX(m_xyRect.bottomRight().rx() - 5);
                            }
                            if (clickPoint.ry() <= m_xyRect.topLeft().ry())
                            {
                                clickPoint.setY(m_xyRect.topLeft().ry() + 5);
                            }
                            else if(clickPoint.ry() >= m_xyRect.bottomRight().ry())
                            {
                                clickPoint.setY(m_xyRect.bottomRight().ry() - 5);
                            }
                            m_points.insert(pos, clickPoint);
                        }
                        m_parent->update();
                        if (m_synData)
                        {
                            emit update3D();
                        }
                        m_currentIndex = pos;
                    }
                    else
                    {
                        if (m_shapeStyle == ShapeStyle::ColorStyle)
                        {
                            bool bok = false;
                            QColor color = QColor::fromRgba(QColorDialog::getRgba(color.rgba(), &bok));
                            if (bok)
                            {
                                color.setAlpha(255);
                                m_colors.replace(index, color);
                                m_parent->update();
                                if (m_synData)
                                {
                                    emit update3D();
                                }
                            }
                            else
                            {
                                return false;
                            }
                        }
                        m_currentIndex = index;
                    }
                }
                else if(mouseEvent->button() == Qt::RightButton)
                {
                    if (removeindex > 0 && removeindex < len - 1)
                    {
                        m_points.remove(removeindex);
                        m_colors.removeAt(removeindex);
                        m_parent->update();
                        if (m_synData)
                        {
                            emit update3D();
                        }
                    }
                }
                break;
            }
            case QEvent::MouseButtonPress:
            {
                if (mouseEvent->button() == Qt::LeftButton)
                {
                    for (int i = 0; i < len; i++)
                    {
                        QPainterPath path, pathPointX;
                        path.addRect(getPointRect(i));
                        pathPointX.addRect(PointInRectX(i));
                        if (path.contains(clickPoint))
                        {
                            index = i;
                            removeindex = i;
                        }
                        else if (pathPointX.contains(clickPoint))
                        {
                            index = i;
                        }
                        if (index > -1)
                        {                  
                            break;
                        }
                    }
                    m_currentIndex = index;
                }
                break;
            }
            case QEvent::MouseMove:
                if (m_currentIndex > -1)
                {
                    QMouseEvent *mouseEvent = (QMouseEvent *)event;
                    movePoints(m_currentIndex, clickPoint, true);
                    if (m_synData)
                    {
                        emit update3D();
                    }
                }
                break;
            case QEvent::MouseButtonRelease:
            {
                m_currentIndex = -1;
                break;
            }

            case QEvent::Resize:
            {
                m_maxH = m_parent->height();
                m_maxW = m_parent->width();
                break;
            }
            case QEvent::Show:
            {
                m_maxH = m_parent->height();
                m_maxW = m_parent->width();
                break;
            }
            case QEvent::Paint:
            {
                //QMouseEvent *mousePoint = (QMouseEvent *)event;
                //QPointF point = mousePoint->pos();
                paintRuler();
                paintPointsLines();
                return true;
            }
            default:
                break;
        }
    }
    return false;
}


//++++++++++++++++++++++++++++++++++++++++++++++++

void (QSpinBox:: *spinBoxSignal)(int) = &QSpinBox::valueChanged;
//fix : W:601 H:308
//    : W:601 H:55
VtkColorGradient::VtkColorGradient(QWidget *parent) : QWidget(parent), ui(new Ui::VtkColorGradient)
{
    m_parentViewer = nullptr;
    if (parent)
    {
        m_parentViewer = parent;
    }
    ui->setupUi(this);
    setWindowFlags(Qt::WindowStaysOnTopHint);
    setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint & ~Qt::WindowMaximizeButtonHint & ~Qt::WindowMinimizeButtonHint);//& ~Qt::WindowCloseButtonHint
    setStyleSheet("background-color:rgb(255,255,255)}");
    //ui->m_colorWidget->
    m_gradientShape = new GradientShape(ui->m_gwidget);
    m_colorBar      = new GradientShape(ui->m_colorWidget, ShapeStyle::ColorStyle);

    //初始化部分参数值
    m_grayMin = 0;
    m_grayMax = 8000;
    m_colorBar->setGrayMaxMin(m_grayMax, m_grayMin);

    setFixedSize(this->width(), this->height());
    ui->m_setslope->setValue(80);
    ui->m_sliderslope->setValue(80);
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    //滑动消息和选框消息变化事件通知
    //connect(ui->m_editorByValues, &QEditorByValues::signalsColorValue,this, &QFourpaneviewer::ResetColor3D);
    connect(ui->m_sliderslope, &QSlider::valueChanged, m_gradientShape, &GradientShape::updateslope);
    connect(ui->m_sliderslope, &QSlider::valueChanged, ui->m_setslope, &QSpinBox::setValue);
    connect(ui->m_setslope, spinBoxSignal, ui->m_sliderslope, &QSlider::setValue);
    connect(ui->m_sliderslope, &QSlider::valueChanged, [=](int vlue) {
                                                        if (ui->m_synUpdate3D->isChecked())
                                                        {
                                                            //m_vtkColorStyle.m_slope = ui->m_sliderslope->value();
                                                            update3D();
                                                        }  
                                                                      });
    connect(ui->m_setslope, spinBoxSignal, [=](int vlue)  {
                                                        if (ui->m_synUpdate3D->isChecked())
                                                        {
                                                            //m_vtkColorStyle.m_slope = ui->m_setslope->value();
                                                            update3D();
                                                        }
                                                          });
    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    //三维刷新事情通知checkbox ->syn --->updateData3D
    connect(ui->m_synUpdate3D, &QCheckBox::clicked, [=](bool check) { 
                                                    ui->m_pbUdate3D->setEnabled(!check); 
                                                    if (m_gradientShape)
                                                    {
                                                        m_gradientShape->m_synData = check;
                                                    }
                                                    if (m_colorBar)
                                                    {
                                                        m_colorBar->m_synData = check;
                                                    }
                                                                     });

    connect(ui->m_pbUdate3D, &QPushButton::pressed, this,&VtkColorGradient::update3D);

    connect(m_gradientShape, &GradientShape::update3D, this, &VtkColorGradient::update3D);
    connect(m_colorBar, &GradientShape::update3D, this, &VtkColorGradient::update3D);
    connect(ui->m_shadingGBox, &QGroupBox::clicked, [=](bool check) {
                                                    if (ui->m_synUpdate3D->isChecked())
                                                    {
                                                        update3D();
                                                    }
                                                                    });
    connect(ui->m_ambientSpinBox,       SIGNAL(valueChanged(double)), this, SLOT(update3D()));
    connect(ui->m_diffuseSpinBox,       SIGNAL(valueChanged(double)), this, SLOT(update3D()));
    connect(ui->m_specularSpinBox,      SIGNAL(valueChanged(double)), this, SLOT(update3D()));
    connect(ui->m_specularPowerSpinBox, SIGNAL(valueChanged(double)), this, SLOT(update3D()));
    m_vtkColorStyle.m_bpointValue = false;
    m_vtkColorStyle.clearAll();
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

}
//void VtkColorGradient::closeEvent(QCloseEvent *event)
//{
//    hide();
//    //event->accept();//不发送给组件的父组件
//    event->ignore();//事件继续发送
//}

void VtkColorGradient::updateDataVtkColorStyle()
{
    //m_vtkColorStyle.m_slope = ui->m_sliderslope->value();
    m_vtkColorStyle.clearAll();
    m_vtkColorStyle.m_colorOpacity = true;
    m_vtkColorStyle.m_colorAdd     = true;
    m_vtkColorStyle.m_slope        = ui->m_setslope->value();
    double graydelta               = m_grayMax - m_grayMin;

    QPolygonF points = m_gradientShape->getPoints();
    int sizeGra      = points.size();
    double x_delta   = points[sizeGra - 1].rx()- points[0].rx();
    double y_delta   = m_gradientShape->m_xyRect.height();
    double x_min     = m_gradientShape->m_xyRect.bottomLeft().rx();
    double y_max     = m_gradientShape->m_xyRect.bottomRight().ry();

    //透明度系数读取
    for (int i = 0; i < sizeGra; i++)
    {
        QPointF pt;
        pt.setX(m_grayMin + (points[i].rx() - x_min)*graydelta / x_delta);
        pt.setY((y_max - points[i].ry()) / y_delta);
        m_vtkColorStyle.m_gradientPoinst.append(pt);
    }

    
    QList<QColor> colors = m_colorBar->getColors();
    QPolygonF clpoints   = m_colorBar->getPoints();
    int sizeColor        = clpoints.size();
    double x_cldelta     = clpoints[sizeColor - 1].rx() - clpoints[0].rx();

    VtkColorPoint pt;
    pt.m_X     = m_grayMin;
    pt.m_Color = colors[0];
    m_vtkColorStyle.m_colorPoint.append(pt);
    //颜色系数读取
    for (int i = 1; i < sizeColor-1; i++)
    {
        pt.m_X     = m_grayMin + (clpoints[i].rx() - clpoints[0].rx())*graydelta / x_cldelta;
        pt.m_Color = colors[i];
    }
    pt.m_X     = m_grayMax;
    pt.m_Color = colors[sizeColor - 1];
    m_vtkColorStyle.m_colorPoint.append(pt);

    m_vtkColorStyle.m_lightShade = false;
    if (ui->m_shadingGBox->isChecked())
    {
        m_vtkColorStyle.m_lightShade     = true;
        m_vtkColorStyle.m_Ambient        = ui->m_ambientSpinBox->value();
        m_vtkColorStyle.m_Diffuse        = ui->m_diffuseSpinBox->value();
        m_vtkColorStyle.m_Specular       = ui->m_specularSpinBox->value();
        m_vtkColorStyle.m_SpecularPower  = ui->m_specularPowerSpinBox->value();
    }
}

void VtkColorGradient::update3D()
{
    if (m_parentViewer)
    {
        if (!m_vtkColorStyle.m_bpointValue)
        {
            updateDataVtkColorStyle();
            ((QFourpaneviewer*)m_parentViewer)->UpdateColorGradient3D(m_vtkColorStyle);
        }
    }
}

VtkColorGradient::~VtkColorGradient()
{
    delete ui;
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
/*
QSlider
{
    background-color: rgba(22, 22, 22, 0.7);
    padding-top: 15px;  //上面端点离顶部的距离
    padding - bottom: 15px;
    border - radius: 5px; //外边框矩形倒角
}

QSlider::add - page:vertical
{
    background - color: #FF7826;
    width:5px;
    border - radius: 2px;
}

QSlider::sub - page:vertical
{
    background - color: #7A7B79;
    width:5px;
    border - radius: 2px;
}

QSlider::groove:vertical
{
    background:transparent;
    width:6px;
}

QSlider::handle:vertical
{
    height: 14px;
    width: 14px;
    margin: 0px - 4px 0px - 4px;
    border - radius: 7px;
    background: white;
}
*/
