/********************************************************************************
** Form generated from reading UI file 'qfourpaneviewer.ui'
**
** Created by: Qt User Interface Compiler version 5.12.12
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_QFOURPANEVIEWER_H
#define UI_QFOURPANEVIEWER_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QSplitter>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>
#include "QVTKRenderWidget.h"

QT_BEGIN_NAMESPACE

class Ui_QFourpaneviewer
{
public:
    QHBoxLayout *horizontalLayout;
    QSplitter *m_horizontalSplitter;
    QSplitter *m_verticalSplitter;
    QWidget *layout4;
    QVBoxLayout *vboxLayout;
    QHBoxLayout *_2;
    QVTKRenderWidget *m_axial2DView;
    QWidget *layout9;
    QVBoxLayout *_3;
    QVTKRenderWidget *m_sagital2DView;
    QSplitter *m_verticalSplitter1;
    QWidget *layout41;
    QVBoxLayout *_4;
    QHBoxLayout *_5;
    QVTKRenderWidget *m_coronal2DView;
    QWidget *layout91;
    QVBoxLayout *_6;
    QVTKRenderWidget *m_mpr2DView;

    void setupUi(QWidget *QFourpaneviewer)
    {
        if (QFourpaneviewer->objectName().isEmpty())
            QFourpaneviewer->setObjectName(QString::fromUtf8("QFourpaneviewer"));
        QFourpaneviewer->setWindowModality(Qt::NonModal);
        QFourpaneviewer->resize(663, 422);
        horizontalLayout = new QHBoxLayout(QFourpaneviewer);
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        m_horizontalSplitter = new QSplitter(QFourpaneviewer);
        m_horizontalSplitter->setObjectName(QString::fromUtf8("m_horizontalSplitter"));
        m_horizontalSplitter->setLayoutDirection(Qt::LeftToRight);
        m_horizontalSplitter->setOrientation(Qt::Horizontal);
        m_horizontalSplitter->setHandleWidth(1);
        m_verticalSplitter = new QSplitter(m_horizontalSplitter);
        m_verticalSplitter->setObjectName(QString::fromUtf8("m_verticalSplitter"));
        m_verticalSplitter->setOrientation(Qt::Vertical);
        layout4 = new QWidget(m_verticalSplitter);
        layout4->setObjectName(QString::fromUtf8("layout4"));
        vboxLayout = new QVBoxLayout(layout4);
        vboxLayout->setSpacing(0);
        vboxLayout->setObjectName(QString::fromUtf8("vboxLayout"));
        vboxLayout->setContentsMargins(0, 0, 0, 0);
        _2 = new QHBoxLayout();
        _2->setSpacing(0);
        _2->setObjectName(QString::fromUtf8("_2"));
        _2->setContentsMargins(0, 0, 0, 0);

        vboxLayout->addLayout(_2);

        m_axial2DView = new QVTKRenderWidget(layout4);
        m_axial2DView->setObjectName(QString::fromUtf8("m_axial2DView"));
        QSizePolicy sizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(m_axial2DView->sizePolicy().hasHeightForWidth());
        m_axial2DView->setSizePolicy(sizePolicy);

        vboxLayout->addWidget(m_axial2DView);

        m_verticalSplitter->addWidget(layout4);
        layout9 = new QWidget(m_verticalSplitter);
        layout9->setObjectName(QString::fromUtf8("layout9"));
        _3 = new QVBoxLayout(layout9);
        _3->setSpacing(0);
        _3->setObjectName(QString::fromUtf8("_3"));
        _3->setContentsMargins(0, 0, 0, 0);
        m_sagital2DView = new QVTKRenderWidget(layout9);
        m_sagital2DView->setObjectName(QString::fromUtf8("m_sagital2DView"));
        sizePolicy.setHeightForWidth(m_sagital2DView->sizePolicy().hasHeightForWidth());
        m_sagital2DView->setSizePolicy(sizePolicy);

        _3->addWidget(m_sagital2DView);

        m_verticalSplitter->addWidget(layout9);
        m_horizontalSplitter->addWidget(m_verticalSplitter);
        m_verticalSplitter1 = new QSplitter(m_horizontalSplitter);
        m_verticalSplitter1->setObjectName(QString::fromUtf8("m_verticalSplitter1"));
        m_verticalSplitter1->setOrientation(Qt::Vertical);
        m_verticalSplitter1->setChildrenCollapsible(false);
        layout41 = new QWidget(m_verticalSplitter1);
        layout41->setObjectName(QString::fromUtf8("layout41"));
        _4 = new QVBoxLayout(layout41);
        _4->setSpacing(0);
        _4->setObjectName(QString::fromUtf8("_4"));
        _4->setContentsMargins(0, 0, 0, 0);
        _5 = new QHBoxLayout();
        _5->setSpacing(6);
        _5->setObjectName(QString::fromUtf8("_5"));
        _5->setContentsMargins(0, 0, 0, 0);

        _4->addLayout(_5);

        m_coronal2DView = new QVTKRenderWidget(layout41);
        m_coronal2DView->setObjectName(QString::fromUtf8("m_coronal2DView"));
        QSizePolicy sizePolicy1(QSizePolicy::Expanding, QSizePolicy::Preferred);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(m_coronal2DView->sizePolicy().hasHeightForWidth());
        m_coronal2DView->setSizePolicy(sizePolicy1);

        _4->addWidget(m_coronal2DView);

        m_verticalSplitter1->addWidget(layout41);
        layout91 = new QWidget(m_verticalSplitter1);
        layout91->setObjectName(QString::fromUtf8("layout91"));
        _6 = new QVBoxLayout(layout91);
        _6->setSpacing(0);
        _6->setObjectName(QString::fromUtf8("_6"));
        _6->setContentsMargins(0, 0, 0, 0);
        m_mpr2DView = new QVTKRenderWidget(layout91);
        m_mpr2DView->setObjectName(QString::fromUtf8("m_mpr2DView"));
        sizePolicy.setHeightForWidth(m_mpr2DView->sizePolicy().hasHeightForWidth());
        m_mpr2DView->setSizePolicy(sizePolicy);
        m_mpr2DView->setMinimumSize(QSize(0, 0));

        _6->addWidget(m_mpr2DView);

        m_verticalSplitter1->addWidget(layout91);
        m_horizontalSplitter->addWidget(m_verticalSplitter1);

        horizontalLayout->addWidget(m_horizontalSplitter);


        retranslateUi(QFourpaneviewer);

        QMetaObject::connectSlotsByName(QFourpaneviewer);
    } // setupUi

    void retranslateUi(QWidget *QFourpaneviewer)
    {
        QFourpaneviewer->setWindowTitle(QApplication::translate("QFourpaneviewer", "Form", nullptr));
    } // retranslateUi

};

namespace Ui {
    class QFourpaneviewer: public Ui_QFourpaneviewer {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_QFOURPANEVIEWER_H
