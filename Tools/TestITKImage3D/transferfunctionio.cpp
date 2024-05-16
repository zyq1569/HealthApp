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

#include "transferfunctionio.h"

#include <QColor>
#include <QDomDocument>
#include <QFile>
#include <QFileInfo>
#include <QTextStream>

//#include "logging.h"
#include "transferfunction.h"

namespace udg {

TransferFunctionIO::TransferFunctionIO()
{
}

TransferFunctionIO::~TransferFunctionIO()
{
}

TransferFunction* TransferFunctionIO::fromFile(QFile &file)
{
    if (!file.open(QFile::ReadOnly | QFile::Text))
    {
//        DEBUG_LOG(qPrintable(QString("No es pot llegir des del fitxer %1").arg(file.fileName())));
        return 0;
    }

    QTextStream in(&file);
    TransferFunction *transferFunction = new TransferFunction();
    // Nom per defecte
    transferFunction->setName(QFileInfo(file).fileName());
    enum { NAME, COLOR, OPACITY } mode = COLOR;

    while (!in.atEnd())
    {
        QString line = in.readLine();
        QTextStream lineIn(&line, QIODevice::ReadOnly);
        QString first;
        bool ok;
        lineIn >> first;
        first = first.trimmed();

        if (first.isEmpty())
        {
            continue;
        }
        else if (first == "[Name]")
        {
            mode = NAME;
        }
        else if (first == "[Color]")
        {
            mode = COLOR;
        }
        else if (first == "[Opacity]")
        {
            mode = OPACITY;
        }
        else
        {
            if (mode == NAME)
            {
                transferFunction->setName(line.trimmed());
            }
            else
            {
                double x = first.toDouble(&ok);

                if (ok)
                {
                    if (mode == COLOR)
                    {
                        double r, g, b;
                        lineIn >> r >> g >> b;
                        transferFunction->setColor(x, r, g, b);
                    }
                    else
                    {
                        double opacity;
                        lineIn >> opacity;
                        transferFunction->setOpacity(x, opacity);
                    }
                }
                else
                {
                    continue;
                }
            }
        }
    }

    file.close();

    return transferFunction;
}

TransferFunction* TransferFunctionIO::fromFile(const QString &fileName)
{
    QFile file(fileName);
    return fromFile(file);
}

TransferFunction* TransferFunctionIO::fromXmlFile(QFile &file)
{
    /// \TODO afegir-hi comprovació d'errors
    QDomDocument xml;

    if (!file.open(QFile::ReadOnly | QFile::Text))
    {
//        DEBUG_LOG(QString("No es pot llegir des del fitxer %1").arg(file.fileName()));
        return 0;
    }

    QString errorMsg;
    int errorLine, errorColumn;

    if (!xml.setContent(&file, &errorMsg, &errorLine, &errorColumn))
    {
        file.close();
//        DEBUG_LOG(errorMsg + QString(". Línia: %1. Columna: %2.").arg(errorLine).arg(errorColumn));
        return 0;
    }

    file.close();

    TransferFunction *transferFunction = new TransferFunction();

    QDomElement transferFunctionElement = xml.documentElement();

    // Nom
    QDomNode nameNode = transferFunctionElement.elementsByTagName("name").item(0);
    if (!nameNode.isNull())
    {
        transferFunction->setName(nameNode.toElement().text());
    }
    else
    {
        // Nom per defecte
        transferFunction->setName(QFileInfo(file).fileName());
    }

    // Color
    QDomNode colorNode = transferFunctionElement.elementsByTagName("color").item(0);
    if (!colorNode.isNull())
    {
        QDomElement colorElement = colorNode.toElement();
        QDomNodeList colorPoints = colorElement.elementsByTagName("point");

        for (int i = 0; i < colorPoints.length(); i++)
        {
            QDomElement colorPointElement = colorPoints.item(i).toElement();
            transferFunction->setColor(colorPointElement.attribute("value").toDouble(),
                                       colorPointElement.attribute("r").toDouble(),
                                       colorPointElement.attribute("g").toDouble(),
                                       colorPointElement.attribute("b").toDouble());
        }
    }

    // Opacitat
    QDomNode opacityNode = transferFunctionElement.elementsByTagName("opacity").item(0);
    if (!opacityNode.isNull())
    {
        QDomElement opacityElement = opacityNode.toElement();
        QDomNodeList opacityPoints = opacityElement.elementsByTagName("point");

        for (int i = 0; i < opacityPoints.length(); i++)
        {
            QDomElement opacityPointElement = opacityPoints.item(i).toElement();
            transferFunction->setOpacity(opacityPointElement.attribute("value").toDouble(),
                                         opacityPointElement.attribute("a").toDouble());
        }
    }

    // Opacitat del gradient
    QDomNode gradientOpacityNode = transferFunctionElement.elementsByTagName("gradientopacity").item(0);
    if (!gradientOpacityNode.isNull())
    {
        QDomElement gradientOpacityElement = gradientOpacityNode.toElement();
        QDomNodeList gradientOpacityPoints = gradientOpacityElement.elementsByTagName("point");

        for (int i = 0; i < gradientOpacityPoints.length(); i++)
        {
            QDomElement gradientOpacityPointElement = gradientOpacityPoints.item(i).toElement();
            transferFunction->setGradientOpacity(gradientOpacityPointElement.attribute("gradient").toDouble(),
                                                 gradientOpacityPointElement.attribute("a").toDouble());
        }
    }

    return transferFunction;
}

TransferFunction* TransferFunctionIO::fromXmlFile(const QString &fileName)
{
    QFile file(fileName);
    return fromXmlFile(file);
}

void TransferFunctionIO::toFile(QFile &file, const TransferFunction &transferFunction)
{
    if (!file.open(QFile::WriteOnly | QFile::Truncate | QFile::Text))
    {
//        DEBUG_LOG(qPrintable(QString("No es pot escriure al fitxer %1").arg(file.fileName())));
        return;
    }

    QTextStream out(&file);

    out << "[Name]\n";
    out << transferFunction.name() << "\n";

    out << "\n";

    out << "[Color]\n";
    QList<double> colorPoints = transferFunction.colorKeys();
    foreach (double x, colorPoints)
    {
        QColor color = transferFunction.getColor(x);
        out << x << " " << color.redF() << " " << color.greenF() << " " << color.blueF() << "\n";
    }

    out << "\n";

    out << "[Opacity]\n";
    QList<double> opacityPoints = transferFunction.opacityKeys();
    foreach (double x, opacityPoints)
    {
        out << x << " " << transferFunction.getOpacity(x) << "\n";
    }

    out.flush();
    file.close();
}

void TransferFunctionIO::toFile(const QString &fileName, const TransferFunction &transferFunction)
{
    QFile file(fileName);
    toFile(file, transferFunction);
}

void TransferFunctionIO::toXmlFile(QFile &file, const TransferFunction &transferFunction)
{
    if (!file.open(QFile::WriteOnly | QFile::Truncate | QFile::Text))
    {
//        DEBUG_LOG(qPrintable(QString("No es pot escriure al fitxer %1").arg(file.fileName())));
        return;
    }

    QDomDocument xml;

    QDomElement transferFunctionElement = xml.createElement("transferfunction");
    transferFunctionElement.setAttribute("version", 0);
    xml.appendChild(transferFunctionElement);

    // Nom
    QDomElement nameElement = xml.createElement("name");
    transferFunctionElement.appendChild(nameElement);
    QDomText nameText = xml.createTextNode(transferFunction.name());
    nameElement.appendChild(nameText);

    // Color
    QDomElement colorElement = xml.createElement("color");
    transferFunctionElement.appendChild(colorElement);
    QList<double> colorPoints = transferFunction.colorKeys();
    foreach (double x, colorPoints)
    {
        QColor color = transferFunction.getColor(x);
        QDomElement colorPointElement = xml.createElement("point");
        colorPointElement.setAttribute("value", x);
        colorPointElement.setAttribute("r", color.redF());
        colorPointElement.setAttribute("g", color.greenF());
        colorPointElement.setAttribute("b", color.blueF());
        colorElement.appendChild(colorPointElement);
    }

    // Opacitat
    QDomElement opacityElement = xml.createElement("opacity");
    transferFunctionElement.appendChild(opacityElement);
    QList<double> opacityPoints = transferFunction.opacityKeys();
    foreach (double x, opacityPoints)
    {
        QDomElement opacityPointElement = xml.createElement("point");
        opacityPointElement.setAttribute("value", x);
        opacityPointElement.setAttribute("a", transferFunction.getOpacity(x));
        opacityElement.appendChild(opacityPointElement);
    }

    // Opacitat del gradient
    QDomElement gradientOpacityElement = xml.createElement("gradientopacity");
    transferFunctionElement.appendChild(gradientOpacityElement);
    QList<double> gradientOpacityPoints = transferFunction.gradientOpacityKeys();
    foreach (double y, gradientOpacityPoints)
    {
        QDomElement gradientOpacityPointElement = xml.createElement("point");
        gradientOpacityPointElement.setAttribute("gradient", y);
        gradientOpacityPointElement.setAttribute("a", transferFunction.getGradientOpacity(y));
        gradientOpacityElement.appendChild(gradientOpacityPointElement);
    }

    QTextStream out(&file);
    out << xml.toString();
    out.flush();
    file.close();
}

void TransferFunctionIO::toXmlFile(const QString &fileName, const TransferFunction &transferFunction)
{
    QFile file(fileName);
    toXmlFile(file, transferFunction);
}

}
