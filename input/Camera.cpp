#include "pch.h"
#include "Camera.h"

#include <string>

using namespace DirectX;

Camera::Camera(): m_yaw(0.0f), m_pitch(0.0f), m_mouseX(0.0f), m_mouseY(0.0f), m_cameraPos(0.0f, 0.0f, 10.0f, 0.0f),
    m_worldUp(0.0f, 1.0f, 0.0f, 0.0f),
    m_right(0.0f, 0.0f, 0.0f, 0.0f),
    m_at(0.0f, 0.0f, 0.0f ,0.0f)
{
    const XMVECTOR cameraPos = XMLoadFloat4(&m_cameraPos);
    const XMVECTOR at = XMVector4Normalize(-cameraPos);
    const XMVECTOR worldUp = XMLoadFloat4(&m_worldUp);
    XMStoreFloat4(&m_at, at);
    const XMVECTOR right = XMVector4Normalize(XMVector3Cross(at, worldUp));
    XMStoreFloat4(&m_right, right);
}

void Camera::Update(float delta)
{
    // TODO: Clean up the code
    const auto mouse = Mouse::Get().GetState();
    static float elapsedTime = 0.0f;
    elapsedTime += delta;
    //if (m_mouseX != mouse.x || m_mouseY != mouse.y)
    {
        //const float offsetX = m_mouseX - mouse.x;
        //const float offsetY = m_mouseY - mouse.y;
        m_mouseX = mouse.x;
        m_mouseY = mouse.y;
        constexpr float sensivity = 0.001f;
        //m_pitch += delta * static_cast<float>(offsetY)  * sensivity;
        //m_yaw += delta * static_cast<float>(offsetX) * sensivity;
        m_pitch -= delta * static_cast<float>(mouse.y) * sensivity;
        m_yaw -= delta * static_cast<float>(mouse.x) * sensivity;


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


        const float r = cosf(m_pitch);
        m_at.x = r * sinf(m_yaw);
        m_at.y = std::sin(m_pitch);
        m_at.z = r * cosf(m_yaw);
        XMVECTOR at = XMLoadFloat4(&m_at);
        at = XMVector4Normalize(at);
        XMStoreFloat4(&m_at, at);
        XMVECTOR right = XMLoadFloat4(&m_right);
        const XMVECTOR worldUp = XMLoadFloat4(&m_worldUp);
        right = XMVector4Normalize(XMVector3Cross(at, worldUp));
        XMStoreFloat4(&m_right, right);
    }

    if (elapsedTime > 1.0f)
    {
        char buff[512];
        sprintf_s(buff, "yaw=%f, pitch=%f\n", m_yaw, m_pitch);
        OutputDebugStringA(buff);
        sprintf_s(buff, "mouse.y=%d, mouse.x=%d\n", mouse.y, mouse.x);
        OutputDebugStringA(buff);
        elapsedTime = 0.0f;
    }



    const float deltaSpeed = delta * DEFAULT_SPEED;
    const XMVECTOR at = XMLoadFloat4(&m_at);
    XMVECTOR cameraPos = XMLoadFloat4(&m_cameraPos);
    const XMVECTOR right = XMLoadFloat4(&m_right);
    
    const auto kb = Keyboard::Get().GetState();
    if (kb.Left || kb.A)
    {
        cameraPos -= right * deltaSpeed;
    }
    if (kb.Right || kb.D)
    {
        cameraPos += right * deltaSpeed;
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
	// todo: calculate new viewMatrix based on new yaw and pitch values
    XMVECTOR cameraPos = XMLoadFloat4(&m_cameraPos);
    const XMVECTOR worldUp = XMLoadFloat4(&m_worldUp);
    const XMVECTOR at = XMLoadFloat4(&m_at);
    const XMVECTOR right = XMLoadFloat4(&m_right);
    const XMVECTOR up = XMVector4Normalize(XMVector3Cross(at, right));
    return XMMatrixLookAtLH(cameraPos, cameraPos + at, up);
}
