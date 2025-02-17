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
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_QFourpaneviewer
{
public:

    void setupUi(QWidget *qfourpaneviewer)
    {
        if (qfourpaneviewer->objectName().isEmpty())
            qfourpaneviewer->setObjectName(QString::fromUtf8("qfourpaneviewer"));
        qfourpaneviewer->resize(400, 300);

        retranslateUi(qfourpaneviewer);

        QMetaObject::connectSlotsByName(qfourpaneviewer);
    } // setupUi

    void retranslateUi(QWidget *qfourpaneviewer)
    {
        qfourpaneviewer->setWindowTitle(QApplication::translate("QFourpaneviewer", "Form", nullptr));
    } // retranslateUi

};

namespace Ui {
    class QFourpaneviewer: public Ui_QFourpaneviewer {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_QFOURPANEVIEWER_H
