#pragma once

namespace Rove
{
	class Application;

	class InfoComponent
	{
	public:
		InfoComponent(Application* application);
		virtual ~InfoComponent() = default;

		void OnRender();

	private:
		Application* m_Application = nullptr;
	};
}