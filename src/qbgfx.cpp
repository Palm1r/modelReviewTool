#include <qbgfx.h>

#include "qquickbgfxitem/qquickbgfxitem.h"

#include <bgfx/bgfx.h>

#include <QList>
#include <QQuickWindow>
//#include <QGuiApplication>
#include <stdexcept>

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

    m_bgfxItems.reserve(m_bgfxItems.size());
    m_bgfxItems.insert(m_bgfxItems.end(), items.begin(), items.end());
}

QBgfx::~QBgfx()
{
    //    shutdown();
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
