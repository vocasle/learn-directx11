#include "pch.h"
#include "Camera.h"

using namespace DirectX;

Camera::Camera(): m_yaw(0.0f), m_pitch(0.0f), m_mouseX(0.0f), m_mouseY(0.0f), m_cameraPos(0.0f, 0.0f, 10.0f, 0.0f)
{
    const auto mouse = Mouse::Get().GetState();
    m_mouseX = 1280.0f / 2.0f;
    m_mouseY = 720.0f / 2.0f;
}

static const XMFLOAT4 WORLD_UP(0.0f, 1.0f, 0.0f, 0.0f);
static XMFLOAT4 UP(0.0f, 1.0f, 0.0f, 0.0f);
static XMFLOAT4 AT(0.0f, 0.0f, -1.0f, 0.0f);
static XMFLOAT4 RIGHT(1.0f, 0.0f, 0.0f, 0.0f);

#include <string>

void Camera::Update(float delta)
{
    // TODO: Clean up the code
    const auto mouse = Mouse::Get().GetState();
    static float elapsedTime = 0.0f;
    elapsedTime += delta;
    if (m_mouseX != mouse.x || m_mouseY != mouse.y)
    {
        const float offsetX = m_mouseX - mouse.x;
        const float offsetY = m_mouseY - mouse.y;
        m_mouseX = mouse.x;
        m_mouseY = mouse.y;
        constexpr float sensivity = 0.1f;
        m_pitch += delta * static_cast<float>(offsetY)  * sensivity;
        m_yaw += delta * static_cast<float>(offsetX) * sensivity;


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

        AT.x = std::cos(m_yaw);
        AT.y = std::sin(m_pitch);
        AT.z = std::sin(m_yaw);
        XMVECTOR at = XMLoadFloat4(&AT);
        at = XMVector4Normalize(at);
        XMStoreFloat4(&AT, at);
        XMVECTOR right = XMLoadFloat4(&RIGHT);
        const XMVECTOR worldUp = XMLoadFloat4(&WORLD_UP);
        right = XMVector4Normalize(XMVector3Cross(at, worldUp));
        XMStoreFloat4(&RIGHT, right);
        const XMVECTOR up = XMVector4Normalize(XMVector3Cross(at, right));
        XMStoreFloat4(&UP, up);
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
    const XMVECTOR at = XMLoadFloat4(&AT);
    XMVECTOR cameraPos = XMLoadFloat4(&m_cameraPos);
    const XMVECTOR right = XMLoadFloat4(&RIGHT);
    
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
	// todo: calculate new viewMatrix based on new yaw and pitch values
    XMVECTOR cameraPos = XMLoadFloat4(&m_cameraPos);
    const XMVECTOR c_up = XMLoadFloat4(&WORLD_UP);
    const XMVECTOR c_at = XMLoadFloat4(&AT);
    return XMMatrixLookAtLH(cameraPos, cameraPos + c_at, c_up);
}
