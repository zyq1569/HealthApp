/*************************************************************************************
  Copyright (C) 2014 Laboratori de Gràfics i Imatge, Universitat de Girona &
  Institut de Diagnòstic per la Imatge.
  Girona 2014. All rights reserved.
  http://starviewer.udg.edu

  This file is part of the Starviewer (Medical Imaging Software) open source project.
  It is subject to the license terms in the LICENSE file found in the top-level
  directory of this distribution and at http://starviewer.udg.edu/license. No part of
  the Starviewer (Medical Imaging Software) open source project, including this file,
  may be copied, modified, propagated, or distributed except according to the
  terms contained in the LICENSE file.
 *************************************************************************************/

#include "opacitytransferfunction.h"

#include <QVariant>

#include <vtkPiecewiseFunction.h>

namespace udg {

OpacityTransferFunction::OpacityTransferFunction()
 : m_vtkOpacityTransferFunction(0)
{
}

OpacityTransferFunction::OpacityTransferFunction(const OpacityTransferFunction &opacityTransferFunction)
 : TransferFunctionTemplate<double>(opacityTransferFunction), m_vtkOpacityTransferFunction(0)
{
}

OpacityTransferFunction::~OpacityTransferFunction()
{
    if (m_vtkOpacityTransferFunction)
    {
        m_vtkOpacityTransferFunction->Delete();
    }
}

OpacityTransferFunction& OpacityTransferFunction::operator =(const OpacityTransferFunction &opacityTransferFunction)
{
    if (this != &opacityTransferFunction)
    {
        TransferFunctionTemplate<double>::operator =(opacityTransferFunction);
        m_vtkOpacityTransferFunction = 0;
    }

    return *this;
}

vtkPiecewiseFunction* OpacityTransferFunction::vtkOpacityTransferFunction() const
{
    if (m_vtkOpacityTransferFunction)
    {
        m_vtkOpacityTransferFunction->Delete();
    }
    m_vtkOpacityTransferFunction = vtkPiecewiseFunction::New();

    QMapIterator<double, double> it(m_map);

    while (it.hasNext())
    {
        it.next();
        m_vtkOpacityTransferFunction->AddPoint(it.key(), it.value());
    }

    return m_vtkOpacityTransferFunction;
}

QString OpacityTransferFunction::toString() const
{
    QString string;
    QMapIterator<double, double> it(m_map);

    while (it.hasNext())
    {
        it.next();
        string += QString("x = %1, opacity = %2\n").arg(it.key()).arg(it.value());
    }

    return string;
}

QVariant OpacityTransferFunction::toVariant() const
{
    QMap<QString, QVariant> map;
    QMapIterator<double, double> it(m_map);

    while (it.hasNext())
    {
        it.next();
        map[QString::number(it.key())] = it.value();
    }

    QMap<QString, QVariant> variant;
    variant["name"] = m_name;
    variant["map"] = map;

    return variant;
}

OpacityTransferFunction OpacityTransferFunction::fromVariant(const QVariant &variant)
{
    OpacityTransferFunction opacityTransferFunction;
    QMap<QString, QVariant> variantMap = variant.toMap();
    opacityTransferFunction.m_name = variantMap.value("name").toString();
    QMap<QString, QVariant> map = variantMap.value("map").toMap();
    QMapIterator<QString, QVariant> it(map);

    while (it.hasNext())
    {
        it.next();
        opacityTransferFunction.m_map[it.key().toDouble()] = it.value().toDouble();
    }

    return opacityTransferFunction;
}

} // End namespace udg
