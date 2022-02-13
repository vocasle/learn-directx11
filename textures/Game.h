//
// Game.h
//

#pragma once

#include "DeviceResources.h"
#include "StepTimer.h"
#include "Camera.h"
#include "Model.h"

#include <Keyboard.h>
#include <GamePad.h>
#include <Mouse.h>

namespace
{
    using namespace DirectX;
    struct Vertex
    {
        XMFLOAT3 position;
        XMFLOAT3 normal;
        XMFLOAT2 tex;
    };

    struct SceneParams
    {
        XMMATRIX worldMatrix;
        XMMATRIX viewMatrix;
        XMMATRIX projectionMatrix;
    };

    struct Material
    {
        Material() { ZeroMemory(this, sizeof(Material)); }

        XMFLOAT4 Ambient;
        XMFLOAT4 Diffuse;
        XMFLOAT4 Specular; // w = SpecPower
        XMFLOAT4 Reflect;
    };

    struct DirectionalLight
    {
        DirectionalLight() { ZeroMemory(this, sizeof(DirectionalLight)); }

        XMFLOAT4 Ambient;
        XMFLOAT4 Diffuse;
        XMFLOAT4 Specular;
        XMFLOAT3 Direction;
        float Pad;
    };

    struct PointLight
    {
        PointLight() { ZeroMemory(this, sizeof(PointLight)); }

        XMFLOAT4 Ambient;
        XMFLOAT4 Diffuse;
        XMFLOAT4 Specular;

        XMFLOAT3 Position;
        float Range;

        XMFLOAT3 Att;
        float Pad;
    };

    struct SpotLight
    {
        SpotLight() { ZeroMemory(this, sizeof(SpotLight)); }

        XMFLOAT4 Ambient;
        XMFLOAT4 Diffuse;
        XMFLOAT4 Specular;

        XMFLOAT3 Position;
        float Range;

        XMFLOAT3 Direction;
        float Spot;

        XMFLOAT3 Att;
        float Pad;
    };

    struct LightingData
    {
        DirectionalLight dirLight;
        PointLight pointLight;
        SpotLight spotLight;
        XMVECTOR eyePos;
        Material material;
        bool hasTexture = false;
    };

    static_assert((sizeof(SceneParams) % 16) == 0, "Constant buffer must always be 16-byte aligned");
    static_assert((sizeof(LightingData) % 16) == 0, "Constant buffer must always be 16-byte aligned");
}

// A basic game implementation that creates a D3D11 device and
// provides a game loop.
class Game final : public DX::IDeviceNotify
{
public:

    Game() noexcept(false);
    ~Game() = default;

    Game(Game&&) = default;
    Game& operator= (Game&&) = default;

    Game(Game const&) = delete;
    Game& operator= (Game const&) = delete;

    // Initialization and management
    void Initialize(HWND window, int width, int height);

    // Basic game loop
    void Tick();

    // IDeviceNotify
    void OnDeviceLost() override;
    void OnDeviceRestored() override;

    // Messages
    void OnActivated();
    void OnDeactivated();
    void OnSuspending();
    void OnResuming();
    void OnWindowMoved();
    void OnWindowSizeChanged(int width, int height);

    // Properties
    void GetDefaultSize( int& width, int& height ) const noexcept;

private:

    void Update(DX::StepTimer const& timer);
    void Render();

    void Clear();

    void CreateDeviceDependentResources();
    void CreateWindowSizeDependentResources();

    // Device resources.
    std::unique_ptr<DX::DeviceResources>    m_deviceResources;

    // Rendering loop timer.
    DX::StepTimer                           m_timer;

    // Sample objects
    Microsoft::WRL::ComPtr<ID3D11InputLayout>       m_inputLayout;
    Microsoft::WRL::ComPtr<ID3D11Buffer>            m_vertexBuffer;
    Microsoft::WRL::ComPtr<ID3D11Buffer>            m_indexBuffer;
    Microsoft::WRL::ComPtr<ID3D11Buffer>            m_constantBuffer;
    Microsoft::WRL::ComPtr<ID3D11Buffer>            m_lightingData;
    Microsoft::WRL::ComPtr<ID3D11VertexShader>      m_vertexShader;
    Microsoft::WRL::ComPtr<ID3D11PixelShader>       m_pixelShader;

    // These computed values will be loaded into a ConstantBuffer
    // during Render
    DirectX::XMFLOAT4X4                        m_worldMatrix;
    DirectX::XMFLOAT4X4                        m_projectionMatrix;
    SceneParams  m_gSceneParams;
    LightingData    m_gLightingData;

    // Input devices
    std::unique_ptr<DirectX::GamePad>       m_gamePad;
    std::unique_ptr<DirectX::Keyboard>      m_keyboard;
    std::unique_ptr<DirectX::Mouse>         m_mouse;

    DirectX::GamePad::ButtonStateTracker    m_gamePadButtons;
    DirectX::Keyboard::KeyboardStateTracker m_keyboardButtons;
    DirectX::Mouse::ButtonStateTracker      m_mouseButtons;

    // Camera
    std::unique_ptr<Camera>     m_camera;
    std::unique_ptr<Model>      m_model;
};
