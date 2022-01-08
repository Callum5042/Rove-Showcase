#include "InfoComponent.h"
#include "Application.h"

Rove::InfoComponent::InfoComponent(Application* application)
{
}

void Rove::InfoComponent::OnRender()
{
	/*ImGui::SetNextWindowPos(ImVec2(20, 20), ImGuiCond_Once);
	ImGui::SetNextWindowSize(ImVec2(200, 60), ImGuiCond_Once);*/

	ImGui::Begin("InfoWindow", nullptr);
	ImGui::Text("Rove Showroom - FPS: %d", 10);

	ImGui::Separator();

	ImGui::End();
}
