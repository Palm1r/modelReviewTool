#pragma once
#include "qquickbgfxitem/qquickbgfxitem.h"

#include <bgfx/platform.h>

#include <QObject>

#include <vector>
#include <cubes.h>

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

public slots:
    void init_example(const bgfx::Init &init);
    void render_example();

private slots:
    void renderFrame();
    void shutdown();
    void init();

signals:
    void render(const std::vector<QQuickBgfxItem *> &);
    void initialized(bgfx::Init &);

private:
    QQuickWindow *m_window{nullptr};
    std::vector<QQuickBgfxItem *> m_bgfxItems;
    bgfx::Init m_bgfxInit;
    ExampleCubes bgfxExample;
};
}    // namespace QQuickBgfx
