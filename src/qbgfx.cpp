#include <qbgfx.h>

#include "qquickbgfxitem/qquickbgfxitem.h"

#include <bgfx/bgfx.h>

#include <QList>
#include <QFile>
#include <QQuickWindow>
//#include <QGuiApplication>
#include <stdexcept>

#include <bgfx/bgfx.h>
#include <bx/bx.h>
#include <bx/math.h>
#include <debugdraw/debugdraw.h>

#include <iostream>
#include <glm/glm.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/quaternion.hpp>

#ifdef __linux__
#include <QOpenGLContext>
#endif

#ifdef __APPLE__
#include <Cocoa/Cocoa.h>
#include <QuartzCore/CAMetalLayer.h>
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
    QObject::connect(this, &QBgfx::render, &QBgfx::render_scene);

    m_bgfxItems.reserve(m_bgfxItems.size());
    m_bgfxItems.insert(m_bgfxItems.end(), items.begin(), items.end());
}

QBgfx::~QBgfx()
{
    //    program.reset();
    //    bgfx::destroy(u_diffuse_color);
    //    model.reset();
    //    shutdown();
}

void QBgfx::open_fbx(const QString &url)
{
    //    QFile fbxfile(url);
    //    qDebug() << "fbxfile exist" << fbxfile.exists();
    //    if (!fbxfile.open(QIODevice::ReadOnly))
    //        return;

    //    auto* content = new ofbx::u8[fbxfile.size()];


    //    g_scene = ofbx::load((ofbx::u8*)content, fbxfile.size(), (ofbx::u64)ofbx::LoadFlags::TRIANGULATE);

    FILE* fp = std::fopen("D:\\projects\\modelReviewTool\\Helmet.fbx", "rb");

    if (!fp) {
        qDebug() << "file not open";
    }

    fseek(fp, 0, SEEK_END);
    long file_size = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    auto* content = new ofbx::u8[file_size];
    fread(content, 1, file_size, fp);
    g_scene = ofbx::load((ofbx::u8*)content, file_size, (ofbx::u64)ofbx::LoadFlags::TRIANGULATE);

    if (!g_scene) {
        qDebug() << "scene not open" << url << file_size;
        OutputDebugString(ofbx::getError());
    }
    fclose(fp);
    //    fbxfile.close();
}

void QBgfx::load_model(const QString &url)
{
    open_fbx("D:\\projects\\modelReviewTool\\Helmet.fbx");

    std::vector<MeshDataTuple> mdt;

    if (!g_scene) {
        qDebug() << "scene not open";
        OutputDebugString(ofbx::getError());
    }

    for(auto mesh_idx = 0; mesh_idx < g_scene->getMeshCount(); ++mesh_idx) {
        const auto* mesh = g_scene->getMesh(mesh_idx);
        const auto* geometry = mesh->getGeometry();

        std::vector<Vertex> vertices;
        std::vector<u16> indices;

        // for now, VertexCount == IndexCount because of default triangulation
        for(auto face_idx = 0; face_idx < geometry->getIndexCount(); ++face_idx) {
            const auto vert_idx = [=]() {
                const auto idx = geometry->getFaceIndices()[face_idx];
                return idx < 0 ? -idx - 1 : idx;
            }();

                auto vertex = Vertex{
                    {
                        geometry->getVertices()[vert_idx].x,
                        geometry->getVertices()[vert_idx].y,
                        geometry->getVertices()[vert_idx].z,
                        },
                    {
                        geometry->getNormals()[face_idx].x,
                        geometry->getNormals()[face_idx].y,
                        geometry->getNormals()[face_idx].z,
                        },
                    // todo: handle models that don't specify uvs
                    geometry->getUVs(0)
                        ? glm::vec2{geometry->getUVs(0)[face_idx].x, geometry->getUVs(0)[face_idx].y}
                        : glm::vec2{0, 0}
                };

            vertices.emplace_back(vertex);
            indices.push_back(indices.size());
        }

        // todo: use ref
        const auto layout = Vertex::get_layout();
        auto vbh = bgfx::createVertexBuffer(bgfx::copy(vertices.data(), sizeof(Vertex) * vertices.size()), layout);
        auto ibh = bgfx::createIndexBuffer(bgfx::copy(indices.data(), sizeof(u16) * indices.size()));

        // todo: other material params
        glm::vec4 diffuse;
        const auto mesh_mat_cnt = mesh->getMaterialCount();
        if(mesh_mat_cnt == 0)
            diffuse = {.9, .6, .8, 1};
        else if(mesh_mat_cnt == 1) {
            const auto color = mesh->getMaterial(0)->getDiffuseColor();
            diffuse = {color.r, color.g, color.b, 1};
        }
        else {
            assert(geometry->getMaterials());
            // todo: handle multiple materials
            //const auto face_num = geometry->getIndexCount();
            //const auto len_mat_array = face_num / 3;
            //const auto mat_id = geometry->getMaterials()[vert_idx / 3];
            const auto mat_id = 0;
            const auto color = mesh->getMaterial(mat_id)->getDiffuseColor();
            diffuse = {color.r, color.g, color.b, 1};
        }

        auto matrix = mesh->getGlobalTransform();
        glm::mat4 trans;

        //for(auto row = 0; row < 4; ++row)
        //    for(auto col = 0; col < 4; ++col)
        //        trans[col][row] = matrix.m[row+col*4];

        for(auto i = 0; i < 16; ++i)
            trans[i / 4][i % 4] = matrix.m[i];

        // todo: consider pre-multiplying model matrix
        // init scale = 100, don't know why
        trans *= .01;
        trans[3][3] = 1;

        mdt.emplace_back(MeshDataTuple{vbh, ibh, diffuse, trans});
    }

    m_model.mdt = std::move(mdt);
    //    m_model.vertices = std::move(vertices);
    //    m_model.indices = std::move(indices);
    //    m_model.vbh = vbh;
    //    m_model.ibh = ibh;
    qDebug() << "size" << m_model.mdt.size();
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
    if (!initModel) {
        qDebug() << "init model";
        load_model(":/Helmet.fbx");
        u_diffuse_color = bgfx::createUniform("u_diffuse_color", bgfx::UniformType::Vec4);

        initModel = true;
    }

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
            //            qDebug() << "item view id"<< item->viewId();
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

void QBgfx::render_scene()
{
    if (!initModel) {
        qDebug() << "init model";
        load_model(":/Helmet.fbx");
        u_diffuse_color = bgfx::createUniform("u_diffuse_color", bgfx::UniformType::Vec4);

        auto teapot = scene.create();
        scene.emplace<Transform>(teapot);

        camera = scene.create();
        scene.emplace<Camera>(camera, Camera::perspective(glm::radians(60.f), (float)(m_bgfxItems[0]->dprWidth() / m_bgfxItems[0]->dprHeight()), .1f, 300.f));
        scene.emplace<Transform>(camera, Transform::look_at(
                                             glm::vec3(-15, 15, -20),
                                             glm::vec3(0, 0, 0)
                                             ));

        initModel = true;
    }

    for(const auto item : m_bgfxItems)
    {
        glm::mat4 view = scene.get<Transform>(camera).view_matrix();
        glm::mat4 proj = scene.get<Camera>(camera).matrix();
        bgfx::setViewTransform(item->viewId(), glm::value_ptr(view), glm::value_ptr(proj));
        bgfx::setViewRect(item->viewId(), 0, 0, uint16_t(item->dprWidth()), uint16_t(item->dprHeight()) );

        const bx::Vec3 at  = { 0.0f, 0.0f,   0.0f };
        const bx::Vec3 eye = { std::clamp(item->mousePosition()[0]/ (float)item->width()-0.5f, -0.5f, 0.5f) * 15.0f, 0.0f, std::clamp(item->mousePosition()[1] / (float)item->height()-0.5f, -0.5f, 0.5f) * 15.0f };
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
