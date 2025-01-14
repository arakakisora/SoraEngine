#pragma once
#include <xaudio2.h>
#pragma comment(lib,"xaudio2.lib")
#include <wrl.h>
#include <fstream>
#include <cassert>



struct ChunkHeader {

	char id[4];
	int32_t size;

};

struct RiffHeader {

	ChunkHeader chunk;
	char type[4];

};

struct FormatChunk {

	ChunkHeader chunk;
	WAVEFORMATEX fmt;

};

struct SoundData {

	//波長フォーマット
	WAVEFORMATEX wfex;
	//バッファの先頭アドレス
	BYTE* PBuffer;
	//バッファのサイズ
	unsigned int bufferSize;

};

class Audio
{
	static Audio* instance_;

	Audio() = default;
	~Audio() = default;
	Audio(const Audio&) = delete;
	Audio& operator=(const Audio&) = delete;


public:

	template <class T> using ComPtr = Microsoft::WRL::ComPtr<T>;

	static Audio* GetInstance();

	void  Initialize();

	void Finalize();

	//Waveの読み込み関数
	SoundData SoundLoadWave(const char* filename);
	//解放関数
	void SoundUnload(SoundData* soundData);
	//再生関数
	void SoundPlayWave(const SoundData& soundData);

	//停止関数
	void StopAudio();

	//一時停止
	void PauseAudio();
	//再開
	void ResumeAudio();

	//再生速度の設定
	void SetPlaybackSpeed(float speed);



	//getter
	IXAudio2* GetXAudio2() const { return xAudio2.Get(); }

	bool IsSoundPlaying() const;



private:
	//aoudio
	ComPtr<IXAudio2> xAudio2=nullptr;
	IXAudio2MasteringVoice* masterVoice=nullptr;
	//波形フォーマット元にSourcwvoiceの生成
	IXAudio2SourceVoice* pSourceVoice = nullptr;

private:
	UINT64 playCursor = 0; // 再生位置（バイト単位）を保持
	bool isPaused = false; // 一時停止状態を管理
	SoundData currentSoundData; // 現在再生中のサウンドデータ

};

