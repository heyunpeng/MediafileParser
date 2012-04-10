#pragma once
#include <ostream>
#include "xhCommentHeader.h"

#ifdef _H_MEDIAPARSER_MODULE_SDK
class __declspec(dllexport) xhWriteMediaFile
#else
class __declspec(dllimport) xhWriteMediaFile
#endif
{
public:
	xhWriteMediaFile(void);
	virtual ~xhWriteMediaFile(void);

public:
	bool OpenMediaFile(std::string strFilePath, MediaFormat* lpMediaFmt);
	int WriteFrameToFile(AVPacket* lpPkt);
	void CloseMediaFile();

private:
	AVStream* AddVideoStream(AVFormatContext* lpFileCxt, MediaFormat::VideoFormat* lpVFormat);
	AVStream* AddAudioStream(AVFormatContext* lpFileCxt, MediaFormat::AudioFormat* lpAFormat);
private:
	AVFormatContext*	m_lpFileCxt;
};
