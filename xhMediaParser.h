#pragma once
#include "xhVideoParser.h"
#include "xhAudioParser.h"
#include <iostream>

#ifdef _H_MEDIAPARSER_MODULE_SDK
class __declspec(dllexport) xhMediaParser : public xhVideoParser,public xhAudioParser
#else
class __declspec(dllimport) xhMediaParser : public xhVideoParser,public xhAudioParser
#endif
{
public:
	enum StreamType
	{
	VIDEO=0,
	AUDIO=1,
	};
	xhMediaParser(void);
	virtual ~xhMediaParser(void);

	int OpenMediaFile(std::string strFilePath);			//open a media file
	void CloseMediaFile();								//close file

	AVPacket* GetNextVideoFrameOfFile(int64_t &curPTS);
	AVPacket* GetNextAudioFrameOfFile(int64_t &curPTS);
	AVPacket* GetNextFrameOfFile(int64_t &curPTS, StreamType &sType);

	MediaFormat* GetMediaFileFroamt(){return &m_mediaFmt;};
	int CreateVideoDecodeEnv(MediaFormat *lpMFormat) {return xhVideoParser::CreateVideoDecodeEnv(lpMFormat);};	//decode video stream
	int CreateAudioDecodeEnv(MediaFormat *lpMFormat) {return xhAudioParser::CreateAudioDecodeEnv(lpMFormat);};	//decode audio stream

	bool SeekFrameTimestamp(int64_t nNewTimestamp);
	inline int GetFramesOfSecond(){return m_mediaFmt.VFormat.r_frame_rare.num/m_mediaFmt.VFormat.r_frame_rare.den;}	//Get frame cout of on second 
	inline int GetDelayTime(){return m_mediaFmt.VFormat.tbase.num*1000000/m_mediaFmt.VFormat.tbase.den;};			//Get frame's delay time of million
	inline int GetFrameDistance(){return (m_mediaFmt.VFormat.tbase.den/m_mediaFmt.VFormat.tbase.num)/(m_mediaFmt.VFormat.r_frame_rare.num/m_mediaFmt.VFormat.r_frame_rare.den);};		//Get frame's timestamp distance
private:
	AVFormatContext* m_lpFileCxt;		//main struct information
	MediaFormat m_mediaFmt;				//media file information
	AVPacket m_videoPkOfFile;			//video frame
	AVPacket m_audioPkOfFile;			//audio frame
};
