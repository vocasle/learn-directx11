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
	const DirectX::XMFLOAT4& GetPos() const;

private:
	float m_yaw; // rotates along y axis
	float m_pitch; // rotates along x axis
	int m_mouseX;
	int m_mouseY;
	DirectX::XMFLOAT4 m_cameraPos;
	DirectX::XMFLOAT4 m_left;
	DirectX::XMFLOAT4 m_at;
	DirectX::XMFLOAT4 m_up;
	const DirectX::XMFLOAT4 m_worldUp;
	float m_sensivity;

	void UpdateEulerAngles(const float delta, const DirectX::Mouse::State& mouse);
	void UpdateCameraVectors();

	static constexpr float DEFAULT_SPEED = 3.0f;
	static constexpr float DEFAULT_SENSIVITY = 0.1f;
};

