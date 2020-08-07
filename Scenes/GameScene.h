#pragma once
#include <vector>
#include "../GLVK/VK/BufferVK.h"
#include "../GLVK/VK/ImageVK.h"
#include "../GLVK/VK/GraphicsEngineVK.h"
#include "../Interfaces/IDisposable.h"
#include "../Interfaces/IScene.h"
#include "../Structures/Model.h"

template <Disposable Texture, Disposable Buffer>
class GameScene :
    public IScene
{
public:
    GameScene(IGraphics* graphics);
    ~GameScene();

    virtual void LoadContent() override;
    virtual void Update(float deltaTime) override;
    virtual void Render(float deltaTime) override;

private:
    std::vector<Model<Texture, Buffer>*> m_models;
    std::vector<Mesh<Texture, Buffer>*> m_meshes;
};

template<Disposable Texture, Disposable Buffer>
inline GameScene<Texture, Buffer>::GameScene(IGraphics* graphics)
    : IScene(graphics)
{
}

template<Disposable Texture, Disposable Buffer>
inline GameScene<Texture, Buffer>::~GameScene()
{
}

template<Disposable Texture, Disposable Buffer>
inline void GameScene<Texture, Buffer>::LoadContent()
{
    auto model_1 = m_graphics->LoadModel("Models/Tank/tank.fbx", Vector3(1.5f, 0.0f, 1.5f), Vector3(1.0f), Vector3(45.0f), Vector4(0.0f, 0.0f, 1.0f, 1.0f));
    /*auto model_2 = m_graphics->LoadModel("Models/Tank/tank.fbx", Vector3(-0.5f, 0.0f, -0.5f), Vector3(1.75f), Vector3(-45.0f), Vector4(1.0f, 0.0f, 1.0f, 1.0f));
    auto model_3 = m_graphics->LoadModel("Models/Tank/tank.fbx", Vector3(-0.25f, 0.0f, 0.25f), Vector3(2.0f), Vector3(30.0f), Vector4(0.0f, 1.0f, 1.0f, 1.0f));*/
    auto mesh_1 = m_graphics->CreateMesh(PrimitiveType::Cube, Vector3(0.75f, 0.25f, 0.75f), Vector3(2.5f), Vector3(-30.0f), Vector4(1.0f, 0.0f, 1.0f, 1.0f));
    auto mesh_2 = m_graphics->CreateMesh(PrimitiveType::Rect, Vector3::Zero(), Vector3(20.0f), Vector3::Zero(), Vector4(0.5f, 1.0f, 0.4f, 1.0f));

    auto model1_ptr = m_models.emplace_back(dynamic_cast<Model<Texture, Buffer>*>(std::get<0>(model_1)));
    /*auto model2_ptr = m_models.emplace_back(dynamic_cast<Model<Texture, Buffer>*>(std::get<0>(model_2)));
    auto model3_ptr = m_models.emplace_back(dynamic_cast<Model<Texture, Buffer>*>(std::get<0>(model_3)));*/
    auto mesh1_ptr = m_meshes.emplace_back(dynamic_cast<Mesh<Texture, Buffer>*>(std::get<0>(mesh_1)));
    auto mesh2_ptr = m_meshes.emplace_back(dynamic_cast<Mesh<Texture, Buffer>*>(std::get<0>(mesh_2)));

    model1_ptr->ModelIndex = std::get<1>(model_1);
    /*model2_ptr->ModelIndex = std::get<1>(model_2);
    model3_ptr->ModelIndex = std::get<1>(model_3);*/
    mesh1_ptr->ModelIndex = std::get<1>(mesh_1);
    mesh2_ptr->ModelIndex = std::get<1>(mesh_2);
}

template<Disposable Texture, Disposable Buffer>
inline void GameScene<Texture, Buffer>::Update(float deltaTime)
{
    
}

template<Disposable Texture, Disposable Buffer>
inline void GameScene<Texture, Buffer>::Render(float deltaTime)
{
    if constexpr (std::is_same_v<Texture, GLVK::VK::Image> && std::is_same_v<Buffer, GLVK::VK::Buffer>)
    {
        auto graphics_ptr = dynamic_cast<GLVK::VK::GraphicsEngine*>(m_graphics);
        auto cmd_buffers = graphics_ptr->GetCommandBufferOrLists();

        for (auto& buffer : cmd_buffers)
        {
            for (auto& mesh : m_meshes)
            {
                mesh->Render<vk::CommandBuffer>(deltaTime, buffer, static_cast<uint32_t>(graphics_ptr->GetDynamicOffset()), graphics_ptr->GetPipeline(), graphics_ptr->GetDescriptorSet(), graphics_ptr->GetPushConstant());
            }

            for (auto& model : m_models)
            {
                model->Render<vk::CommandBuffer>(deltaTime, buffer, static_cast<uint32_t>(graphics_ptr->GetDynamicOffset()), graphics_ptr->GetPipeline(), graphics_ptr->GetDescriptorSet(), graphics_ptr->GetPushConstant());
            }
        }
    }
}
