#include "pch.h"
#include "Camera.h"

#include <string>
#include <sstream>

using namespace DirectX;

#include <string>

void DebugPrintf(char* fmt...)
{
    char out[512];
    va_list args;
    va_start(args, fmt);
    vsnprintf_s(out, 512, fmt, args);
    va_end(args);
    OutputDebugStringA(out);
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
    m_mouseX(0.0f), 
    m_mouseY(0.0f), 
    m_cameraPos(0.0f, 0.0f, 10.0f, 0.0f),
    m_right(1.0f, 0.0f, 0.0f, 0.0f),
    m_at(0.0f, 0.0f, 0.0f, 0.0f),
    m_up(0.0f, 1.0f, 0.0f, 0.0f),
    m_worldUp{0.0f, 1.0f, 0.0f, 0.0f},
    m_sensivity(DEFAULT_SENSIVITY)
{
    const XMVECTOR cameraPos = XMLoadFloat4(&m_cameraPos);
    XMStoreFloat4(&m_at, XMVector4Normalize(-cameraPos));
}

void Camera::Update(float delta, const Mouse::State &mouse)
{
    static float elapsedTime = 0.0f;
    elapsedTime += delta;
    // TODO: Clean up the code

    if (m_mouseX != mouse.x || m_mouseY != mouse.y)
    {
        UpdateEulerAngles(delta, mouse);
    }

    const float deltaSpeed = delta * DEFAULT_SPEED;
    const XMVECTOR at = XMLoadFloat4(&m_at);
    XMVECTOR cameraPos = XMLoadFloat4(&m_cameraPos);
    const XMVECTOR right = XMLoadFloat4(&m_right);
    
    const auto kb = Keyboard::Get().GetState();
    if (kb.Left || kb.A)
    {
        cameraPos += right * deltaSpeed;
    }
    if (kb.Right || kb.D)
    {
        cameraPos -= right * deltaSpeed;
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

void Camera::UpdateEulerAngles(const float delta, const Mouse::State& mouse)
{
    DebugPrintf("Before: at=%s, pitch=%f, yaw=%f\n",
        XMFloat4ToString(m_at).c_str(),
        XMConvertToDegrees(m_pitch),
        XMConvertToDegrees(m_yaw));

    constexpr float sensivity = 0.1f;
    m_pitch += delta * static_cast<float>(mouse.y) * m_sensivity;
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

    // temporary lock the pitch
    m_pitch = 0.0f;

    const float h = std::cos(m_pitch);
    m_at.x = h * std::cos(m_yaw);
    m_at.y = std::sin(m_pitch);
    m_at.z = -h * std::sin(m_yaw);

    DebugPrintf("Before: right=%s, up=%s\n",
        XMFloat4ToString(m_right).c_str(),
        XMFloat4ToString(m_up).c_str());

    XMVECTOR at = XMLoadFloat4(&m_at);
    at = XMVector4Normalize(at);
    XMStoreFloat4(&m_at, at);
    XMVECTOR right = XMLoadFloat4(&m_right);
    right = XMVector4Normalize(XMVector3Cross(at, m_worldUp));
    XMStoreFloat4(&m_right, right);
    const XMVECTOR up = XMVector4Normalize(XMVector3Cross(at, right));
    XMStoreFloat4(&m_right, up);

    DebugPrintf("After: right=%s, up=%s\n",
        XMFloat4ToString(m_right).c_str(),
        XMFloat4ToString(m_up).c_str());

    DebugPrintf("After: at=%s, pitch=%f, yaw=%f\n", 
        XMFloat4ToString(m_at).c_str(),
        XMConvertToDegrees(m_pitch),
        XMConvertToDegrees(m_yaw));
}