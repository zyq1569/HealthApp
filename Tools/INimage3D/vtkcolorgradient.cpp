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
        //���ϵ� ���µ�
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

void GradientShape::setVtkColor(VtkColorGradient *vtkcolor)
{
    m_vtkcolor = vtkcolor;
}
inline QRectF GradientShape::PointInRectX(int i)const
{
    QPointF point = m_points.at(i);
    double w = 10;
    double x = point.x() - w / 2;
    return QRectF(x, 0, w, m_maxH);
}
inline QRectF GradientShape::getPointRect(int i, double w , double h)const
{
    QPointF point = m_points.at(i);
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
        int colorH = 18;
        int colorW = 0;
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
            QRectF bounds = getPointRect(i,15,15);
            painter.drawRect(bounds);
            QBrush brush(QColor(255, 255, 255));
            painter.setBrush(brush);
            painter.drawEllipse(bounds);

            bounds = getPointRect(i,8,8);
            painter.drawRect(bounds);
            painter.fillRect(bounds, QBrush(m_colors.at(i)));
        }
    }
    // test �ͻ���XY����ϵ�ķ�Χ
    //QColor red(255, 0, 0);
    //QPen pen(red);
    //painter.setPen(red);
    //painter.drawRect(m_xyRect);

}

void GradientShape::paintRuler()
{
    QPainter painter(m_parent);
    painter.setRenderHint(QPainter::Antialiasing);

    //XY�̶�
    //Y
    if (m_shapeStyle == ShapeStyle::Default)
    {
        //to do��Ŀǰ����λ����Ҫ�����Ż�Ϊ��Ե�Զ� (0,4) h:601 w:308
        //XY����
        QPolygonF points;
        int deltaX = 30, deltaY = 0;
        int delta = (m_maxH - 2 * deltaY) / 12;
        QPointF pointY(deltaX, deltaY + delta), pointO(deltaX, m_maxH - deltaY), pointX(m_maxW - deltaX, m_maxH - deltaY);
        points << pointY << pointO << pointX;
        painter.setPen(m_linePen);
        painter.drawPolyline(points);
        for (int i = 0; i < 11; i++)
        {
            painter.drawLine(QPointF(deltaX, m_maxH - deltaY - i * delta), QPointF(deltaX / 2, m_maxH - deltaY - i * delta));
            painter.drawText(QPointF(deltaX / 2 - 10, m_maxH - deltaY - i * delta - delta / 2 + 7), QString::number(i / 10.00, 'f', 2));
        }

        //++++++++++++++++++���ƻҶ�ͼ��ʼ------------������Ҫ��ϸ���������Ż�++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
        //int m_grayMax, m_grayMin;
        //int m_imageGrayHis[4096] = { 0 };
        //int m_lValue = -1, m_hValue = -1;
        //�������Ӧ�Ҷ�ֵXYֵ ������
        painter.setPen(QColor(0,0, 0, 200));
        int start = m_grayMin;
        int end   = m_grayMax;
        if (start < 0)
        {
            start = 0;
        }
        if (end > 4095)
        {
            end = 4095;
        }
        //to do:�Ҷ�ֵ �󲿷����ݼ�����ĳһ����(XY����ϵ�����޷��õ�Ч����ʾ, �ҶȵȾ����ʾ ��: �Ҷ�20-25 ����εĵ����ܺ� ռ�Ȼ���)
        if (m_vtkcolor)
        {
            int Ymin     = m_maxH - deltaY - 10 * delta-1;//Y�������ߵ�Yֵ
            int zoom     = 1;
            if (m_grayZoom > 1)
            {
                zoom = m_grayZoom;
            }
            int pixels   = m_numberPixels;
            double ratio = (double)m_vtkcolor->m_imageGrayHis[0] / m_numberPixels;
            double disX  = (end - start)/(double)(m_maxW-35 -35);
            int graydis  = disX;//���ֻҶȶ�. ��󲿷ֻҶȿ��Ժ���
            if (ratio > 0.3)
            {
                //--->�Ҷ�Ϊ0 ��ֵ����̫�󣬹�ֻ����0ֵ�ϵ���������,ͬʱ��pixels ������Z �ԷŴ�������صĻ�ֵ����
                start  = 1;
                pixels = (m_numberPixels - m_vtkcolor->m_imageGrayHis[0]) / zoom;
                
                QPointF pt1, pt2;
                pt1.setX(35);
                pt1.setY(m_maxH - 7);
                pt2.setX(35);              
                pt2.setY(m_maxH - deltaY - (10 * delta)* ratio*zoom);// Y:[0.0 - 1.0] ratioռ��
                if (pt2.y() < Ymin )
                {
                    pt2.setY(Ymin);
                }
                painter.drawLine(pt1, pt2);
            }
            else
            {
                pixels = m_numberPixels / zoom;
            }

            for (int i = start, x = 35 + start; i < end; i++, x++)
            {
                QPointF pt1, pt2;
                int gray = 0;
                for (int j = 0; j < graydis; j++)
                {
                    gray += m_vtkcolor->m_imageGrayHis[i + j];
                }
                i += graydis;

                if (gray > 1)
                {
                    ratio = (double)gray / pixels;
                    if (ratio > 0.0001)//ռ��̫С����,�Ѿ�zoom�Ŵ��� m_maxH - deltaY - 10 * delta
                    {
                        pt1.setX(x);
                        pt1.setY(m_maxH - 7 - ratio*delta);                      
                        if (pt1.y() < Ymin)// ����Ŵ��߶ȳ������߶�,�����߶�
                        {
                            pt1.setY(Ymin);
                        }
                        pt2.setX(x);
                        pt2.setY(m_maxH - 7);
                        painter.drawLine(pt1, pt2);
                    }
                }
            }
        }
        //++++++++++++++++++���ƻҶ�ͼ ����++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    }
    else
    {
        //to do��Ŀǰ����λ����Ҫ�����Ż�Ϊ��Ե�Զ� (0,312) h:601 w:55
        //color bar VTK9.4 C++ QT  ������ vtkImageData ��λ��ƻҶ�ֱ��ͼ
        QPolygonF points;
        int deltaX = 35, deltaY = -1, deltaH = 20;
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
            (9 == i) ? (j = m_grayMax) : (j += dis);
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
void (QDoubleSpinBox:: *spinDBoxSignal)(double) = &QDoubleSpinBox::valueChanged;

//fix : W:601 H:308
//    : W:601 H:55
VtkColorGradient::VtkColorGradient(QWidget *parent) : QWidget(parent), ui(new Ui::VtkColorGradient)
{
    m_parentViewer = nullptr;

    m_grayMin = 0;
    m_grayMax = 8000;

    if (parent)
    {
        m_parentViewer = parent;
        QFourpaneviewer* pane = (QFourpaneviewer*)parent;
        m_grayMax             = pane->m_maxGray;
        m_grayMin             = pane->m_minGray;
        m_lValue              = pane->m_lValue;
        m_hValue              = pane->m_hValue;
        m_numberPixels        = pane->m_grayMaxOfNumbers;
        //+++++++++++++[0 , 4096]//��Χֵ��ĻҶ�ֵ ���ڿ��Ǵ���
        if (m_lValue > 200) //<0
        {
            m_grayMin -= 100;
        }
        if (m_hValue > 200)//>4096
        {
            m_grayMax += 100;
        }
        //+++++++++++++
        int len = sizeof(m_imageGrayHis) / sizeof(m_imageGrayHis[0]);
        memcpy(m_imageGrayHis, pane->m_imageGrayHis, len*sizeof(int));

    }
    ui->setupUi(this);
    setWindowFlags(Qt::WindowStaysOnTopHint);
    setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint & ~Qt::WindowMaximizeButtonHint & ~Qt::WindowMinimizeButtonHint);//& ~Qt::WindowCloseButtonHint
    setStyleSheet("background-color:rgb(255,255,255)}");
    //ui->m_colorWidget->
    m_gradientShape = new GradientShape(ui->m_gwidget);
    m_colorBar      = new GradientShape(ui->m_colorWidget, ShapeStyle::ColorStyle);

    //��ʼ�����ֲ���ֵ
    m_colorBar->setGrayMaxMin(m_grayMax, m_grayMin);
    m_gradientShape->setGrayMaxMin(m_grayMax, m_grayMin, ((QFourpaneviewer*) parent)->m_grayMaxOfNumbers);
    m_gradientShape->setVtkColor(this);

    setFixedSize(this->width(), this->height());
    ui->m_setslope->setValue(80);
    ui->m_sliderslope->setValue(80);
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    //������Ϣ��ѡ����Ϣ�仯�¼�֪ͨ
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
    //��άˢ������֪ͨcheckbox ->syn --->updateData3D
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

    //��ֵͼ�Ŵ���
    connect(ui->m_grayZoom, spinDBoxSignal, [=](double vlue) {
                                                                m_gradientShape->setgrayZoom(vlue);
                                                                ui->m_gwidget->update();
                                                              });

    m_vtkColorStyle.m_bpointValue = false;
    m_vtkColorStyle.clearAll();

    //m_selectColor
    connect(ui->m_selectColor, &QPushButton::pressed, this, &VtkColorGradient::updateIsosurface);
    connect(ui->m_blendBox, SIGNAL(activated(int)), this, SLOT(updateBlend(int)));
    
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


}

void VtkColorGradient::updateDataVtkColorStyle()
{
    m_vtkColorStyle.clearAll();
    m_vtkColorStyle.m_colorOpacity = true;
    m_vtkColorStyle.m_colorAdd     = true;
    m_vtkColorStyle.m_slope        = ui->m_setslope->value();
    double graydelta               = m_grayMax - m_grayMin;

    m_vtkColorStyle.m_blendMode = ui->m_blendBox->currentIndex();

    QPolygonF points = m_gradientShape->getPoints();
    int sizeGra      = points.size();
    double x_delta   = points[sizeGra - 1].rx()- points[0].rx();
    double y_delta   = m_gradientShape->m_xyRect.height();
    double x_min     = m_gradientShape->m_xyRect.bottomLeft().rx();
    double y_max     = m_gradientShape->m_xyRect.bottomRight().ry();

    //͸����ϵ����ȡ
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
    //��ɫϵ����ȡ
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

void VtkColorGradient::updateBlend(int blend)
{
    //m_vtkColorStyle.m_blendMode = blend;
    update3D();
}

void VtkColorGradient::updateIsosurface()
{
   //bool bok = false;
   //QColor color = QColor::fromRgba(QColorDialog::getRgba(color.rgba(), &bok));
   //if (bok)
   //{
   //    int alpha = color.alpha();
   //    QString foreground = QString(";color:") + (color.value() < 128 ? "white" : "black");
   //    ui->m_isosurfaceSpinBox->setStyleSheet(QString("background-color:") + color.name() + foreground);
   //    ui->m_isosurfaceSpinBox->setValue(color.alpha());
   //
   //    m_vtkColorStyle.m_bisosurface = true;
   //    m_vtkColorStyle.m_isosurface  = color;
   //}
   //update3D();
}

VtkColorGradient::~VtkColorGradient()
{
    delete ui;
}
