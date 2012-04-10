#pragma once
#include "xhCommentHeader.h"

#ifdef _H_MEDIAPARSER_MODULE_SDK
class __declspec(dllexport) xhVideoParser
#else
class __declspec(dllimport) xhVideoParser
#endif
{
public:
	xhVideoParser(void);
	virtual ~xhVideoParser(void);
public:
	int CreateVideoDecodeEnv(MediaFormat *lpMFormat);	//decode video stream
	void CloseVideoDecodeEnv();

	AVFrame* DecodeVideo(AVPacket *packet);

	//Convert data format, example yuv to rgb24
	int CreateConvertEnv(PixelFormat srcPIXType, int nSrcWidth, int nSrcHight, PixelFormat desPIXType, int nDesWidth, int nDesHight);													//创建转换格式环境
	int ConvertFormat(int nHeigth, AVFrame *pFrame, uint8_t** pData, int &nLen);
	void CloseConverEnv();

	void YUVtoUYVY(uint8_t *y_plane, uint8_t *u_plane, uint8_t *v_plane, int y_stride, 
		int uv_stride, uint8_t* &pDstBuf, int width, int height);

	int SeekFrameTimestamp(int64_t nNewTimestamp);
private:
	AVCodecContext* m_lpVideoCodecCtx;	//video code environment
	AVCodec* m_lpVDCode;				//video codec
	AVFrame* m_lpAVFrame;
	uint8_t *m_pCodeBuf;
	SwsContext *m_pSwsCxt;
	short m_nVideonIndex;		//video stream index in the file
	AVFrame* m_lpVDDecodeFrame;	//The frame of video codec
};
