#include "pch.h"
#include "Camera.h"

#include <string>
#include <sstream>

using namespace DirectX;

#include <string>

void DebugPrintf(char* fmt...)
{
#if 0
    char out[512];
    va_list args;
    va_start(args, fmt);
    vsnprintf_s(out, 512, fmt, args);
    va_end(args);
    OutputDebugStringA(out);
#endif
}

std::string XMFloat4ToString(XMFLOAT4 v)
{
    char out[512];
    sprintf_s(out, "(%f %f %f %f)", v.x, v.y, v.z, v.w);
    return out;
}

std::string XMVectorToString(XMVECTOR v)
{
    XMFLOAT4 fv;
    XMStoreFloat4(&fv, v);
    return XMFloat4ToString(fv);
}


Camera::Camera() : 
    m_yaw(XMConvertToRadians(90.0f)), 
    m_pitch(XMConvertToRadians(0.0f)), 
    m_mouseX(0), 
    m_mouseY(0), 
    m_cameraPos(0.0f, 1.0f, 10.0f, 0.0f),
    m_left(1.0f, 0.0f, 0.0f, 0.0f),
    m_at(0.0f, 0.0f, 0.0f, 0.0f),
    m_up(0.0f, 1.0f, 0.0f, 0.0f),
    m_worldUp(0.0f, 1.0f, 0.0f, 0.0f),
    m_sensivity(DEFAULT_SENSIVITY)
{
    const XMVECTOR cameraPos = XMLoadFloat4(&m_cameraPos);
    XMStoreFloat4(&m_at, XMVector4Normalize(-cameraPos));
    UpdateCameraVectors();
}

void Camera::Update(float delta, const Mouse::State &mouse)
{
    static float elapsedTime = 0.0f;
    elapsedTime += delta;

    if (m_mouseX != mouse.x || m_mouseY != mouse.y)
    {
        UpdateEulerAngles(delta, mouse);
        UpdateCameraVectors();
    }

    const float deltaSpeed = delta * DEFAULT_SPEED;
    const XMVECTOR at = XMLoadFloat4(&m_at);
    XMVECTOR cameraPos = XMLoadFloat4(&m_cameraPos);
    const XMVECTOR left = XMLoadFloat4(&m_left);
    
    const auto kb = Keyboard::Get().GetState();
    if (kb.Left || kb.A)
    {
        cameraPos += left * deltaSpeed;
    }
    if (kb.Right || kb.D)
    {
        cameraPos -= left * deltaSpeed;
    }
    if (kb.Up || kb.W)
    {
        cameraPos += at * deltaSpeed;
    }
    if (kb.Down || kb.S)
    {
        cameraPos -= at * deltaSpeed;
    }
    XMStoreFloat4(&m_cameraPos, cameraPos);
}

XMMATRIX Camera::GetView() const
{
    const XMVECTOR cameraPos = XMLoadFloat4(&m_cameraPos);
    const XMVECTOR c_up = XMLoadFloat4(&m_up);
    const XMVECTOR c_at = XMLoadFloat4(&m_at);
    DebugPrintf("cameraPos=%s, up=%s, at=%s\n",
        XMFloat4ToString(m_cameraPos).c_str(),
        XMFloat4ToString(m_up).c_str(),
        XMFloat4ToString(m_at).c_str());
    
    return XMMatrixLookAtLH(cameraPos, cameraPos + c_at, c_up);
}

const DirectX::XMFLOAT4& Camera::GetPos() const
{
    return m_cameraPos;
}

const DirectX::XMFLOAT4& Camera::GetAt() const
{
    return m_at;
}

void Camera::UpdateEulerAngles(const float delta, const Mouse::State& mouse)
{
    DebugPrintf("Before: at=%s, pitch=%f, yaw=%f\n",
        XMFloat4ToString(m_at).c_str(),
        XMConvertToDegrees(m_pitch),
        XMConvertToDegrees(m_yaw));

    m_pitch -= delta * static_cast<float>(mouse.y) * m_sensivity; // reversed, because mouse y is from top to bottom, but World is bottom to top
    m_yaw += delta * static_cast<float>(mouse.x) * m_sensivity;
    m_mouseX = mouse.x;
    m_mouseY = mouse.y;

    //limit pitch to straight up or straight down
    constexpr float limit = XM_PIDIV2 - 0.01f;
    m_pitch = std::max(-limit, m_pitch);
    m_pitch = std::min(+limit, m_pitch);

    // keep longitude in sane range by wrapping
    if (m_yaw > XM_PI)
    {
        m_yaw -= XM_2PI;
    }
    else if (m_yaw < -XM_PI)
    {
        m_yaw += XM_2PI;
    }
}

void Camera::UpdateCameraVectors()
{
    DebugPrintf("Before: at=%s, left=%s, up=%s\n",
        XMFloat4ToString(m_at).c_str(),
        XMFloat4ToString(m_left).c_str(),
        XMFloat4ToString(m_up).c_str());

    const float h = std::cos(m_pitch);
    m_at.x = h * std::cos(m_yaw);
    m_at.y = std::sin(m_pitch);
    m_at.z = -h * std::sin(m_yaw); // z of the camera is facing oposite direction of World's Z axis

    XMVECTOR at = XMLoadFloat4(&m_at);
    at = XMVector4Normalize(at);
    XMStoreFloat4(&m_at, at);
    const XMVECTOR worldUp = XMLoadFloat4(&m_worldUp);
    const XMVECTOR left = XMVector4Normalize(XMVector3Cross(at, worldUp));
    XMStoreFloat4(&m_left, left);
    const XMVECTOR up = XMVector4Normalize(XMVector3Cross(left, at)); // we have left handed frame
    XMStoreFloat4(&m_up, up);

    DebugPrintf("After: at=%s, left=%s, up=%s\n",
        XMFloat4ToString(m_at).c_str(),
        XMFloat4ToString(m_left).c_str(),
        XMFloat4ToString(m_up).c_str());
 }