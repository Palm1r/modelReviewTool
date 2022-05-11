#include <src/qbgfx.h>
#include <qquickbgfxitem/qquickbgfxitem.h>

#include <QGuiApplication>
#include <QQuickView>

#include <bgfx/bgfx.h>
#include <bx/bx.h>
#include <bx/math.h>
//#include <debugdraw/debugdraw.h>
//#include <entt/entt.hpp>

int main(int argc, char **argv)
{
    QGuiApplication app(argc, argv);

#ifdef __APPLE__
    QQuickWindow::setGraphicsApi(QSGRendererInterface::MetalRhi);
#endif

#ifdef _WIN32
    QQuickWindow::setGraphicsApi(QSGRendererInterface::Direct3D11Rhi);
#endif

#ifdef __linux__
    QQuickWindow::setGraphicsApi(QSGRendererInterface::OpenGL);
#endif

    QQuickView view;
    view.setResizeMode(QQuickView::SizeRootObjectToView);
    view.setSource(QUrl(u"qrc:/modelReviewTool/main.qml"_qs));
    view.show();

//    const auto qbgfx = QQuickBgfx::QBgfx(static_cast<QQuickWindow*>(&view), view.rootObject()->findChildren<QQuickBgfxItem*>());
    
    return app.exec();
}
