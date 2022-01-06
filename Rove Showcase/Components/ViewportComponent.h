#pragma once

namespace Rove
{
	class Application;

	// Viewport
	class ViewportComponent
	{
	public:
		ViewportComponent(Application* application);
		virtual ~ViewportComponent() = default;

		void OnCreate();
		void OnRender();

		void Set();

	private:
		Application* m_Application = nullptr;

		ComPtr<ID3D11ShaderResourceView> m_RenderedTexture = nullptr;
		ComPtr<ID3D11Texture2D> m_Texture = nullptr;

		ComPtr<ID3D11RenderTargetView> m_TextureRenderTargetView = nullptr;
		ComPtr<ID3D11DepthStencilView> m_TextureDepthStencilView = nullptr;
	};
}