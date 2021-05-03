#pragma once
#include "qquickbgfxitem/qquickbgfxitem.h"

#include <bgfx/platform.h>

#include <QObject>

#include <vector>

class QQmlEngine;
class QQuickWindow;

namespace QQuickBgfx {

inline bool isBgfxInit()
{
    return bgfx::getInternalData()->context;
}

inline void frame()
{
    if (isBgfxInit())
    {
        bgfx::frame();
    }
}

class QBgfx : public QObject
{
    Q_OBJECT
public:
    QBgfx() = delete;
    QBgfx(QQuickWindow *, const QList<QQuickBgfxItem *>);

    ~QBgfx();

private slots:
    void renderFrame();
    void shutdown();
    void init();

signals:
    void render(const std::vector<QQuickBgfxItem *> &);
    void initialized(bgfx::Init &);

private:
    std::vector<QQuickBgfxItem *> m_bgfxItems;
    QQuickWindow *m_window{nullptr};
    bgfx::Init m_bgfxInit;
};
}    // namespace QQuickBgfx
