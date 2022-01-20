#pragma once

#include <Keyboard.h>
#include <Mouse.h>
#include <GamePad.h>
#include <DirectXMath.h>

class Camera
{
public:
	Camera();
	void Update(float delta);
	DirectX::XMMATRIX GetView() const;

private:
	float m_yaw; // rotates along y axis
	float m_pitch; // rotates along x axis
	DirectX::XMFLOAT4 m_cameraPos;

	static constexpr float DEFAULT_SPEED = 3.0f;
};

