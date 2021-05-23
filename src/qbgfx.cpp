#include <qbgfx.h>

#include "qquickbgfxitem/qquickbgfxitem.h"

#include <bgfx/bgfx.h>

#include <QList>
#include <QQuickWindow>
//#include <QGuiApplication>
#include <stdexcept>

#include <bgfx/bgfx.h>
#include <bx/bx.h>
#include <bx/math.h>
#include <debugdraw/debugdraw.h>
#include <entt/entt.hpp>

#ifdef __linux__
#include <QOpenGLContext>
#endif

#ifdef _WIN32
#include <d3d11.h>
#endif

using namespace QQuickBgfx;

QBgfx::QBgfx(QQuickWindow *w, const QList<QQuickBgfxItem *> items): m_window(w)
{
    //Qt::DirectConnection needs to be specified in order to call the slot from the signal thread
    connect(m_window, &QQuickWindow::beforeFrameBegin, this, &QBgfx::init, Qt::DirectConnection);
    connect(m_window, &QQuickWindow::beforeRenderPassRecording, this, &QBgfx::renderFrame, Qt::DirectConnection);
    //Free standing function instead will always be called from the signal thread
    connect(m_window, &QQuickWindow::afterRenderPassRecording, QQuickBgfx::frame);
    //    connect(QGuiApplication::instance(), &QGuiApplication::aboutToQuit, this, &Renderer::shutdown, Qt::QueuedConnection);

    //Connection to initialized signal allows to decouple the bgfx initialization from the qquick_bgfx::QBgfx wrapper
    QObject::connect(this, &QBgfx::initialized, &QBgfx::init_example);
    //Connection to render signal allows to decouple the rendering code from the qquick_bgfx::QBgfx wrapper
    QObject::connect(this, &QBgfx::render, &QBgfx::render_example);

    m_bgfxItems.reserve(m_bgfxItems.size());
    m_bgfxItems.insert(m_bgfxItems.end(), items.begin(), items.end());
}

QBgfx::~QBgfx()
{
    //    shutdown();
}

void QBgfx::init_example(const bgfx::Init& init)
{
    if (!QQuickBgfx::isBgfxInit())
    {
        bgfx::renderFrame();
        bgfx::init(init);
        bgfx::setDebug(BGFX_DEBUG_TEXT);
        ddInit();
    }
}

void QBgfx::render_example()
{
    for(const auto item : m_bgfxItems)
    {
        if (item->viewId() < 256)
        {
            float r{0.0f};
            float g{0.0f};
            float b{0.0f};
            auto c = item->backgroundColor();
            c.setHslF(c.hueF(), c.saturationF(), c.lightnessF() * std::clamp(item->mousePosition()[1] / (float)item->height(), 0.0f, 1.0f));
            c.getRgbF(&r, &g, &b);

            const uint32_t color = uint8_t(r * 255) << 24 | uint8_t(g * 255) << 16 | uint8_t(b * 255) << 8 | 255;

            bgfx::setViewClear(item->viewId(), BGFX_CLEAR_COLOR|BGFX_CLEAR_DEPTH, color, 1.0f, 0);
            bgfx::touch(item->viewId());

            const auto w = item->dprWidth();
            const auto h = item->dprHeight();
            static float time{0.0f};
            time += 0.003f;

            const bx::Vec3 at  = { 0.0f, 0.0f,   0.0f };
            const bx::Vec3 eye = { std::clamp(item->mousePosition()[0]/ (float)item->width()-0.5f, -0.5f, 0.5f) * 15.0f, 0.0f, std::clamp(item->mousePosition()[1] / (float)item->height()-0.5f, -0.5f, 0.5f) * 15.0f };

            float view[16];
            bx::mtxLookAt(view, eye, at);

            float proj[16];
            bx::mtxProj(proj, 60.0f, float(w)/float(h), 0.1f, 100.0f, bgfx::getCaps()->homogeneousDepth);
            qDebug() << "item view id"<< item->viewId();
            bgfx::setViewTransform(item->viewId(), view, proj);
            bgfx::setViewRect(item->viewId(), 0, 0, uint16_t(w), uint16_t(h) );

            float mtx[16];
            bx::mtxRotateXY(mtx, time, time);
            DebugDrawEncoder dde;
            dde.begin(item->viewId());
            dde.drawCapsule({-2.0f, -2.0f, 0.0f}, {-2.0f, 0.0f, 0.0f}, 1.0);
            dde.drawCone({3.0f, -2.0f, 0.0f}, {3.0f, 2.0f, 0.0f}, 1.0f);
            dde.drawAxis(0.0f, 0.0f, 0.0f);
            dde.setTransform(&mtx);
            dde.draw(Aabb{{0.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 1.0f}});
            dde.end();
        }
    }
}

void QBgfx::init()
{
    QSGRendererInterface *rif = m_window->rendererInterface();
    const auto dpr = m_window->effectiveDevicePixelRatio();
    auto winHandle = reinterpret_cast<void *>(m_window->winId());
    auto context = static_cast<void *>(rif->getResource(m_window, QSGRendererInterface::DeviceResource));

    if (!isBgfxInit())
    {
        bgfx::Init init;
        init.resolution.reset = BGFX_RESET_VSYNC;
        init.resolution.width = m_window->width() * dpr;
        init.resolution.height = m_window->height() * dpr;

#ifdef _WIN32
        init.type = bgfx::RendererType::Direct3D11;
        init.platformData.context = reinterpret_cast<ID3D11Device*>(context);
#endif

#ifdef __linux__
        init.type = bgfx::RendererType::OpenGL;
        init.platformData.context = QOpenGLContext::currentContext();
#endif

#ifdef __APPLE__
        init.type = bgfx::RendererType::Metal;
        init.platformData.nwh = reinterpret_cast<CAMetalLayer *>(reinterpret_cast<NSView *>(windowHandler).layer);
        init.platformData.context = static_cast<id<MTLDevice>>(context);
#endif
        m_bgfxInit = init;
    }
    else
        m_bgfxInit = bgfx::Init();

    emit initialized(m_bgfxInit);
}

void QBgfx::renderFrame()
{
    if (!QQuickBgfx::isBgfxInit())
        return;

    m_window->beginExternalCommands();
    emit render(m_bgfxItems);
    m_window->endExternalCommands();
}

void QBgfx::shutdown()
{
    if (QQuickBgfx::isBgfxInit())
    {
        bgfx::shutdown();
    }
    m_bgfxItems.clear();
}
