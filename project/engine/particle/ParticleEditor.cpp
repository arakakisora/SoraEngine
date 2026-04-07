#define NOMINMAX
#include "ParticleEditor.h"
#include "ParticleMnager.h"
#include "ParticleBehaviorFactory.h"
#include <algorithm>
#include <filesystem>
#include "imgui.h"
#include <ChargeBehabiaor.h>

#ifdef USE_IMGUI

namespace fs = std::filesystem;

void ParticleEditor::DrawImguiEditor()
{
	if (!manager_) {
		return;
	}

	auto& groups = manager_->particleGroups;

	ImGui::Begin("Particle Editor");

	if (textureFilePaths_.empty()) {
		LoadTexturesFromDirectory("Resources/ParticleTexture");
	}

	// -----------------------------
	// Effect 一覧 + New Create
	// -----------------------------
	std::vector<std::string> names;
	names.reserve(groups.size() + 1);
	for (const auto& [name, group] : groups) {
		names.push_back(name);
	}
	names.push_back("+ New Create");

	if (currentIndex_ >= (int)names.size()) {
		currentIndex_ = 0;
	}

	std::vector<const char*> items;
	items.reserve(names.size());
	for (const auto& name : names) {
		items.push_back(name.c_str());
	}

	ImGui::Combo("Particle", &currentIndex_, items.data(), (int)items.size());

	const bool isCreateMode = (currentIndex_ == (int)names.size() - 1);

	if (isCreateMode) {
		CreateModeIMGui();
	}
	else {
		const std::string& currentName = names[currentIndex_];

		BasicIMGui(currentName);
		VertexTypeIMGui(currentName);

		ImGui::SeparatorText("Texture");
		TextureSelectIMGui(currentName);

		BehaviorIMGui(currentName);

		if (ImGui::Button("Save To Json")) {
			manager_->SaveToJson("Resources/Data/Particles.json");
		}
	}

	ImGui::End();
}

void ParticleEditor::CreateModeIMGui()
{
	ImGui::SeparatorText("New Particle");

	ImGui::InputText("New Name", newEffectName_, IM_ARRAYSIZE(newEffectName_));

	DrawBehaviorSelector("Behavior", newBehaviorIndex_);
	DrawMeshSelector("Mesh", newMeshIndex_);
	DrawTextureSelector("Texture", newTextureIndex_);

	// Preview Emit
	if (ImGui::Button("Preview Emit")) {
		const std::string previewName = "__preview_effect__";

		const auto& behaviorNames = ParticleBehaviorFactory::GetBehaviorNames();
		if (behaviorNames.empty()) {
			return;
		}

		const std::string& behaviorType = behaviorNames[newBehaviorIndex_];
		auto behavior = ParticleBehaviorFactory::Create(behaviorType);
		if (!behavior) {
			return;
		}

		VerticesType meshType = GetMeshTypeFromIndex(newMeshIndex_);

		std::string texturePath = "Resources/ParticleTexture/default.png";
		if (!textureFilePaths_.empty()) {
			texturePath = (fs::path(textureDirectory_) / textureFilePaths_[newTextureIndex_]).string();
		}

		if (!manager_->particleGroups.contains(previewName)) {
			manager_->CreateParticleGroup(previewName, texturePath, meshType, std::move(behavior));
		}
		else {
			manager_->SetBehavior(previewName, std::move(behavior));
			manager_->SetGroupVerticesType(previewName, meshType);
			manager_->SetGroupTexture(previewName, texturePath);
			manager_->particleGroups[previewName].particles.clear();
		}

		manager_->EmitAtCamera(previewName);
	}

	ImGui::SameLine();

	// Create
	if (ImGui::Button("Create")) {
		std::string effectName = newEffectName_;
		if (effectName.empty()) {
			return;
		}
		if (manager_->particleGroups.contains(effectName)) {
			return;
		}

		const auto& behaviorNames = ParticleBehaviorFactory::GetBehaviorNames();
		if (behaviorNames.empty()) {
			return;
		}

		const std::string& behaviorType = behaviorNames[newBehaviorIndex_];
		auto behavior = ParticleBehaviorFactory::Create(behaviorType);
		if (!behavior) {
			return;
		}

		VerticesType meshType = GetMeshTypeFromIndex(newMeshIndex_);

		std::string texturePath = "Resources/ParticleTexture/default.png";
		if (!textureFilePaths_.empty()) {
			texturePath = (fs::path(textureDirectory_) / textureFilePaths_[newTextureIndex_]).string();
		}

		manager_->CreateParticleGroup(effectName, texturePath, meshType, std::move(behavior));
	}

	if (ImGui::Button("Save To Json")) {
		manager_->SaveToJson("Resources/Data/Particles.json");
	}
}

void ParticleEditor::BasicIMGui(const std::string& currentName)
{
	auto& groups = manager_->particleGroups;
	auto& currentGroup = groups.at(currentName);

	ImGui::SeparatorText("Basic");
	ImGui::Text("ID: %s", currentName.c_str());

	int count = static_cast<int>(currentGroup.defaultCount);
	if (ImGui::DragInt("Count", &count, 1, 1, 1000)) {
		currentGroup.defaultCount = (uint32_t)std::max(count, 1);
	}
	ImGui::DragFloat("Lifetime", &currentGroup.defaultLifetime, 0.1f, 0.1f, 10.0f);
	ImGui::ColorEdit4("Color", &currentGroup.defaultColor.x);
	ImGui::Checkbox("Loop", &currentGroup.isLoop);
	if (currentGroup.isLoop) {
		ImGui::DragFloat("Loop Interval", &currentGroup.loopInterval, 0.01f, 0.05f, 10.0f);
	}
}

void ParticleEditor::VertexTypeIMGui(const std::string& currentName)
{
	auto& groups = manager_->particleGroups;
	auto& currentGroup = groups.at(currentName);

	ImGui::SeparatorText("Mesh");

	int shapeIndex = 0;
	switch (currentGroup.verticesType) {
	case VerticesType::Ring:     shapeIndex = 0; break;
	case VerticesType::Cylinder: shapeIndex = 1; break;
	case VerticesType::Quad:     shapeIndex = 2; break;
	case VerticesType::Triangle: shapeIndex = 3; break;
	}

	if (ImGui::Combo("Shape", &shapeIndex, "Ring\0Cylinder\0Quad\0Triangle\0")) {
		manager_->SetGroupVerticesType(currentName, GetMeshTypeFromIndex(shapeIndex));
	}
}

void ParticleEditor::TextureSelectIMGui(const std::string& currentName)
{
	auto& groups = manager_->particleGroups;
	auto& currentGroup = groups.at(currentName);

	if (textureFilePaths_.empty()) {
		ImGui::Text("No textures in %s", textureDirectory_.c_str());
		return;
	}

	int texIndex = 0;
	std::string currentFileName =
		fs::path(currentGroup.materialdata.textureFilePath).filename().string();

	for (int i = 0; i < (int)textureFilePaths_.size(); ++i) {
		if (textureFilePaths_[i] == currentFileName) {
			texIndex = i;
			break;
		}
	}

	std::vector<const char*> texItems;
	texItems.reserve(textureFilePaths_.size());
	for (auto& s : textureFilePaths_) {
		texItems.push_back(s.c_str());
	}

	if (ImGui::Combo("Texture", &texIndex, texItems.data(), (int)texItems.size())) {
		const std::string& fileName = textureFilePaths_[texIndex];
		std::string fullPathStr = (fs::path(textureDirectory_) / fileName).string();
		manager_->SetGroupTexture(currentName, fullPathStr);
	}

	ImGui::Text("Path: %s", currentGroup.materialdata.textureFilePath.c_str());
}

void ParticleEditor::BehaviorIMGui(const std::string& currentName)
{
	auto& groups = manager_->particleGroups;
	auto& currentGroup = groups.at(currentName);

	ImGui::SeparatorText("Behavior");

	const auto& behaviorNames = ParticleBehaviorFactory::GetBehaviorNames();
	if (!behaviorNames.empty()) {
		// 現在のbehavior型から index 推定
		std::string currentType = "Explosion";
		if (dynamic_cast<ChargeBehabiaor*>(currentGroup.behavior.get())) {
			currentType = "Charge";
		}
		else if (dynamic_cast<ExhaustGasBehavior*>(currentGroup.behavior.get())) {
			currentType = "ExhaustGas";
		}

		editBehaviorIndex_ = 0;
		for (int i = 0; i < (int)behaviorNames.size(); ++i) {
			if (behaviorNames[i] == currentType) {
				editBehaviorIndex_ = i;
				break;
			}
		}

		std::vector<const char*> behaviorItems;
		behaviorItems.reserve(behaviorNames.size());
		for (const auto& name : behaviorNames) {
			behaviorItems.push_back(name.c_str());
		}

		if (ImGui::Combo("Behavior Type", &editBehaviorIndex_,
			behaviorItems.data(), (int)behaviorItems.size())) {
			auto newBehavior = ParticleBehaviorFactory::Create(behaviorNames[editBehaviorIndex_]);
			if (newBehavior) {
				manager_->SetBehavior(currentName, std::move(newBehavior));
			}
		}
	}

	if (currentGroup.behavior) {
		ImGui::SeparatorText("Behavior Params");
		currentGroup.behavior->DrawImgui(currentName.c_str());

		if (ImGui::Button("Test Emit (Camera)")) {
			currentGroup.particles.clear();
			manager_->EmitAtCamera(currentName);
		}
	}
}

void ParticleEditor::LoadTexturesFromDirectory(const std::string& directory)
{
	textureDirectory_ = directory;
	textureFilePaths_.clear();

	for (const auto& entry : fs::directory_iterator(directory)) {
		if (!entry.is_regular_file()) {
			continue;
		}

		auto path = entry.path();
		std::string ext = path.extension().string();
		std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

		if (ext == ".png") {
			textureFilePaths_.push_back(path.filename().string());
		}
	}

	std::sort(textureFilePaths_.begin(), textureFilePaths_.end());
}

void ParticleEditor::DrawBehaviorSelector(const char* label, int& index)
{
	const auto& behaviorNames = ParticleBehaviorFactory::GetBehaviorNames();

	std::vector<const char*> items;
	items.reserve(behaviorNames.size());
	for (const auto& name : behaviorNames) {
		items.push_back(name.c_str());
	}

	if (!items.empty()) {
		if (index >= (int)items.size()) {
			index = 0;
		}
		ImGui::Combo(label, &index, items.data(), (int)items.size());
	}
}

void ParticleEditor::DrawMeshSelector(const char* label, int& index)
{
	static const char* meshItems[] = {
		"Ring",
		"Cylinder",
		"Quad",
		"Triangle",
	};

	ImGui::Combo(label, &index, meshItems, IM_ARRAYSIZE(meshItems));
}

void ParticleEditor::DrawTextureSelector(const char* label, int& index)
{
	std::vector<const char*> textureItems;
	textureItems.reserve(textureFilePaths_.size());
	for (const auto& path : textureFilePaths_) {
		textureItems.push_back(path.c_str());
	}

	if (!textureItems.empty()) {
		if (index >= (int)textureItems.size()) {
			index = 0;
		}
		ImGui::Combo(label, &index, textureItems.data(), (int)textureItems.size());
	}
}

VerticesType ParticleEditor::GetMeshTypeFromIndex(int index) const
{
	switch (index) {
	case 0: return VerticesType::Ring;
	case 1: return VerticesType::Cylinder;
	case 2: return VerticesType::Quad;
	case 3: return VerticesType::Triangle;
	default: return VerticesType::Quad;
	}
}

#endif // USE_IMGUI