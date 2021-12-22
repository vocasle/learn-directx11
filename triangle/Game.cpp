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
}

Game::Game() noexcept(false)
{
    m_deviceResources = std::make_unique<DX::DeviceResources>(DXGI_FORMAT_B8G8R8A8_UNORM_SRGB);
    m_deviceResources->RegisterDeviceNotify(this);
}

// Initialize the Direct3D resources required to run.
void Game::Initialize(HWND window, int width, int height)
{
    m_deviceResources->SetWindow(window, width, height);

    m_deviceResources->CreateDeviceResources();
    CreateDeviceDependentResources();

    m_deviceResources->CreateWindowSizeDependentResources();
    CreateWindowSizeDependentResources();

    // TODO: Change the timer settings if you want something other than the default variable timestep mode.
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

    // TODO: Add your game logic here.
    elapsedTime;
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
    auto context = m_deviceResources->GetD3DDeviceContext();

    // TODO: Add your rendering code here.
    // Set input assembler state
    context->IASetInputLayout(m_spInputLayout.Get());

    UINT strides = sizeof(Vertex);
    UINT offsets = 0;
    context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    context->IASetVertexBuffers(0, 1, m_spVertexBuffer.GetAddressOf(), &strides, &offsets);

    // Set shaders
    context->VSSetShader(m_spVertexShader.Get(), nullptr, 0);
    context->PSSetShader(m_spPixelShader.Get(), nullptr, 0);

    // Draw triangle
    context->Draw(3, 0);

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
    // TODO: Game is becoming active window.
}

void Game::OnDeactivated()
{
    // TODO: Game is becoming background window.
}

void Game::OnSuspending()
{
    // TODO: Game is being power-suspended (or minimized).
}

void Game::OnResuming()
{
    m_timer.ResetElapsedTime();

    // TODO: Game is being power-resumed (or returning from minimize).
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

    // TODO: Initialize device dependent objects here (independent of window size).
    // Load and create shaders
    auto vertexShaderBlob = DX::ReadData(L"VertexShader.cso");

    DX::ThrowIfFailed(
        device->CreateVertexShader(vertexShaderBlob.data(), vertexShaderBlob.size(),
            nullptr, m_spVertexShader.ReleaseAndGetAddressOf()));

    auto pixelShaderBlob = DX::ReadData(L"PixelShader.cso");

    DX::ThrowIfFailed(
        device->CreatePixelShader(pixelShaderBlob.data(), pixelShaderBlob.size(),
            nullptr, m_spPixelShader.ReleaseAndGetAddressOf()));

    // Create input layout
    static constexpr D3D11_INPUT_ELEMENT_DESC s_inputElementDesc[2] = {
        {"SV_Position", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 16, D3D11_INPUT_PER_VERTEX_DATA, 0}
    };

    DX::ThrowIfFailed(
        device->CreateInputLayout(s_inputElementDesc, _countof(s_inputElementDesc),
            vertexShaderBlob.data(), vertexShaderBlob.size(),
            m_spInputLayout.ReleaseAndGetAddressOf()));

    // Create vertex buffer
    static constexpr Vertex s_vertexData[3] =
    {
        { { 0.0f,   0.5f,  0.5f, 1.0f },{ 1.0f, 0.0f, 0.0f, 1.0f } },  // Top / Red
        { { 0.5f,  -0.5f,  0.5f, 1.0f },{ 0.0f, 1.0f, 0.0f, 1.0f } },  // Right / Green
        { { -0.5f, -0.5f,  0.5f, 1.0f },{ 0.0f, 0.0f, 1.0f, 1.0f } }   // Left / Blue
    };

    D3D11_SUBRESOURCE_DATA initialData = {};
    initialData.pSysMem = s_vertexData;

    D3D11_BUFFER_DESC bufferDesc = {};
    bufferDesc.ByteWidth = sizeof(s_vertexData);
    bufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
    bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bufferDesc.StructureByteStride = sizeof(Vertex);

    DX::ThrowIfFailed(
        device->CreateBuffer(&bufferDesc, &initialData,
            m_spVertexBuffer.ReleaseAndGetAddressOf()));
}

// Allocate all memory resources that change on a window SizeChanged event.
void Game::CreateWindowSizeDependentResources()
{
    // TODO: Initialize windows-size dependent objects here.
}

void Game::OnDeviceLost()
{
    // TODO: Add Direct3D resource cleanup here.
}

void Game::OnDeviceRestored()
{
    CreateDeviceDependentResources();

    CreateWindowSizeDependentResources();
}
#pragma endregion
