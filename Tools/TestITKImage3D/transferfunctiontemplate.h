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

#ifndef TRANSFERFUNCTIONTEMPLATE_H
#define TRANSFERFUNCTIONTEMPLATE_H

//#include "logging.h"
#include "mathtools.h"

#include <QMap>
#include <QString>

namespace udg {

// Forward declare the class template
template <typename T>
class TransferFunctionTemplate;

// Forward declare the stream operators (see http://web.mst.edu/~nmjxv3/articles/templates.html and http://en.cppreference.com/w/cpp/language/friend)
template <typename T>
QDataStream& operator <<(QDataStream &stream, const TransferFunctionTemplate<T> &transferFunction);
template <typename T>
QDataStream& operator >>(QDataStream &stream, TransferFunctionTemplate<T> &transferFunction);

/**
    Representa una funció de transferència bàsica f: X -> Y, on X és un conjunt de valors reals (valors de propietat o magnitud del gradient)
    i Y un conjunt de valors de tipus T.
    Aquesta funció té uns quants punts definits explícitament i la resta s'obtenen per interpolació lineal o extrapolació del veí més proper.
    La funció de transferència també té un nom.

    Aquesta template té el constructor protegit, de manera que no es pot instanciar directament, sinó que se n'ha de fer una subclasse per
    cada tipus T que es vulgui.
  */
template <typename T>
class TransferFunctionTemplate {

public:
    /// Retorna cert si els noms i les funcions són iguals.
    /// \note Les funcions poden ser diferents però equivalents. Són equivalents si un cop simplificades són iguals.
    bool operator ==(const TransferFunctionTemplate<T> &transferFunction) const;

    /// Retorna el nom.
    const QString& name() const;
    /// Assigna el nom.
    void setName(const QString &name);

    /// Retorna la y corresponent a x.
    T operator ()(double x) const;
    /// Retorna la y corresponent a x.
    T get(double x) const;
    /// Defineix explícitament el punt (x,y).
    void set(double x, const T &y);
    /// Esborra la definició explícita del punt (x,y) si existeix.
    void unset(double x);
    /// Esborra tots els punts definits explícitament.
    void clear();
    /// Retorna cert si hi ha un punt (x,y) definit explícitament, i fals altrament.
    bool isSet(double x) const;

    /// Returns true if this transfer function doesn't have any point defined.
    bool isEmpty() const;

    /// Retorna la llista de valors x de tots els punts (x,y) definits explícitament.
    QList<double> keys() const;

    /// Retalla la funció de manera que només tingui punts explícits en el rang [x1, x2] i tingui punts explícits a x1 i x2.
    void trim(double x1, double x2);
    /// Simplifica la funció esborrant els punts (x,y) que es poden obtenir per interpolació o extrapolació.
    void simplify();

    /// Writes the given transfer function to the given stream.
    friend QDataStream& operator << <>(QDataStream &stream, const TransferFunctionTemplate &transferFunction);
    /// Fills the given transfer function from the given stream.
    friend QDataStream& operator >> <>(QDataStream &stream, TransferFunctionTemplate &transferFunction);

protected:
    /// Crea una funció de transferència buida, sense cap punt i sense nom.
    TransferFunctionTemplate();

protected:
    /// Nom de la funció de transferència.
    QString m_name;
    /// Mapa amb els punts (x,y) definits explícitament.
    QMap<double, T> m_map;

private:

    /// Returns a default value of type T.
    static T defaultValue();
    /// Retorna la interpolació lineal entre a i b avaluant "a + alpha * (b - a)", que és equivalent a "a * (1 - alpha) + b * alpha".
    static T linearInterpolation(const T &a, const T &b, double alpha);

};

template <typename T>
bool TransferFunctionTemplate<T>::operator ==(const TransferFunctionTemplate<T> &transferFunction) const
{
    return m_name == transferFunction.m_name && m_map == transferFunction.m_map;
}

template <typename T>
const QString& TransferFunctionTemplate<T>::name() const
{
    return m_name;
}

template <typename T>
void TransferFunctionTemplate<T>::setName(const QString &name)
{
    m_name = name;
}

template <typename T>
T TransferFunctionTemplate<T>::operator ()(double x) const
{
    return get(x);
}

template <typename T>
T TransferFunctionTemplate<T>::get(double x) const
{
    Q_ASSERT(!MathTools::isNaN(x));

    if (m_map.isEmpty())
    {
//        DEBUG_LOG("La funció de transferència és buida. Retornem un valor per defecte.");
//        WARN_LOG("La funció de transferència és buida. Retornem un valor per defecte.");
        return defaultValue();
    }

    typename QMap<double, T>::const_iterator lowerBound = m_map.lowerBound(x);

    // > últim
    if (lowerBound == m_map.end())
    {
        return (--lowerBound).value();
    }

    // Exacte o < primer
    if (lowerBound.key() == x || lowerBound == m_map.begin())
    {
        return lowerBound.value();
    }

    typename QMap<double, T>::const_iterator a = lowerBound - 1, b = lowerBound;
    double alpha = (x - a.key()) / (b.key() - a.key());

    return linearInterpolation(a.value(), b.value(), alpha);
}

template <typename T>
void TransferFunctionTemplate<T>::set(double x, const T &y)
{
    // Problema amb QTransferFunctionEditorByGradient
    // Q_ASSERT(!MathTools::isNaN(x));
    m_map[x] = y;
}

template <typename T>
void TransferFunctionTemplate<T>::unset(double x)
{
    Q_ASSERT(!MathTools::isNaN(x));
    m_map.remove(x);
}

template <typename T>
void TransferFunctionTemplate<T>::clear()
{
    m_map.clear();
}

template <typename T>
bool TransferFunctionTemplate<T>::isSet(double x) const
{
    return m_map.contains(x);
}

template <typename T>
bool TransferFunctionTemplate<T>::isEmpty() const
{
    return m_map.isEmpty();
}

template <typename T>
QList<double> TransferFunctionTemplate<T>::keys() const
{
    return m_map.keys();
}

template <typename T>
void TransferFunctionTemplate<T>::trim(double x1, double x2)
{
    Q_ASSERT(!MathTools::isNaN(x1));
    Q_ASSERT(!MathTools::isNaN(x2));
    Q_ASSERT(x1 <= x2);

    if (m_map.isEmpty())
    {
//        DEBUG_LOG("La funció de transferència és buida.");
//        WARN_LOG("La funció de transferència és buida.");
        return;
    }

    // Primer afegim els extrems. Si ja existeixen es quedarà igual.
    set(x1, get(x1));
    set(x2, get(x2));

    // Després eliminem els punts fora del rang.
    QMutableMapIterator<double, T> it(m_map);
    while (it.hasNext())
    {
        it.next();
        if (it.key() < x1)
        {
            it.remove();
        }
        else
        {
            break;
        }
    }
    it.toBack();
    while (it.hasPrevious())
    {
        it.previous();
        if (it.key() > x2)
        {
            it.remove();
        }
        else
        {
            break;
        }
    }
}

template <typename T>
void TransferFunctionTemplate<T>::simplify()
{
    QList<double> keys = m_map.keys();

    foreach (double x, keys)
    {
        if (m_map.size() == 1)
        {
            // Cas especial: si només queda un punt no l'hem de treure
            break;
        }
        T y = get(x);
        unset(x);
        if (get(x) != y)
        {
            set(x, y);
        }
    }
}

template <typename T>
QDataStream& operator <<(QDataStream &stream, const TransferFunctionTemplate<T> &transferFunction)
{
    return stream << transferFunction.m_name << transferFunction.m_map;
}

template <typename T>
QDataStream& operator >>(QDataStream &stream, TransferFunctionTemplate<T> &transferFunction)
{
    return stream >> transferFunction.m_name >> transferFunction.m_map;
}

template <typename T>
TransferFunctionTemplate<T>::TransferFunctionTemplate()
{
}

template <typename T>
T TransferFunctionTemplate<T>::defaultValue()
{
    return T();
}

template <typename T>
T TransferFunctionTemplate<T>::linearInterpolation(const T &a, const T &b, double alpha)
{
    Q_ASSERT(!MathTools::isNaN(alpha));
    return a + alpha * (b - a);
}

} // End namespace udg

#endif // TRANSFERFUNCTIONTEMPLATE_H
