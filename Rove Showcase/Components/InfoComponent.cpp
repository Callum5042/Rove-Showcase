#include "InfoComponent.h"
#include "Application.h"

Rove::InfoComponent::InfoComponent(Application* application)
{
}

void Rove::InfoComponent::OnRender()
{
	ImGui::SetNextWindowPos(ImVec2(20, 20), ImGuiCond_Once);
	ImGui::SetNextWindowSize(ImVec2(200, 60), ImGuiCond_Once);

	ImGui::Begin("InfoWindow", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar);
	ImGui::Text("Rove Showroom - FPS: %d", 10);

	ImGui::End();
}
