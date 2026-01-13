
#pragma once
#include <vector>
#include <string>
#include "MapChipDatabase.h"

/// <summary>
/// グリッドセル構造体
/// </summary>
struct GridCell {
	int type = 0; // 0=empty, 1=block, 2=enemy, 3=player
};

/// <summary>
/// ステージエディタークラス
/// </summary>
class StageEditor {
public:
	// ファイル名バッファ
	char fileNameBuffer[64] = "stage1.csv";
	std::vector<std::string> availableStages;
	int selectedStageIndex = 0;

	/// <summary>
	/// エディターの実行
	/// </summary>
	void Run();
	/// <summary>
	/// リロードが要求されたかどうかを取得します
	/// </summary>
	/// <returns></returns>
	bool GetReloadRequested() const { return isReloadRequested_; }
	/// <summary>
	/// リロード要求を設定します
	/// </summary>
	/// <param name="value"></param>
	void SetReloadRequested(bool value) { isReloadRequested_ = value; }
	/// <summary>
	/// ファイル名を取得します
	/// </summary>
	const char* GetFileName() const { return fileNameBuffer; }
private:
	/// <summary>
	/// UIの描画
	/// </summary>
	void RenderUI();
	/// <summary>
	/// CSVの保存と読み込み
	/// </summary>
	void SaveCSV(const std::string& filename);
	/// <summary>
	/// CSVの読み込み
	/// </summary>
	void LoadCSV(const std::string& filename);
private:
	std::vector<std::vector<GridCell>> grid_;
	int selectedType_ = 1;
	bool isReloadRequested_ = false;

};
