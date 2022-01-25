//
// Game.cpp
//

#include "pch.h"
#include "Game.h"

#include "ATGColors.h"
#include "ReadData.h"

extern void ExitGame() noexcept;

using namespace DirectX;

using Microsoft::WRL::ComPtr;

namespace
{
    struct Vertex
    {
        XMFLOAT4 position;
        XMFLOAT4 color;
    };

    struct ConstantBuffer
    {
        XMMATRIX worldMatrix;
        XMMATRIX viewMatrix;
        XMMATRIX projectionMatrix;
    };

    static_assert((sizeof(ConstantBuffer) % 16) == 0, "Constant buffer must always be 16-byte aligned");
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

// Updates the world.
void Game::Update(DX::StepTimer const& timer)
{
    float elapsedTime = float(timer.GetElapsedSeconds());

    // Add your game logic here.
    elapsedTime;

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

    // Set the per-frame constants
    ConstantBuffer sceneParams = {};
    // For shaders compiled with default row-major we need to transpose matrices
    sceneParams.worldMatrix = XMMatrixTranspose(XMLoadFloat4x4(&m_worldMatrix));
    sceneParams.viewMatrix = XMMatrixTranspose(m_camera->GetView());
    sceneParams.projectionMatrix = XMMatrixTranspose(XMLoadFloat4x4(&m_projectionMatrix));
    {
        D3D11_MAPPED_SUBRESOURCE mapped;
        DX::ThrowIfFailed(context->Map(m_constantBuffer.Get(), 0,
            D3D11_MAP_WRITE_DISCARD, 0, &mapped));
        memcpy(mapped.pData, &sceneParams, sizeof(ConstantBuffer));
        context->Unmap(m_constantBuffer.Get(), 0);
    }

    // Vertex shader needs view and projection matrices to perform vertex transform
    context->VSSetConstantBuffers(0, 1, m_constantBuffer.GetAddressOf());
    // Set vertex shader
    context->VSSetShader(m_vertexShader.Get(), nullptr, 0);
    // Set pixel shader
    context->PSSetShader(m_pixelShader.Get(), nullptr, 0);

    // Draw the cube indexed
    context->DrawIndexed(36, 0, 0);

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
        static constexpr D3D11_INPUT_ELEMENT_DESC s_inputElementDesc[2] = {
            {"SV_Position", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
            {"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 16, D3D11_INPUT_PER_VERTEX_DATA, 0}
        };

        DX::ThrowIfFailed(
            device->CreateInputLayout(s_inputElementDesc, _countof(s_inputElementDesc),
                vertexShaderBlob.data(), vertexShaderBlob.size(),
                m_inputLayout.ReleaseAndGetAddressOf()));
    }

    // Create vertex buffer
    {
        constexpr XMFLOAT4 White = { 1.0f, 1.0f, 1.0f, 1.0f };
        constexpr XMFLOAT4 Black = { 0.0f, 0.0f, 0.0f, 1.0f };
        constexpr XMFLOAT4 Red = { 1.0f, 0.0f, 0.0f, 1.0f };
        constexpr XMFLOAT4 Green = { 0.0f, 1.0f, 0.0f, 1.0f };
        constexpr XMFLOAT4 Blue = { 0.0f, 0.0f, 1.0f, 1.0f };
        constexpr XMFLOAT4 Yellow = { 1.0f, 1.0f, 0.0f, 1.0f };
        constexpr XMFLOAT4 Cyan = { 0.0f, 1.0f, 1.0f, 1.0f };
        constexpr XMFLOAT4 Magenta = { 1.0f, 0.0f, 1.0f, 1.0f };
        static constexpr XMFLOAT4 color = { 1.0f, 0.0f, 0.0f, 1.0f };
        static constexpr Vertex s_vertexData[8] =
        {
            { XMFLOAT4(-1.0f, -1.0f, -1.0f, 1.0f), White }, // front bottom left
            { XMFLOAT4(-1.0f, +1.0f, -1.0f, 1.0f), Black }, // front top left
            { XMFLOAT4(+1.0f, -1.0f, -1.0f, 1.0f), Green }, // front bottom right
            { XMFLOAT4(+1.0f, +1.0f, -1.0f, 1.0f), Red }, // front top right
            { XMFLOAT4(-1.0f, -1.0f, +1.0f, 1.0f), Blue }, // rear bottom left
            { XMFLOAT4(-1.0f, +1.0f, +1.0f, 1.0f), Yellow }, // rear top left
            { XMFLOAT4(+1.0f, -1.0f, +1.0f, 1.0f), Magenta }, // rear bottom right
            { XMFLOAT4(+1.0f, +1.0f, +1.0f, 1.0f), Cyan }, // rear top right
        };

        D3D11_SUBRESOURCE_DATA initialData = {};
        initialData.pSysMem = s_vertexData;

        D3D11_BUFFER_DESC bufferDesc = {};
        bufferDesc.ByteWidth = sizeof(Vertex) * 8;
        bufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
        bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        bufferDesc.StructureByteStride = sizeof(Vertex);

        DX::ThrowIfFailed(
            device->CreateBuffer(&bufferDesc, &initialData,
                m_vertexBuffer.ReleaseAndGetAddressOf()));
    }

    // Create index buffer
    {
        constexpr unsigned int s_indexData[36] = {
            // front face
            0, 1, 2,
            1, 3, 2,
            // back face
            4, 6, 7,
            4, 7, 5,
            // left face
            0, 4, 5,
            0, 5, 1,
            // right face
			2, 3, 6,
            6, 3, 7,
            // top face
            1, 5, 3,
            3, 5, 7,
            // bottom face
            0, 2, 4,
            2, 6, 4
        };

        D3D11_BUFFER_DESC bufferDesc = {};
        bufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
        bufferDesc.ByteWidth = sizeof(unsigned int) * 36;
        bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
        bufferDesc.StructureByteStride = 0;

        D3D11_SUBRESOURCE_DATA initialData = {};
        initialData.pSysMem = s_indexData;

        DX::ThrowIfFailed(
            device->CreateBuffer(&bufferDesc, &initialData,
                m_indexBuffer.ReleaseAndGetAddressOf()));
    }

    // Create the constant buffer
    {
        const CD3D11_BUFFER_DESC bufferDesc(sizeof(ConstantBuffer), D3D11_BIND_CONSTANT_BUFFER,
            D3D11_USAGE_DYNAMIC, D3D11_CPU_ACCESS_WRITE);
        DX::ThrowIfFailed(device->CreateBuffer(&bufferDesc, nullptr,
            m_constantBuffer.GetAddressOf()));
    }

    // Initialize the world matrix
    XMStoreFloat4x4(&m_worldMatrix, XMMatrixIdentity());
}

// Allocate all memory resources that change on a window SizeChanged event.
void Game::CreateWindowSizeDependentResources()
{
    // Initialize windows-size dependent objects here.
    const RECT size = m_deviceResources->GetOutputSize();
    const XMMATRIX projection = XMMatrixPerspectiveFovLH(XM_PIDIV4,
        static_cast<float>(size.right) / static_cast<float>(size.bottom),
        0.01f, 100.0f);
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
}

void Game::OnDeviceRestored()
{
    CreateDeviceDependentResources();

    CreateWindowSizeDependentResources();
}
#pragma endregion
