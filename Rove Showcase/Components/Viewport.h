#pragma once

namespace Rove
{
	class Application;

	// Viewport
	class Viewport
	{
	public:
		Viewport(Application* application);
		virtual ~Viewport() = default;

		void OnCreate();

		void Set();

		ID3D11ShaderResourceView* GetViewportTexture() { return m_RenderedTexture.Get(); }

	private:
		Application* m_Application = nullptr;

		ComPtr<ID3D11ShaderResourceView> m_RenderedTexture = nullptr;
		ComPtr<ID3D11Texture2D> m_Texture = nullptr;

		ComPtr<ID3D11RenderTargetView> m_TextureRenderTargetView = nullptr;
		ComPtr<ID3D11DepthStencilView> m_TextureDepthStencilView = nullptr;
	};
}