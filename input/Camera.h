#pragma once

#include <Keyboard.h>
#include <Mouse.h>
#include <GamePad.h>
#include <DirectXMath.h>

class Camera
{
public:
	Camera();
	void Update(float delta, const DirectX::Mouse::State& mouse);
	DirectX::XMMATRIX GetView() const;

private:
	float m_yaw; // rotates along y axis
	float m_pitch; // rotates along x axis
	float m_mouseX;
	float m_mouseY;
	DirectX::XMFLOAT4 m_cameraPos;
	DirectX::XMFLOAT4 m_right;
	DirectX::XMFLOAT4 m_at;
	DirectX::XMFLOAT4 m_up;
	const DirectX::XMVECTORF32 m_worldUp;
	float m_sensivity;

	void UpdateEulerAngles(const float delta, const DirectX::Mouse::State& mouse);

	static constexpr float DEFAULT_SPEED = 3.0f;
	static constexpr float DEFAULT_SENSIVITY = 0.1f;
};

