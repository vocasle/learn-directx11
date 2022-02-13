//
// Game.cpp
//

// TODO: Add support to draw axii by pressing some special key.
// TODO: Add support to draw in wireframe/shaded modes
// TODO: Cleanup vertex buffer, index buffer creation code

#include "pch.h"
#include "Game.h"

#include "ATGColors.h"
#include "ReadData.h"
#include "DDSTextureLoader.h"
#include "stb_image.h"

extern void ExitGame() noexcept;

using namespace DirectX;

using Microsoft::WRL::ComPtr;

namespace
{
    struct Plane
    {
        std::vector<Vertex> Vertices;
        std::vector<Face> Faces;
    };

    Plane CreatePlane(XMFLOAT3 o, XMFLOAT3 v, XMFLOAT3 n, float w, float h)
    {
        XMVECTOR or = XMLoadFloat3(&o);
        XMVECTOR vec = XMLoadFloat3(&v);
        XMVECTOR norm = XMLoadFloat3(&n);
        XMVECTOR m = XMVector3Cross(norm, vec);
        XMVECTOR e = XMVector3Cross(norm, m);
        XMVECTOR p1 = or + XMVectorScale(m, h);
        XMVECTOR p3 = or + XMVectorScale(e, w);
        XMVECTOR p2 = (p1 - or) + (p3 - or);

        Plane p;
        Vertex ver;
        // bottom left
        XMStoreFloat3(&ver.position, or);
        XMStoreFloat3(&ver.normal, norm);
        ver.tex = XMFLOAT2(0.0f, 1.0f);
        p.Vertices.push_back(ver);

        // top left
        XMStoreFloat3(&ver.position, p1);
        ver.tex = XMFLOAT2(0.0f, 0.0f);
        p.Vertices.push_back(ver);

        // bottom right
        XMStoreFloat3(&ver.position, p3);
        ver.tex = XMFLOAT2(1.0f, 1.0f);
        p.Vertices.push_back(ver);

        // top right
        XMStoreFloat3(&ver.position, p2);
        ver.tex = XMFLOAT2(1.0f, 0.0f);
        p.Vertices.push_back(ver);

        p.Faces.push_back({ 0, 1, 2 });
        p.Faces.push_back({ 2, 1, 3 });
        return p;
    }
}

Game::Game() noexcept(false)
{
    m_deviceResources = std::make_unique<DX::DeviceResources>(DXGI_FORMAT_B8G8R8A8_UNORM_SRGB);
    m_deviceResources->RegisterDeviceNotify(this);
}

// Initialize the Direct3D resources required to run.
void Game::Initialize(HWND window, int width, int height)
{
    // Initialize input devices
    m_gamePad = std::make_unique<GamePad>();
    m_keyboard = std::make_unique<Keyboard>();
    m_mouse = std::make_unique<Mouse>();
    m_deviceResources->SetWindow(window, width, height);
    m_mouse->SetWindow(window);
    m_mouse->SetMode(Mouse::MODE_RELATIVE);
    // Initialize camera
    m_camera = std::make_unique<Camera>();
    m_model = Model::LoadModel("../assets/cube_text.obj");

    m_deviceResources->CreateDeviceResources();
    CreateDeviceDependentResources();

    m_deviceResources->CreateWindowSizeDependentResources();
    CreateWindowSizeDependentResources();

    // Change the timer settings if you want something other than the default variable timestep mode.
    // e.g. for 60 FPS fixed timestep update logic, call:
    m_timer.SetFixedTimeStep(true);
    m_timer.SetTargetElapsedSeconds(1.0 / 60);
}

#pragma region Frame Update
// Executes the basic game loop.
void Game::Tick()
{
    m_timer.Tick([&]()
    {
        Update(m_timer);
    });

    Render();
}

static float x = 0.0f;
static float z = 0.0f;

// Updates the world.
void Game::Update(DX::StepTimer const& timer)
{
    float elapsedTime = float(timer.GetElapsedSeconds());

    // Add your game logic here.
    elapsedTime;

    x += elapsedTime;
    z += elapsedTime;

    // Handle controller input for exit
    const auto pad = m_gamePad->GetState(0);
    if (pad.IsConnected())
    {
        m_gamePadButtons.Update(pad);

        if (pad.IsViewPressed())
        {
            ExitGame();
        }
    }
    else
    {
        m_gamePadButtons.Reset();
    }

    auto kb = m_keyboard->GetState();
    m_keyboardButtons.Update(kb);

    if (kb.Escape)
    {
        ExitGame();
    }

    auto mouse = m_mouse->GetState();
    m_mouseButtons.Update(mouse);

    // Update camera movement
    m_camera->Update(elapsedTime, mouse);

    // Update per frame cbuffers
    {
        XMVECTOR eyePos = XMLoadFloat4(&m_camera->GetPos());
        m_gLightingData.eyePos = eyePos;
        static constexpr float r = 3.0f;
        m_gLightingData.pointLight.Position = XMFLOAT3(r * sin(x), r, r * cos(x));
        const XMFLOAT4& cameraPos = m_camera->GetPos();
        m_gLightingData.spotLight.Position = XMFLOAT3(cameraPos.x, cameraPos.y, cameraPos.z);
        const XMFLOAT4 cameraDir = m_camera->GetAt();
        m_gLightingData.spotLight.Direction = XMFLOAT3(cameraDir.x, cameraDir.y, cameraDir.z);
    }

    {
        // Set the per-frame constants
        // For shaders compiled with default row-major we need to transpose matrices
        m_gSceneParams.worldMatrix = XMMatrixTranspose(XMLoadFloat4x4(&m_worldMatrix));
        m_gSceneParams.viewMatrix = XMMatrixTranspose(m_camera->GetView());
        m_gSceneParams.projectionMatrix = XMMatrixTranspose(XMLoadFloat4x4(&m_projectionMatrix));
    }
}
#pragma endregion

#pragma region Frame Render
// Draws the scene.
void Game::Render()
{
    // Don't try to render anything before the first Update.
    if (m_timer.GetFrameCount() == 0)
    {
        return;
    }

    Clear();

    m_deviceResources->PIXBeginEvent(L"Render");
    const auto context = m_deviceResources->GetD3DDeviceContext();

    // Add your rendering code here.

    // Set the vertex buffer
    constexpr UINT strides = sizeof(Vertex);
    constexpr UINT offsets = 0;
    context->IASetVertexBuffers(0, 1, m_vertexBuffer.GetAddressOf(), &strides, &offsets);
    // Set the index buffer
    context->IASetIndexBuffer(m_indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
    // Set input assembler state
    context->IASetInputLayout(m_inputLayout.Get());
    // Set the primitive topology
    context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    {
        D3D11_MAPPED_SUBRESOURCE mapped;
        DX::ThrowIfFailed(context->Map(m_constantBuffer.Get(), 0,
            D3D11_MAP_WRITE_DISCARD, 0, &mapped));
        memcpy(mapped.pData, &m_gSceneParams, sizeof(SceneParams));
        context->Unmap(m_constantBuffer.Get(), 0);
    }

    // Set lighting data cbuffer
    {
        D3D11_MAPPED_SUBRESOURCE mapped;
        DX::ThrowIfFailed(context->Map(m_lightingData.Get(), 0,
            D3D11_MAP_WRITE_DISCARD, 0, &mapped));
        memcpy(mapped.pData, &m_gLightingData, sizeof(LightingData));
        context->Unmap(m_lightingData.Get(), 0);
    }

    // Vertex shader needs view and projection matrices to perform vertex transform
    context->VSSetConstantBuffers(0, 1, m_constantBuffer.GetAddressOf());
    // Pixel shader needs lighting data
    context->PSSetConstantBuffers(0, 1, m_lightingData.GetAddressOf());
    // Set vertex shader
    context->VSSetShader(m_vertexShader.Get(), nullptr, 0);
    // Set pixel shader
    context->PSSetShader(m_pixelShader.Get(), nullptr, 0);

    // Set texture and sampler.
    const auto sampler = m_model->GetTextureSampler();
    context->PSSetSamplers(0, 1, &sampler);

    const auto texture = m_model->GetTextureView();
    context->PSSetShaderResources(0, 1, &texture);

    // Draw indexed
    context->DrawIndexed(m_model->GetFaces().size() * 3, 0, 0);

    m_deviceResources->PIXEndEvent();

    // Show the new frame.
    m_deviceResources->Present();
}

// Helper method to clear the back buffers.
void Game::Clear()
{
    m_deviceResources->PIXBeginEvent(L"Clear");

    // Clear the views.
    auto context = m_deviceResources->GetD3DDeviceContext();
    auto renderTarget = m_deviceResources->GetRenderTargetView();
    auto depthStencil = m_deviceResources->GetDepthStencilView();

    context->ClearRenderTargetView(renderTarget, Colors::CornflowerBlue);
    context->ClearDepthStencilView(depthStencil, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
    context->OMSetRenderTargets(1, &renderTarget, depthStencil);

    // Set the viewport.
    auto viewport = m_deviceResources->GetScreenViewport();
    context->RSSetViewports(1, &viewport);

    m_deviceResources->PIXEndEvent();
}
#pragma endregion

#pragma region Message Handlers
// Message handlers
void Game::OnActivated()
{
    // Game is becoming active window.
}

void Game::OnDeactivated()
{
    // Game is becoming background window.
}

void Game::OnSuspending()
{
    // Game is being power-suspended (or minimized).
}

void Game::OnResuming()
{
    m_timer.ResetElapsedTime();

    m_gamePadButtons.Reset();
    m_keyboardButtons.Reset();
    m_mouseButtons.Reset();
}

void Game::OnWindowMoved()
{
    auto r = m_deviceResources->GetOutputSize();
    m_deviceResources->WindowSizeChanged(r.right, r.bottom);
}

void Game::OnWindowSizeChanged(int width, int height)
{
    if (!m_deviceResources->WindowSizeChanged(width, height))
        return;

    CreateWindowSizeDependentResources();

    // TODO: Game window is being resized.
}

// Properties
void Game::GetDefaultSize(int& width, int& height) const noexcept
{
    // TODO: Change to desired default window size (note minimum size is 320x200).
    width = 1280;
    height = 720;
}
#pragma endregion

#pragma region Direct3D Resources
// These are the resources that depend on the device.
void Game::CreateDeviceDependentResources()
{
    auto device = m_deviceResources->GetD3DDevice();

    // Initialize device dependent objects here (independent of window size).
    // Load and create shaders
    {
        auto vertexShaderBlob = DX::ReadData(L"VertexShader.cso");

        DX::ThrowIfFailed(
            device->CreateVertexShader(vertexShaderBlob.data(), vertexShaderBlob.size(),
                nullptr, m_vertexShader.ReleaseAndGetAddressOf()));

        auto pixelShaderBlob = DX::ReadData(L"PixelShader.cso");

        DX::ThrowIfFailed(
            device->CreatePixelShader(pixelShaderBlob.data(), pixelShaderBlob.size(),
                nullptr, m_pixelShader.ReleaseAndGetAddressOf()));

        // Create input layout
        static constexpr D3D11_INPUT_ELEMENT_DESC s_inputElementDesc[3] = {
            {"SV_Position", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
            {"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
            {"TEXTCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0}
        };

        DX::ThrowIfFailed(
            device->CreateInputLayout(s_inputElementDesc, _countof(s_inputElementDesc),
                vertexShaderBlob.data(), vertexShaderBlob.size(),
                m_inputLayout.ReleaseAndGetAddressOf()));
    }

    // Create vertex buffer
    {
        std::vector<Vertex> vertices;
        vertices.reserve(m_model->GetPositions().size());
        for (unsigned int i = 0; i < m_model->GetPositions().size(); ++i)
        {
            const Position& p = m_model->GetPositions()[i];
            const Normal& n = m_model->GetNormals()[i];
            if (m_model->GetTextCoords().empty())
            {
                vertices.push_back({
                    XMFLOAT3(p.X, p.Y, p.Z),
                    XMFLOAT3(n.X, n.Y, n.Z),
                    XMFLOAT2(0.0f, 0.0f)
                    });
            }
            else
            {
                const TextCoord& tc = m_model->GetTextCoords()[i];
                vertices.push_back({
                    XMFLOAT3(p.X, p.Y, p.Z),
                    XMFLOAT3(n.X, n.Y, n.Z),
                    XMFLOAT2(tc.X, tc.Y)
                    });
            }
        }

        D3D11_SUBRESOURCE_DATA initialData = {};
        //initialData.pSysMem = s_vertexData;
        initialData.pSysMem = &vertices[0];

        D3D11_BUFFER_DESC bufferDesc = {};
        //bufferDesc.ByteWidth = sizeof(Vertex) * s_numVertices;
        bufferDesc.ByteWidth = sizeof(Vertex) * vertices.size();
        bufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
        bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        bufferDesc.StructureByteStride = sizeof(Vertex);

        DX::ThrowIfFailed(
            device->CreateBuffer(&bufferDesc, &initialData,
                m_vertexBuffer.ReleaseAndGetAddressOf()));
    }

    // Create index buffer
    {
        std::vector<unsigned int> indices;
        const auto& faces = m_model->GetFaces();
        indices.reserve(faces.size());
        // there is 3 indices per face
        for (unsigned int i = 0; i < faces.size(); ++i)
        {
            indices.push_back(faces[i].X);
            indices.push_back(faces[i].Y);
            indices.push_back(faces[i].Z);
        }

        D3D11_BUFFER_DESC bufferDesc = {};
        bufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
        //bufferDesc.ByteWidth = sizeof(unsigned int) * s_numIndices;
        bufferDesc.ByteWidth = sizeof(unsigned int) * indices.size();
        bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
        bufferDesc.StructureByteStride = 0;

        D3D11_SUBRESOURCE_DATA initialData = {};
        //initialData.pSysMem = s_indexData;
        initialData.pSysMem = &indices[0];

        DX::ThrowIfFailed(
            device->CreateBuffer(&bufferDesc, &initialData,
                m_indexBuffer.ReleaseAndGetAddressOf()));
    }

    // Create textures for model
    // TODO: use loop to load texture for array of models
    {
        m_model->LoadTexture(device);
    }

    // Create the constant buffer
    {
        const CD3D11_BUFFER_DESC bufferDesc(sizeof(SceneParams), D3D11_BIND_CONSTANT_BUFFER,
            D3D11_USAGE_DYNAMIC, D3D11_CPU_ACCESS_WRITE);
        DX::ThrowIfFailed(device->CreateBuffer(&bufferDesc, nullptr,
            m_constantBuffer.GetAddressOf()));
    }

    // Create lighting data cbuffer
    {
        const CD3D11_BUFFER_DESC bufferDesc(sizeof(LightingData),
            D3D11_BIND_CONSTANT_BUFFER,
            D3D11_USAGE_DYNAMIC, D3D11_CPU_ACCESS_WRITE);
        DX::ThrowIfFailed(device->CreateBuffer(&bufferDesc, nullptr,
            m_lightingData.GetAddressOf()));
    }

    // Create lighting data cbuffer source
    {
        m_gLightingData.material.Ambient = XMFLOAT4(0.48f, 0.77f, 0.46f, 1.0f);
        m_gLightingData.material.Diffuse = XMFLOAT4(0.48f, 0.77f, 0.46f, 1.0f);
        m_gLightingData.material.Specular = XMFLOAT4(0.2f, 0.2f, 0.2f, 16.0f);
        m_gLightingData.dirLight.Ambient = XMFLOAT4(0.2f, 0.0f, 0.0f, 1.0f);
        m_gLightingData.dirLight.Diffuse = XMFLOAT4(0.5f, 0.0f, 0.0f, 1.0f);
        m_gLightingData.dirLight.Specular = XMFLOAT4(0.5f, 0.0f, 0.0f, 1.0f);
        m_gLightingData.dirLight.Direction = XMFLOAT3(0.57735f, -0.57735f, 0.57735f);
        m_gLightingData.pointLight.Ambient = XMFLOAT4(0.0f, 0.3f, 0.0f, 1.0f);
        m_gLightingData.pointLight.Diffuse = XMFLOAT4(0.0f, 0.7f, 0.0f, 1.0f);
        m_gLightingData.pointLight.Specular = XMFLOAT4(0.0f, 0.7f, 0.0f, 1.0f);
        m_gLightingData.pointLight.Att = XMFLOAT3(0.0f, 0.1f, 0.0f);
        m_gLightingData.pointLight.Range = 25.0f;
        m_gLightingData.spotLight.Ambient = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
        m_gLightingData.spotLight.Diffuse = XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f);
        m_gLightingData.spotLight.Specular = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
        m_gLightingData.spotLight.Att = XMFLOAT3(1.0f, 0.0f, 0.0f);
        m_gLightingData.spotLight.Spot = 96.0f;
        m_gLightingData.spotLight.Range = 10000.0f;
    }

    // Initialize the world matrix
    XMMATRIX world = XMMatrixIdentity();
    //XMFLOAT4 translate(0.0f, 0.0f, -10.0f, 0.0f);
    /*XMMatrixTranslationFromVector(XMLoadFloat4(&translate));*/
    XMStoreFloat4x4(&m_worldMatrix, XMMatrixIdentity());
    //XMStoreFloat4x4(&m_worldMatrix, XMMatrixTranslationFromVector(XMLoadFloat4(&translate)));
}

// Allocate all memory resources that change on a window SizeChanged event.
void Game::CreateWindowSizeDependentResources()
{
    // Initialize windows-size dependent objects here.
    const RECT size = m_deviceResources->GetOutputSize();
    const XMMATRIX projection = XMMatrixPerspectiveFovLH(XM_PIDIV4,
        static_cast<float>(size.right) / static_cast<float>(size.bottom),
        0.0001f, 1000.0f);
    XMStoreFloat4x4(&m_projectionMatrix, projection);
}

void Game::OnDeviceLost()
{
    // Add Direct3D resource cleanup here.
    m_inputLayout.Reset();
    m_vertexBuffer.Reset();
    m_indexBuffer.Reset();
    m_vertexShader.Reset();
    m_pixelShader.Reset();
    m_constantBuffer.Reset();
    m_lightingData.Reset();
}

void Game::OnDeviceRestored()
{
    CreateDeviceDependentResources();

    CreateWindowSizeDependentResources();
}
#pragma endregion
