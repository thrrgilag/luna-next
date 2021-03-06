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

#include "compositor.h"
#include "windowmodel.h"

#include <QWaylandInputDevice>

#include <QtCore/QtGlobal>

namespace luna
{

static CompositorWindow *surfaceWindow(QWaylandSurface *surface)
{
    return surface->views().isEmpty() ? 0 : static_cast<CompositorWindow*>(surface->views().first());
}

Compositor* Compositor::mInstance = 0;

Compositor::Compositor()
    : QWaylandQuickCompositor(this),
      mFullscreenSurface(0),
      mNextWindowId(1)
{
    setColor(Qt::black);
    setRetainedSelectionEnabled(true);
    addDefaultShell();

    if (mInstance)
        qFatal("Compositor: Only one compositor instance per process is supported");

    qDebug() << __PRETTY_FUNCTION__;

    mInstance = this;

    connect(this, SIGNAL(frameSwapped()), this, SLOT(frameSwappedSlot()));
}

Compositor* Compositor::instance()
{
    return mInstance;
}

void Compositor::classBegin()
{
}

void Compositor::componentComplete()
{
    QWaylandCompositor::setOutputGeometry(QRect(0, 0, width(), height()));
}

void Compositor::registerWindowModel(WindowModel *model)
{
    mWindowModels.append(model);
}

void Compositor::unregisterWindowModel(WindowModel *model)
{
    mWindowModels.removeAll(model);
}

CompositorWindow* Compositor::windowForId(int id)
{
    if (!mWindows.contains(id))
        return 0;

    return mWindows[id];
}

void Compositor::setFullscreenSurface(QWaylandSurface *surface)
{
    if (surface == mFullscreenSurface)
        return;

    qDebug() << Q_FUNC_INFO << surface;

    // Prevent flicker when returning to composited mode
    if (!surface && mFullscreenSurface) {
        foreach (QWaylandSurfaceView *view, mFullscreenSurface->views())
            static_cast<QWaylandSurfaceItem *>(view)->update();
    }

    mFullscreenSurface = surface;

    emit fullscreenSurfaceChanged();
}

void Compositor::clearKeyboardFocus()
{
    defaultInputDevice()->setKeyboardFocus(0);
}

void Compositor::closeWindowWithId(int winId)
{
    qDebug() << Q_FUNC_INFO << "winId" << winId;

    CompositorWindow *window = mWindows.value(winId, 0);
    if (window) {
        if (window->surface() && window->checkIsAllowedToStay())
            window->surface()->destroySurface();
        else if (window->surface())
            destroyClientForSurface(window->surface());
    }
}

CompositorWindow* Compositor::createWindowForSurface(QWaylandSurface *surface)
{
    unsigned int windowId = mNextWindowId++;

    qDebug() << Q_FUNC_INFO << "windowId" << windowId << surface;

    CompositorWindow *window = new CompositorWindow(windowId, static_cast<QWaylandQuickSurface*>(surface), contentItem());
    window->setSize(surface->size());
    // window->setPosition(surface->pos());
    window->setFlag(QQuickItem::ItemIsFocusScope, true);
    // window->setUseTextureAlpha(true);
    QObject::connect(surface, &QWaylandSurface::surfaceDestroyed, this, &Compositor::surfaceDying);

    mWindows.insert(windowId, window);

    return window;
}

QWaylandSurfaceItem* Compositor::createView(QWaylandSurface *surface)
{
    qDebug() << __PRETTY_FUNCTION__ << "surface" << surface;

    return createWindowForSurface(surface);
}

void Compositor::surfaceMapped()
{
    qDebug() << __PRETTY_FUNCTION__;

    QWaylandSurface *surface = qobject_cast<QWaylandSurface *>(sender());

    CompositorWindow *window = surfaceWindow(surface);
    if (!window)
        window = createWindowForSurface(surface);

    window->setTouchEventsEnabled(true);

    qDebug() << __PRETTY_FUNCTION__ << window << "appId" << window->appId() << "windowType" << window->windowType();

    if (WindowModel::isWindowAlreadyAdded(mWindowModels, window)) {
        emit windowShown(QVariant::fromValue(static_cast<QQuickItem*>(window)));
    }
    else {
        emit windowAdded(QVariant::fromValue(static_cast<QQuickItem*>(window)));
        WindowModel::addWindowForEachModel(mWindowModels, window);
    }
}

void Compositor::surfaceUnmapped()
{
    qDebug() << __PRETTY_FUNCTION__;

    QWaylandSurface *surface = qobject_cast<QWaylandSurface *>(sender());
    if (surface == mFullscreenSurface)
        setFullscreenSurface(0);

    CompositorWindow *window = surfaceWindow(surface);
    qWarning() << Q_FUNC_INFO << window;

    emit windowHidden(QVariant::fromValue(static_cast<QQuickItem*>(window)));

    WindowModel::removeWindowForEachModel(mWindowModels, window);
}

void Compositor::surfaceDying()
{
    QWaylandSurface *surface = static_cast<QWaylandSurface *>(sender());
    qDebug() << Q_FUNC_INFO << surface;

    CompositorWindow *window = surfaceWindow(surface);

    if (surface == mFullscreenSurface)
        setFullscreenSurface(0);

    if (window) {
        WindowModel::removeWindowForEachModel(mWindowModels, window);

        mWindows.remove(window->winId());
        emit windowRemoved(QVariant::fromValue(static_cast<QQuickItem*>(window)));

        window->setClosed(true);
        window->tryRemove();
    }
}

void Compositor::frameSwappedSlot()
{
#if QT_VERSION > QT_VERSION_CHECK(5,2,1)
    sendFrameCallbacks(surfaces());
#else
    frameFinished(mFullscreenSurface);
#endif
}

void Compositor::resizeEvent(QResizeEvent *event)
{
    QQuickView::resizeEvent(event);
    QWaylandCompositor::setOutputGeometry(QRect(0, 0, width(), height()));
}

void Compositor::surfaceCreated(QWaylandSurface *surface)
{
    qDebug() << __PRETTY_FUNCTION__ << "surface" << surface;

    Q_UNUSED(surface);
    connect(surface, SIGNAL(mapped()), this, SLOT(surfaceMapped()));
    connect(surface, SIGNAL(unmapped()), this, SLOT(surfaceUnmapped()));
    connect(surface, SIGNAL(raiseRequested()), this, SLOT(surfaceRaised()));
    connect(surface, SIGNAL(lowerRequested()), this, SLOT(surfaceLowered()));
    connect(surface, SIGNAL(sizeChanged()), this, SLOT(surfaceSizeChanged()));
#if QT_VERSION > QT_VERSION_CHECK(5,2,1)
    connect(surface, SIGNAL(damaged(const QRegion)), this, SLOT(surfaceDamaged(QRegion)));
#else
    connect(surface, SIGNAL(damaged(const QRect)), this, SLOT(surfaceDamaged(const QRect&)));
#endif
}

void Compositor::surfaceRaised()
{
    QWaylandSurface *surface = qobject_cast<QWaylandSurface*>(sender());
    CompositorWindow *window = surfaceWindow(surface);

    qWarning() << Q_FUNC_INFO << "the window " << window << "is going to be raised";

    if (window)
        emit windowRaised(QVariant::fromValue(static_cast<QQuickItem*>(window)));
}

void Compositor::surfaceLowered()
{
    QWaylandSurface *surface = qobject_cast<QWaylandSurface*>(sender());
    CompositorWindow *window = surfaceWindow(surface);

    qWarning() << Q_FUNC_INFO << "the window " << window << "is going to be lowered";

    if (window)
        emit windowLowered(QVariant::fromValue(static_cast<QQuickItem*>(window)));
}

void Compositor::surfaceSizeChanged()
{
    QWaylandSurface *surface = qobject_cast<QWaylandSurface *>(sender());

    CompositorWindow *window = surfaceWindow(surface);
    if (window)
        window->setSize(surface->size());
}

#if QT_VERSION > QT_VERSION_CHECK(5,2,1)
void Compositor::surfaceDamaged(const QRegion &)
#else
void Compositor::surfaceDamaged(const QRect&)
#endif
{
    if (!isVisible())
#if QT_VERSION > QT_VERSION_CHECK(5,2,1)
        sendFrameCallbacks(surfaces());
#else
        frameFinished(0);
#endif
}


} // namespace luna
