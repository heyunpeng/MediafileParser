#pragma once
#include "xhCommentHeader.h"

#ifdef _H_MEDIAPARSER_MODULE_SDK
class __declspec(dllexport) xhAudioParser
#else
class __declspec(dllimport) xhAudioParser
#endif
{
public:
	xhAudioParser(void);
	virtual ~xhAudioParser(void);
public:
	int CreateAudioDecodeEnv(MediaFormat *lpMFormat);	//decode audio stream
	void CloseAudioDecodeEnv();
	int16_t* DecodeAudio(AVPacket *packet, int &nPkSize);
private:
	AVFormatContext* m_lpFileCxt;		//main struct information
	AVCodecContext* m_lpAudioCodecCtx;	//audio code environment
	AVCodec* m_lpADCode;				//audio codec
	int16_t* m_lpADBuffer;				//The frame of audio codec
};
