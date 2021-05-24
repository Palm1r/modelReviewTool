#pragma once
#include "qquickbgfxitem/qquickbgfxitem.h"

#include <bgfx/platform.h>

#include <QObject>

#include <vector>

#include "3dparts/model.h"
#include "3dparts/shader.h"
#include "3dparts/transform.h"
#include "3dparts/camera.h"
#include <entt/entt.hpp>
#include <openfbx/ofbx.h>

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

    void open_fbx(const QString &url);
    void load_model(const QString& url);

public slots:
    void init_example(const bgfx::Init &init);
    void render_example();
    void render_scene();

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

    Model m_model;
    ofbx::IScene* g_scene = nullptr;
    bool initModel = false;
    std::shared_ptr<Shader> program;
    bgfx::UniformHandle u_diffuse_color = BGFX_INVALID_HANDLE;

//    // todo put these into base class
    entt::registry scene;
    entt::entity camera;
};
}    // namespace QQuickBgfx
