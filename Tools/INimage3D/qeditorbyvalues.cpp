
#include "qeditorbyvalues.h"
#include <cmath>
#include <QBoxLayout>
#include <QScrollArea>
#include <QStyle>

#include <QSettings>
QEditorByValues::QEditorByValues(QWidget *parent): QWidget(parent), m_minimum(-2000), m_maximum(2000)
{
    setupUi(this);

	this->setStyleSheet("background-color:rgb(240,240,240)}");
    //  We create a scroll area in case there are many intervals (it cannot be created from Qt Designer)

    QScrollArea *scrollArea = new QScrollArea(this);
    qobject_cast<QBoxLayout*>(this->layout())->insertWidget(1, scrollArea);

    m_intervalEditorsWidget = new QWidget(scrollArea);
    QBoxLayout *layout = new QVBoxLayout(m_intervalEditorsWidget);
    m_intervalEditorsLayout = new QVBoxLayout();
    layout->addLayout(m_intervalEditorsLayout);
    layout->addStretch();
    layout->setMargin(0);

    scrollArea->setWidget(m_intervalEditorsWidget);
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);


	QIntervalEditor *first = new QIntervalEditor(m_intervalEditorsWidget);
    first->setIsFirst(true);
    first->setIsLast(true);
    first->setObjectName("interval0");
    m_intervalEditorsLayout->addWidget(first);

    connect(first, SIGNAL(startChanged(int)), SLOT(markAsChanged()));
    connect(first, SIGNAL(endChanged(int)), SLOT(markAsChanged()));
    connect(first, SIGNAL(colorChanged(const QColor&)), SLOT(markAsChanged()));

    // Minimum Scroll Area Measurement
    QStyle *style = scrollArea->style();
    int scrollBarWidth = style->pixelMetric(QStyle::PM_ScrollBarExtent);
    scrollArea->setMinimumWidth(first->minimumSizeHint().width() + scrollBarWidth);

    m_numberOfIntervals = 1;

    m_changed = true;

    connect(m_addPushButton, SIGNAL(clicked()), SLOT(addInterval()));
    connect(m_removePushButton, SIGNAL(clicked()), SLOT(removeInterval()));
    connect(m_nameLineEdit, SIGNAL(textChanged(const QString&)), SLOT(markAsChanged()));

	connect(m_setValuesPB, SIGNAL(clicked()), SLOT(setValues3D()));
	connect(m_saveColorPB, SIGNAL(clicked()), SLOT(saveValues3D()));
	connect(m_loadFilesPB, SIGNAL(clicked()), SLOT(loadFileValues()));
	//

	//
	m_nameLabel->hide();
	m_nameLineEdit->hide();

	//--------------load config.ini-----------start--------------------------------------------------------------//
	QSettings settings("config.ini", QSettings::IniFormat);//settings.value("start/3DInterType", 0).toInt()
	QString str = settings.value("VtkColorStyle/points", "").toString();
	m_vtkColorStyle.clearAll();
	m_colorOpacityCK->setChecked(settings.value("VtkColorStyle/colorOpacity", false).toBool());
	m_colorAddCK->setChecked(settings.value("VtkColorStyle/colorAdd", false).toBool());

	QStringList colorstr = str.split(";");
	int count = colorstr.size();
	if (count > 1)
	{
		for (int i=0 ; i<count-1; i++)
		{
			QString var = colorstr[i];
			VtkColorPoint point;
			QStringList pointstr = var.split("|");
			point.m_X = pointstr[0].toDouble();
			point.m_Color = QColor(pointstr[1].toInt(), pointstr[2].toInt(), pointstr[3].toInt(), pointstr[4].toInt());
			m_vtkColorStyle.m_colorPoint.append(point);
		}
	}
	//---------------load config.ini-----------end------------------------------------------------------------------\\\


}

QEditorByValues::~QEditorByValues()
{
}

void QEditorByValues::setMinimum(int minimum)
{
    QList<QIntervalEditor*> intervalList = m_intervalEditorsWidget->findChildren<QIntervalEditor*>();
	QIntervalEditor *interval;

    foreach (interval, intervalList)
    {
        interval->setMinimum(m_minimum);
    }

    m_changed = true;
}

void QEditorByValues::setMaximum(int maximum)
{
    QList<QIntervalEditor*> intervalList =  m_intervalEditorsWidget->findChildren<QIntervalEditor*>();
	QIntervalEditor *interval;
    foreach (interval, intervalList)
    {
        interval->setMaximum(m_maximum);
    }

    m_changed = true;
}

const VtkColorStyle *QEditorByValues::getVtkColorStyle()
{
	return &m_vtkColorStyle;
}

void QEditorByValues::getValues() const
{
    if (m_changed)
    {
		m_vtkColorStyle.clearAll();
        QList<QIntervalEditor*> intervalList = m_intervalEditorsWidget->findChildren<QIntervalEditor*>();
		QIntervalEditor *interval;
        foreach (interval, intervalList)
        {
			VtkColorPoint point;
			point.m_Color = interval->color();
			point.m_Opacity = interval->color().alphaF();
			point.m_X = interval->start();
			m_vtkColorStyle.m_colorPoint.append(point);
            if (interval->isInterval())
            {
				VtkColorPoint point;
				point.m_Color = interval->color();
				point.m_Opacity = interval->color().alphaF();
				point.m_X = interval->end();
				m_vtkColorStyle.m_colorPoint.append(point);
            }
        }

        m_changed = false;
    }
}

void QEditorByValues::loadFileValues()
{
	//setValues(&m_vtkColorStyle);
	removeInterval();
	QIntervalEditor *current = m_intervalEditorsWidget->findChild<QIntervalEditor*>("interval0");

	QIntervalEditor *next = addIntervalAndReturnIt();

	QList<VtkColorPoint> points = m_vtkColorStyle.m_colorPoint;
	bool lastIsInterval = false;

	for (unsigned short i = 0; i < points.size(); i++)
	{
		double x = points.at(i).m_X;

		if (i == 0)
		{
			current->setIsInterval(false);
		}
		QColor color = points.at(i).m_Color;
		if (i == 0 || color != current->color())
		{
			if (i > 0)
			{
				current = next;
				if (i < points.size() - 1)
				{
					next = addIntervalAndReturnIt();
				}
			}

			current->setStart(static_cast<int>(qRound(x)));
			current->setColor(color);
			lastIsInterval = false;
		}
		else
		{
			current->setIsInterval(true);
			current->setEnd(static_cast<int>(qRound(x)));
			lastIsInterval = true;
		}
	}
}
void QEditorByValues::setValues(VtkColorStyle *vtkColors)
{
	if (!m_changed && &m_vtkColorStyle == vtkColors)
	{
		return;
	}
	
	//m_nameLineEdit->?(m_vtkColorStyle.m_styleName);

	while (m_numberOfIntervals > 1)
	{
		removeInterval();
	}

	QIntervalEditor *current = m_intervalEditorsWidget->findChild<QIntervalEditor*>("interval0");

	QIntervalEditor *next = addIntervalAndReturnIt();

	QList<VtkColorPoint> points = vtkColors->m_colorPoint;
	bool lastIsInterval = false;

	for (unsigned short i = 0; i < points.size(); i++)
	{
		double x = points.at(i).m_X;

		if (i == 0)
		{
			current->setIsInterval(false);
		}
		QColor color = points.at(i).m_Color;
		if (i == 0 || color != current->color())
		{
			if (i > 0)
			{
				current = next;
				if (i < points.size() - 1)
				{
					next = addIntervalAndReturnIt();
				}
			}

			current->setStart(static_cast<int>(qRound(x)));
			current->setColor(color);
			lastIsInterval = false;
		}
		else
		{
			current->setIsInterval(true);
			current->setEnd(static_cast<int>(qRound(x)));
			lastIsInterval = true;
		}
	}

	if (lastIsInterval)
	{
		removeInterval();
	}

	m_changed = true;
}

void QEditorByValues::saveValues3D()
{
	getValues();
	m_vtkColorStyle.m_colorOpacity = m_colorOpacityCK->isChecked();
	m_vtkColorStyle.m_colorAdd = m_colorAddCK->isChecked();
	QList<VtkColorPoint> points = m_vtkColorStyle.m_colorPoint;
	int count = points.size();
	if (count > 0)
	{
		QSettings settings("config.ini", QSettings::IniFormat);
		bool colorOpacity, colorAdd;
		colorOpacity = m_vtkColorStyle.m_colorOpacity;
		colorAdd = m_vtkColorStyle.m_colorAdd;
		int r, g, b;
		QString str;
		for (unsigned short i = 0; i < count; i++)
		{
			double x = points.at(i).m_X;
			QColor color = points.at(i).m_Color;
			int alpha = color.alpha();
			r = color.red();
			g = color.green();
			b = color.blue();
			str += QString("%1|%2|%3|%4|%5;").arg(x).arg(r).arg(g).arg(b).arg(alpha);
		}
		settings.setValue("VtkColorStyle/points", str);
		settings.setValue("VtkColorStyle/colorOpacity", colorOpacity);
		settings.setValue("VtkColorStyle/colorAdd", colorAdd);
	}
}
void QEditorByValues::setValues3D()
{
	if (m_colorOpacityCK->isChecked() || m_colorAddCK->isChecked())
	{
		getValues();
	}
	m_vtkColorStyle.m_colorOpacity = m_colorOpacityCK->isChecked();
	m_vtkColorStyle.m_colorAdd = m_colorAddCK->isChecked();
	emit signalsColorValue(m_vtkColorStyle);
}

void QEditorByValues::addInterval()
{
    addIntervalAndReturnIt();
}

void QEditorByValues::removeInterval()
{
    if (m_numberOfIntervals == 1)
    {
        return;
    }

    m_numberOfIntervals--;

	QIntervalEditor *last =   m_intervalEditorsWidget->findChild<QIntervalEditor*>( QString("interval%1").arg(m_numberOfIntervals));
    last->setParent(0);
    delete last;

	QIntervalEditor *beforeLast =  m_intervalEditorsWidget->findChild<QIntervalEditor*>( QString("interval%1").arg(m_numberOfIntervals - 1));
    beforeLast->setIsLast(true);

    markAsChanged();
}

QIntervalEditor *QEditorByValues::addIntervalAndReturnIt()
{
    if (m_numberOfIntervals == m_maximum - m_minimum + 1)
    {
        return 0;
    }

	QIntervalEditor *last        = m_intervalEditorsWidget->findChild<QIntervalEditor*>(  QString("interval%1").arg(m_numberOfIntervals - 1));
	QIntervalEditor *afterLast   = new QIntervalEditor(m_intervalEditorsWidget);
    afterLast->setMinimum(m_minimum);
    afterLast->setMaximum(m_maximum);

    connect(last, SIGNAL(endChanged(int)), afterLast, SLOT(setPreviousEnd(int)));
    connect(afterLast, SIGNAL(startChanged(int)), last, SLOT(setNextStart(int)));

    connect(afterLast, SIGNAL(startChanged(int)), SLOT(markAsChanged()));
    connect(afterLast, SIGNAL(endChanged(int)), SLOT(markAsChanged()));
    connect(afterLast, SIGNAL(colorChanged(const QColor&)), SLOT(markAsChanged()));

    last->setIsLast(false);

    afterLast->setIsLast(true);
    afterLast->setObjectName(QString("interval%1").arg(m_numberOfIntervals));

    m_intervalEditorsLayout->addWidget(afterLast);

    m_numberOfIntervals++;

    markAsChanged();

    return afterLast;
}

void QEditorByValues::markAsChanged()
{
    m_changed = true;
}
