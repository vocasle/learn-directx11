#include "pch.h"
#include "Camera.h"

using namespace DirectX;

Camera::Camera(): m_yaw(0.0f), m_pitch(0.0f), m_cameraPos(0.0f, 0.0f, 10.0f, 0.0f)
{
}

static XMFLOAT4 UP(0.0f, 1.0f, 0.0f, 0.0f);
static XMFLOAT4 AT(0.0f, 0.0f, -1.0f, 0.0f);
static XMFLOAT4 RIGHT(1.0f, 0.0f, 0.0f, 0.0f);

#include <string>

void Camera::Update(float delta)
{
    //const auto mouse = Mouse::Get().GetState();
    //m_pitch -= mouse.y;
    //m_yaw -= mouse.x;

    //static float elapsedTime = 0.0f;
    //elapsedTime += delta;

    //if (elapsedTime > 1.0f)
    //{
    //    char buff[512];
    //    sprintf_s(buff, "yaw=%f, pitch=%f\n", m_yaw, m_pitch);
    //    sprintf_s(buff, "mouse.y=%d, mouse.x=%d\n", mouse.y, mouse.x);
    //    OutputDebugStringA(buff);
    //    elapsedTime = 0.0f;
    //}

    // limit pitch to straight up or straight down
    //constexpr float limit = XM_PIDIV2 - 0.01f;
    //m_pitch = std::max(-limit, m_pitch);
    //m_pitch = std::min(+limit, m_pitch);

    //// keep longitude in sane range by wrapping
    //if (m_yaw > XM_PI)
    //{
    //    m_yaw -= XM_2PI;
    //}
    //else if (m_yaw < -XM_PI)
    //{
    //    m_yaw += XM_2PI;
    //}

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
    const XMVECTOR c_up = XMLoadFloat4(&UP);
    const XMVECTOR c_at = XMLoadFloat4(&AT);
    return XMMatrixLookAtLH(cameraPos, cameraPos + c_at, c_up);
}
