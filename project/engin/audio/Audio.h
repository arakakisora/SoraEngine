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

	void StopAudio();
	//getter
	IXAudio2* GetXAudio2() const { return xAudio2.Get(); }

private:
	//aoudio
	ComPtr<IXAudio2> xAudio2=nullptr;
	IXAudio2MasteringVoice* masterVoice=nullptr;
	//波形フォーマット元にSourcwvoiceの生成
	IXAudio2SourceVoice* pSourceVoice = nullptr;
};

