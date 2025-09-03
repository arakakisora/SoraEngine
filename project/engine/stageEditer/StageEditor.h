
#pragma once
#include <vector>
#include <string>
//#include "GridCell.h"

struct GridCell {
	int type = 0; // 0=empty, 1=block, 2=enemy, 3=player
};
class StageEditor {
public:
	// ファイル名バッファ
	char fileNameBuffer[64] = "stage1.csv";
	std::vector<std::string> availableStages;
	int selectedStageIndex = 0;

	void Run();
	bool GetReloadRequested() const { return isReloadRequested_; }
	void SetReloadRequested(bool value) { isReloadRequested_ = value; }
	const char* GetFileName() const { return fileNameBuffer; }
private:
	void RenderUI();
	void SaveCSV(const std::string& filename);
	void LoadCSV(const std::string& filename);

	std::vector<std::vector<GridCell>> grid_;
	int selectedType_ = 1;
	bool isReloadRequested_ = false; // GamePlayScene のメンバに追加
};
