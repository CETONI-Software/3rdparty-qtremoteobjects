/****************************************************************************
**
** Copyright (C) 2021 Ford Motor Company
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtRemoteObjects module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl-3.0.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or (at your option) the GNU General
** Public license version 3 or any later version approved by the KDE Free
** Qt Foundation. The licenses are as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-2.0.html and
** https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include <QtCore/qiodevice.h>

#include "qremoteobjectcontainers_p.h"

QT_BEGIN_NAMESPACE

QDataStream &operator>>(QDataStream &ds, QtROSequentialContainer &p)
{
    QByteArray typeName;
    quint32 count;
    ds >> typeName;
    p.setValueType(typeName);
    ds >> count;
    p.reserve(count);
    QVariant value{p.m_valueType, nullptr};
    for (quint32 i = 0; i < count; i++) {
        if (!p.m_valueType.load(ds, value.data())) {
            qWarning("QSQ_: unable to load type '%s', returning an empty list.\n", p.m_valueTypeName.constData());
            p.clear();
            break;
        }
        p.append(value);
    }
    return ds;
}

QDataStream &operator<<(QDataStream &ds, const QtROSequentialContainer &p)
{
    ds << p.m_valueTypeName;
    auto pos = ds.device()->pos();
    quint32 count = p.count();
    ds << count;
    for (quint32 i = 0; i < count; i++) {
        if (!p.m_valueType.save(ds, p.at(i).data())) {
            ds.device()->seek(pos);
            ds.resetStatus();
            ds << quint32(0);
            qWarning("QSQ_: unable to save type '%s'.\n", p.m_valueTypeName.constData());
            break;
        }
    }
    return ds;
}

QT_END_NAMESPACE