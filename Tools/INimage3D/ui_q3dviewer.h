/********************************************************************************
** Form generated from reading UI file 'q3dviewer.ui'
**
** Created by: Qt User Interface Compiler version 5.12.12
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_Q3DVIEWER_H
#define UI_Q3DVIEWER_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QWidget>
#include "qvtkrenderwidget.h"

QT_BEGIN_NAMESPACE

class Ui_Q3dviewer
{
public:
    QHBoxLayout *horizontalLayout;
    QVTKRenderWidget *m_3DView;

    void setupUi(QWidget *Q3dviewer)
    {
        if (Q3dviewer->objectName().isEmpty())
            Q3dviewer->setObjectName(QString::fromUtf8("Q3dviewer"));
        Q3dviewer->resize(464, 356);
        horizontalLayout = new QHBoxLayout(Q3dviewer);
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        m_3DView = new QVTKRenderWidget(Q3dviewer);
        m_3DView->setObjectName(QString::fromUtf8("m_3DView"));

        horizontalLayout->addWidget(m_3DView);


        retranslateUi(Q3dviewer);

        QMetaObject::connectSlotsByName(Q3dviewer);
    } // setupUi

    void retranslateUi(QWidget *Q3dviewer)
    {
        Q3dviewer->setWindowTitle(QApplication::translate("Q3dviewer", "Form", nullptr));
    } // retranslateUi

};

namespace Ui {
    class Q3dviewer: public Ui_Q3dviewer {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_Q3DVIEWER_H
