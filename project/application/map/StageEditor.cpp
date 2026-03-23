#include "StageEditor.h"
#ifdef USE_IMGUI
#include "imgui.h"
#endif // USE_IMGUI
#include <fstream>
#include <iostream>
#include <filesystem>
#include "MapChipDatabase.h"

namespace fs = std::filesystem;

void StageEditor::Run() {
	// 初期化
	if (grid_.empty()) {
		// グリッドの初期化
		constexpr int kGridWidth = 50;
		constexpr int kGridHeight = 25;
		// 2Dグリッドを指定サイズで初期化
		grid_.resize(kGridHeight, std::vector<GridCell>(kGridWidth));
	}
	// ストローク訪問管理の初期化
	if (strokeVisited_.empty()) {
		// グリッドと同じサイズで初期化
		strokeVisited_.resize(grid_.size(), std::vector<bool>(grid_[0].size(), false));
	}
	RenderUI();
}

void StageEditor::RenderUI() {
#ifdef USE_IMGUI

	if (fs::exists("Resources/Mapdata") && fs::is_directory("Resources/Mapdata")) {
		std::vector<std::string> newList;
		for (const auto& entry : fs::directory_iterator("Resources/Mapdata")) {
			if (entry.path().extension() == ".csv") {
				newList.push_back(entry.path().filename().string());
			}
		}
		std::sort(newList.begin(), newList.end());
		if (newList != availableStages) {
			availableStages = std::move(newList);
			if (!availableStages.empty()) {
				if (selectedStageIndex >= availableStages.size()) selectedStageIndex = 0;
				strncpy_s(fileNameBuffer, sizeof(fileNameBuffer), availableStages[selectedStageIndex].c_str(), _TRUNCATE);
				fileNameBuffer[sizeof(fileNameBuffer) - 1] = '\0';
			} else {
				selectedStageIndex = 0;
				fileNameBuffer[0] = '\0';
			}
		}
	}

	ImGui::Begin("Stage Editor");
	ImGuiIO& io = ImGui::GetIO();

	// Ctrl+Z / Ctrl+Y
	if (io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_Z)) { Undo(); }
	if (io.KeyCtrl && (ImGui::IsKeyPressed(ImGuiKey_Y) || (io.KeyShift && ImGui::IsKeyPressed(ImGuiKey_Z)))) { Redo(); }

	ImGui::Checkbox("Show Stage Window", &showStagewindow_);

	ImGui::InputText("FileName", fileNameBuffer, IM_ARRAYSIZE(fileNameBuffer));
	if (ImGui::Button("New")) {
		for (auto& row : grid_) {
			for (auto& cell : row) {
				cell.type = 0; // 空白に初期化
			}
		}
	}

	// セーブ
	if (ImGui::Button("Save CSV")) {
		// ファイルパスの生成
		std::string fullPath = "Resources/Mapdata/" + std::string(fileNameBuffer);
		SaveCSV(fullPath);
	}

	// ステージリスト（ドロップダウン） — 常に表示。空なら "No Stages"
	const char* comboPreview = availableStages.empty() ? "No Stages" : availableStages[selectedStageIndex].c_str();
	if (ImGui::BeginCombo("Stage List", comboPreview)) {
		if (availableStages.empty()) {
			ImGui::Selectable("No stages", false, ImGuiSelectableFlags_Disabled);
		} else {
			for (int i = 0; i < static_cast<int>(availableStages.size()); ++i) {
				const bool isSelected = (i == selectedStageIndex);
				if (ImGui::Selectable(availableStages[i].c_str(), isSelected)) {
					selectedStageIndex = i;
					strncpy_s(fileNameBuffer, sizeof(fileNameBuffer), availableStages[i].c_str(), _TRUNCATE);
					fileNameBuffer[sizeof(fileNameBuffer) - 1] = '\0'; // null終端
				}
				if (isSelected) {
					ImGui::SetItemDefaultFocus();
				}
			}
		}
		ImGui::EndCombo();
	}

	// ロード（選択中のファイル）
	if (ImGui::Button("Load Selected Stage")) {
		if (!availableStages.empty()) {
			// 選択中のステージをロード
			std::string selectedPath = "Resources/Mapdata/" + availableStages[selectedStageIndex];
			LoadCSV(selectedPath);
		}
	}

	if (ImGui::Button("Reload Map")) {
		isReloadRequested_ = true;
	}

	for (const auto& info : MapChipDatabase::GetInstance()->GetAll()) {
		ImGui::RadioButton(info.label.c_str(), &selectedType_, static_cast<int>(info.id));
	}

	ImGui::End();

	ImGui::Begin("Stage");

	const float kCellSize = 15.0f;
	const int rows = static_cast<int>(grid_.size());
	const int cols = rows > 0 ? static_cast<int>(grid_[0].size()) : 0;

	ImGui::BeginChild("StageCanvasChild", ImVec2(0, 0), true, ImGuiWindowFlags_HorizontalScrollbar);

	ImDrawList* dl = ImGui::GetWindowDrawList();
	ImVec2 canvasPos = ImGui::GetCursorScreenPos();
	ImVec2 scroll = ImVec2(ImGui::GetScrollX(), ImGui::GetScrollY());

	// キャンバス全体サイズ
	ImVec2 canvasSize(cols * kCellSize, rows * kCellSize);

	// 1個だけ入力用の透明ボタンを置く
	ImGui::InvisibleButton("StageCanvasButton", canvasSize,
		ImGuiButtonFlags_MouseButtonLeft | ImGuiButtonFlags_MouseButtonRight | ImGuiButtonFlags_MouseButtonMiddle);

	bool isHovered = ImGui::IsItemHovered();
	bool isActive = ImGui::IsItemActive();

	ImVec2 mousePos = ImGui::GetIO().MousePos;

	// 背景
	dl->AddRectFilled(
		canvasPos,
		ImVec2(canvasPos.x + canvasSize.x, canvasPos.y + canvasSize.y),
		IM_COL32(40, 40, 40, 255));

	// 見えてる範囲だけ描画する
	ImVec2 winPos = ImGui::GetWindowPos();
	ImVec2 winSize = ImGui::GetWindowSize();

	float visibleMinX = scroll.x;
	float visibleMinY = scroll.y;
	float visibleMaxX = scroll.x + winSize.x;
	float visibleMaxY = scroll.y + winSize.y;

	int startX = std::max(0, static_cast<int>(visibleMinX / kCellSize));
	int startY = std::max(0, static_cast<int>(visibleMinY / kCellSize));
	int endX = std::min(cols, static_cast<int>(visibleMaxX / kCellSize) + 2);
	int endY = std::min(rows, static_cast<int>(visibleMaxY / kCellSize) + 2);

	// マウス座標→セル座標
	int hoverX = -1;
	int hoverY = -1;
	if (isHovered) {
		hoverX = static_cast<int>((mousePos.x - canvasPos.x) / kCellSize);
		hoverY = static_cast<int>((mousePos.y - canvasPos.y) / kCellSize);
		if (hoverX < 0 || hoverX >= cols || hoverY < 0 || hoverY >= rows) {
			hoverX = -1;
			hoverY = -1;
		}
	}

	// ストローク開始/終了
	if (isHovered) {
		if (ImGui::IsMouseClicked(ImGuiMouseButton_Left) || ImGui::IsMouseClicked(ImGuiMouseButton_Right)) {
			BeginStroke();
			isEditingDrag_ = true;
			dragButton_ = ImGui::IsMouseClicked(ImGuiMouseButton_Left) ? ImGuiMouseButton_Left : ImGuiMouseButton_Right;
		}

		if (ImGui::IsMouseClicked(ImGuiMouseButton_Middle) && hoverX >= 0 && hoverY >= 0) {
			selectedType_ = grid_[hoverY][hoverX].type;
		}
	}

	if (isEditingDrag_) {
		if ((dragButton_ == ImGuiMouseButton_Left && ImGui::IsMouseDown(ImGuiMouseButton_Left)) ||
			(dragButton_ == ImGuiMouseButton_Right && ImGui::IsMouseDown(ImGuiMouseButton_Right))) {
			if (hoverX >= 0 && hoverY >= 0) {
				if (dragButton_ == ImGuiMouseButton_Left) {
					ApplyCellWithUndo(hoverX, hoverY, selectedType_);
				}
				if (dragButton_ == ImGuiMouseButton_Right) {
					ApplyCellWithUndo(hoverX, hoverY, 0);
				}
			}
		}

		if ((dragButton_ == ImGuiMouseButton_Left && ImGui::IsMouseReleased(ImGuiMouseButton_Left)) ||
			(dragButton_ == ImGuiMouseButton_Right && ImGui::IsMouseReleased(ImGuiMouseButton_Right))) {
			isEditingDrag_ = false;
			dragButton_ = -1;
			EndStroke();
		}
	}

	// セル描画
	for (int y = startY; y < endY; ++y) {
		for (int x = startX; x < endX; ++x) {
			const GridCell& cell = grid_[y][x];
			MapChipType typeId = static_cast<MapChipType>(cell.type);
			const MapChipInfo* info = MapChipDatabase::GetInstance()->GetById(typeId);

			ImVec4 colorV = ImVec4(1, 1, 1, 1);
			if (info) {
				colorV = ImVec4(info->color.x, info->color.y, info->color.z, info->color.w);
			}

			ImU32 fillColor = ImGui::ColorConvertFloat4ToU32(colorV);

			ImVec2 p0(canvasPos.x + x * kCellSize, canvasPos.y + y * kCellSize);
			ImVec2 p1(p0.x + kCellSize, p0.y + kCellSize);

			dl->AddRectFilled(p0, p1, fillColor);
			dl->AddRect(p0, p1, IM_COL32(25, 25, 25, 255));

			if (info && info->collision != "none") {
				dl->AddRect(p0, p1, IM_COL32(0, 0, 0, 255), 0.0f, 0, 1.0f);
			}

			if (info && info->hitPoints > 0) {
				dl->AddLine(ImVec2(p0.x, p1.y), ImVec2(p1.x, p0.y), IM_COL32(0, 0, 0, 255), 1.0f);
			}

			if (info && !info->spawn.empty() && info->spawn != "none") {
				ImVec2 c((p0.x + p1.x) * 0.5f, (p0.y + p1.y) * 0.5f);
				dl->AddCircleFilled(c, 3.0f, IM_COL32(0, 0, 0, 255));
			}
		}
	}

	// ホバー中セルを強調
	if (hoverX >= 0 && hoverY >= 0) {
		ImVec2 p0(canvasPos.x + hoverX * kCellSize, canvasPos.y + hoverY * kCellSize);
		ImVec2 p1(p0.x + kCellSize, p0.y + kCellSize);
		dl->AddRect(p0, p1, IM_COL32(255, 255, 0, 255), 0.0f, 0, 2.0f);

		const MapChipInfo* hoverInfo =
			MapChipDatabase::GetInstance()->GetById(static_cast<MapChipType>(grid_[hoverY][hoverX].type));

		if (hoverInfo) {
			ImGui::BeginTooltip();
			ImGui::Text("id:%d  %s", hoverInfo->id, hoverInfo->label.c_str());
			ImGui::Text("collision: %s", hoverInfo->collision.c_str());
			ImGui::Text("hp: %d", hoverInfo->hitPoints);
			ImGui::Text("spawn: %s", hoverInfo->spawn.c_str());
			ImGui::EndTooltip();
		}
	}

	ImGui::EndChild();
	ImGui::End();

#endif // USE_IMGUI
}

void StageEditor::SaveCSV(const std::string& filename) {
	fs::create_directories("Resources/Mapdata"); // フォルダがなければ作成
	std::ofstream file(filename);
	for (const auto& row : grid_) {
		for (size_t x = 0; x < row.size(); ++x) {
			file << row[x].type;
			if (x + 1 < row.size()) file << ",";
		}
		file << "\n";
	}
}

void StageEditor::LoadCSV(const std::string& filename) {
	// ファイルを開く
	std::ifstream file(filename);
	std::string line;
	int y = 0;
	// 1行ずつ読み込み
	//ホットリロード
	while (std::getline(file, line)) {
		int x = 0;
		size_t start = 0;
		while (start < line.size()) {
			size_t end = line.find(',', start);
			if (end == std::string::npos) end = line.size();
			int val = std::stoi(line.substr(start, end - start));
			if (y < grid_.size() && x < grid_[y].size()) {
				grid_[y][x].type = val;
			}
			start = end + 1;
			++x;
		}
		++y;
	}
}

void StageEditor::BeginStroke()
{
	// ストローク開始
	isStrokeActive_ = true;// ストロークアクティブ化
	currentStroke_.clear();// 現在のストローククリア
	// すでに記録したセルの重複防止用配列をクリア
	for (auto& row : strokeVisited_) {
		// セル訪問情報をクリア
		std::fill(row.begin(), row.end(), false);
	}

}

void StageEditor::EndStroke()
{
	if (!isStrokeActive_) return;
	isStrokeActive_ = false;

	if (!currentStroke_.empty()) {
		undoStack_.push_back(currentStroke_);
		redoStack_.clear(); // 新規編集が入ったらRedoは消す
	}
	currentStroke_.clear();

}

void StageEditor::ApplyCellWithUndo(int x, int y, int newType)
{
	int& cell = grid_[y][x].type;
	if (cell == newType) return;

	// ストローク中、同じセルを二重記録しない
	if (isStrokeActive_ && !strokeVisited_[y][x]) {
		strokeVisited_[y][x] = true;
		currentStroke_.push_back(CellEdit{ x, y, cell, newType });
	}
	cell = newType;
}

void StageEditor::Undo()
{
	if (undoStack_.empty()) { return; }
	Stroke s = undoStack_.back();
	undoStack_.pop_back();

	// 逆順で戻す
	for (auto it = s.rbegin(); it != s.rend(); ++it) {
		grid_[it->y][it->x].type = it->before;
	}
	redoStack_.push_back(std::move(s));
}

void StageEditor::Redo()
{
	
	if (redoStack_.empty()) { return; }// リドゥスタックが空なら何もしない
	Stroke s = redoStack_.back();
	redoStack_.pop_back();
	// 順番に適用する
	for (auto& e : s) {
		grid_[e.y][e.x].type = e.after;
	}
	undoStack_.push_back(std::move(s));
}
