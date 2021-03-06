#pragma once

#include "Pch.h"

namespace Rove
{
	// Perspective orbital camera
	class Camera
	{
	public:
		Camera(int width, int height);
		virtual ~Camera() = default;

		// Recalculates the view based on the pitch and yaw
		void Rotate(float pitch_radians, float yaw_radians);

		// Update aspect ratio
		void UpdateAspectRatio(int width, int height);

		// Updates field of view relative
		void UpdateFov(float fov);

		// Sets field of view
		void SetFov(float fov_degrees);

		// Get projection matrix
		constexpr DirectX::XMMATRIX GetProjection() { return m_Projection; }

		// Get view matrix
		constexpr DirectX::XMMATRIX GetView() { return m_View; }

		// Get field of view in degrees
		constexpr float GetFieldOfView() { return m_FieldOfViewDegrees; }

		// Get camera position
		constexpr DirectX::XMFLOAT3 GetPosition() { return m_Position; }

	private:
		// Camera position
		DirectX::XMFLOAT3 m_Position;

		// Projection matrix
		DirectX::XMMATRIX m_Projection;

		// View matrix
		DirectX::XMMATRIX m_View;

		// Camera pitch in radians
		float m_PitchRadians = 0.0f;

		// Camera yaw in radians
		float m_YawRadians = 0.0f;

		// Camera field of view in degrees
		float m_FieldOfViewDegrees = 50.0f;

		// Float aspect ratio
		float m_AspectRatio = 0.0f;

		// Recalculates the projection based on the new window size
		void CalculateProjection();
	};
}