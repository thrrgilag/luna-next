/*
 * Copyright (C) 2013 Simon Busch <morphis@gravedo.de>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>
 */

#ifndef LUNA_COMPOSITORWINDOW_H_
#define LUNA_COMPOSITORWINDOW_H_

#include <QWaylandQuickSurface>
#include <QWaylandSurfaceItem>

#include "eventtype.h"

namespace luna
{

class CompositorWindow : public QWaylandSurfaceItem
{
    Q_OBJECT
    Q_PROPERTY(int winId READ winId CONSTANT)
    Q_PROPERTY(int windowType READ windowType CONSTANT)
    Q_PROPERTY(QString appId READ appId CONSTANT)
    Q_PROPERTY(quint64 processId READ processId CONSTANT)
    Q_PROPERTY(QVariant userData READ userData WRITE setUserData NOTIFY userDataChanged)
    Q_PROPERTY(bool ready READ ready NOTIFY readyChanged)

public:
    CompositorWindow(unsigned int winId, QWaylandQuickSurface *surface, QQuickItem *parent = 0);
    virtual ~CompositorWindow();

    unsigned int winId() const;
    unsigned int windowType() const;
    QString appId() const;
    quint64 processId() const;
    bool ready() const;

    QVariant userData() const;
    void setUserData(QVariant);

    void setClosed(bool closed);
    void tryRemove();

    bool checkIsAllowedToStay();

    Q_INVOKABLE void postEvent(int event);
    Q_INVOKABLE void changeSize(const QSize& size);

signals:
    void userDataChanged();
    void readyChanged();

public slots:
    void onWindowPropertyChanged(const QString&, const QVariant&);

protected:
    virtual bool event(QEvent *event);

private:
    unsigned int mId;
    unsigned int mWindowType;
    bool mClosed;
    bool mRemovePosted;
    QString mAppId;
    QVariant mUserData;
    bool mReady;

    void checkStatus();
};

} // namespace luna

#endif
