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

#ifndef UDGTRANSFERFUNCTIONIO_H
#define UDGTRANSFERFUNCTIONIO_H

class QFile;
class QString;

namespace udg {

class TransferFunction;

/**
    Aquesta classe té les funcions d'entrada i sortida amb fitxers de les
    funcions de transferència. Només té mètodes de classe.

    Els fitxers són fitxers de text planer, amb l'extensió recomanada <b>.tf</b>.
    El format dels fitxers és el següent:

    <tt>
    [Color]
    v r g b
    ...
    [Opacity]
    v a
    ...
    </tt>

    \c v és un valor de propietat (real positiu).
    \c r \c g \c b són valors R G B del color (reals entre 0 i 1).
    \c a és un valor d'opacitat (real entre 0 i 1).

    \author Grup de Gràfics de Girona (GGG) <vismed@ima.udg.edu>
  */
class TransferFunctionIO {

public:
    /// Retorna la funció de transferència llegida des del fitxer \a file. Retorna nul si no es pot llegir el fitxer.
    static TransferFunction* fromFile(QFile &file);
    /// Retorna la funció de transferència llegida des del fitxer anomenat \a fileName. Retorna nul si no es pot llegir el fitxer.
    static TransferFunction* fromFile(const QString &fileName);

    /// Retorna la funció de transferència llegida des del fitxer XML \a file. Retorna nul si no es pot llegir el fitxer.
    static TransferFunction* fromXmlFile(QFile &file);
    /// Retorna la funció de transferència llegida des del fitxer XML anomenat \a fileName. Retorna nul si no es pot llegir el fitxer.
    static TransferFunction* fromXmlFile(const QString &fileName);

    /// Escriu la funció de transferència \a transferFunction al fitxer \a file.
    static void toFile(QFile &file, const TransferFunction &transferFunction);
    /// Escriu la funció de transferència \a transferFunction al fitxer anomentat \a fileName.
    static void toFile(const QString &fileName, const TransferFunction &transferFunction);

    /// Escriu la funció de transferència \a transferFunction al fitxer XML \a file.
    static void toXmlFile(QFile &file, const TransferFunction &transferFunction);
    /// Escriu la funció de transferència \a transferFunction al fitxer XML anomentat \a fileName.
    static void toXmlFile(const QString &fileName, const TransferFunction &transferFunction);

private:
    // Fem el constructor i el destructor privats perquè no siguin instanciables
    TransferFunctionIO();
    ~TransferFunctionIO();

};

}

#endif
