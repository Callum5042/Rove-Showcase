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

		// Texture used for both shader resource and render target
		ComPtr<ID3D11Texture2D> m_SharedTexture = nullptr;
		void CreateSharedTexture(int width, int height);

		// Shader resource view
		ComPtr<ID3D11ShaderResourceView> m_RenderedTexture = nullptr;
		void CreateShaderResourceView();

		// Render target view
		ComPtr<ID3D11RenderTargetView> m_TextureRenderTargetView = nullptr;
		void CreateRenderTargetView();

		// Depth stencil view
		ComPtr<ID3D11DepthStencilView> m_TextureDepthStencilView = nullptr;
		void CreateDepthStencilView(int width, int height);
	};
}