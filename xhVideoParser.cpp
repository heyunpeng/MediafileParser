#include "xhVideoParser.h"

xhVideoParser::xhVideoParser(void):m_lpVideoCodecCtx(NULL),m_lpVDCode(NULL),m_pCodeBuf(NULL),
m_pSwsCxt(NULL),m_lpVDDecodeFrame(NULL),m_lpAVFrame(NULL)
{
}

xhVideoParser::~xhVideoParser(void)
{
	CloseConverEnv();
	CloseVideoDecodeEnv();
}

int xhVideoParser::CreateVideoDecodeEnv(MediaFormat *lpMFormat)
{
	if (lpMFormat == NULL)
		return -1;

	CloseVideoDecodeEnv();

	m_lpVideoCodecCtx = avcodec_alloc_context();
	m_lpVideoCodecCtx->codec_id = lpMFormat->VFormat.cID;
	m_lpVideoCodecCtx->codec_type = AVMEDIA_TYPE_VIDEO;
	m_lpVideoCodecCtx->width = lpMFormat->VFormat.nWigth;
	m_lpVideoCodecCtx->height = lpMFormat->VFormat.nHeight;
	m_lpVideoCodecCtx->pix_fmt = lpMFormat->VFormat.pix_fmt;
	m_lpVideoCodecCtx->time_base.den = lpMFormat->VFormat.tbase.den;
	m_lpVideoCodecCtx->time_base.num = lpMFormat->VFormat.tbase.num;
	if (lpMFormat->VFormat.nExdataSize != 0)
	{
		m_lpVideoCodecCtx->extradata = new uint8_t[lpMFormat->VFormat.nExdataSize];
		memmove(m_lpVideoCodecCtx->extradata, lpMFormat->VFormat.lpExdata, lpMFormat->VFormat.nExdataSize);
		m_lpVideoCodecCtx->extradata_size = lpMFormat->VFormat.nExdataSize;
	}

	m_lpVDCode = avcodec_find_decoder(lpMFormat->VFormat.cID);
	if (m_lpVDCode == NULL)
	{
		av_free(m_lpVideoCodecCtx);
		m_lpVideoCodecCtx = NULL;
		return -1;
	}

	//接受截断的bit流，两帧之间的边界不一定刚好是包得边界
	if(m_lpVDCode->capabilities&CODEC_CAP_TRUNCATED)
		m_lpVideoCodecCtx->flags|= CODEC_FLAG_TRUNCATED;

	if (avcodec_open(m_lpVideoCodecCtx, m_lpVDCode) < 0)
	{
		av_free(m_lpVDCode);
		m_lpVDCode = NULL;
		av_free(m_lpVideoCodecCtx);
		m_lpVideoCodecCtx = NULL;
		return -1;
	}

	m_lpVDDecodeFrame = avcodec_alloc_frame();
	if (m_lpVDDecodeFrame == NULL)
	{
		avcodec_close(m_lpVideoCodecCtx);
		av_free(m_lpVDCode);
		m_lpVDCode = NULL;
		av_free(m_lpVideoCodecCtx);
		m_lpVideoCodecCtx = NULL;
		return -1;
	}

	return 1;
}

void xhVideoParser::CloseVideoDecodeEnv()
{
	if (m_lpVDDecodeFrame)
	{
		av_free(m_lpVDDecodeFrame);
		m_lpVDDecodeFrame = NULL;
	}

	if (m_lpVideoCodecCtx)
	{
		avcodec_close(m_lpVideoCodecCtx);
		av_free(m_lpVideoCodecCtx);
		m_lpVideoCodecCtx = NULL;
	}

	if (m_lpVDCode)
	{
		av_free(m_lpVDCode);
		m_lpVDCode = NULL;
	}
}

AVFrame* xhVideoParser::DecodeVideo(AVPacket *packet)
{
	if (m_lpVideoCodecCtx == NULL)
		return NULL;

	int nFrameFin = 0;
	if (avcodec_decode_video2(m_lpVideoCodecCtx, m_lpVDDecodeFrame, &nFrameFin, packet) > 0 && nFrameFin == 0)
		avcodec_decode_video2(m_lpVideoCodecCtx, m_lpVDDecodeFrame, &nFrameFin, packet);

	if (nFrameFin == 0)
		return NULL;
	else
		return m_lpVDDecodeFrame;
}

void xhVideoParser::CloseConverEnv()
{
	if (m_pSwsCxt)
	{
		sws_freeContext(m_pSwsCxt);
		m_pSwsCxt = NULL;
	}

	if (m_lpAVFrame)
	{
		av_free(m_lpAVFrame);
		m_lpAVFrame = NULL;
	}

	if (m_pCodeBuf)
	{
		delete []m_pCodeBuf;
		m_pCodeBuf = NULL;
	}
}

int xhVideoParser::CreateConvertEnv(PixelFormat srcPIXType, int nSrcWidth, int nSrcHight, PixelFormat desPIXType, int nDesWidth, int nDesHight)
{
	CloseConverEnv();

	m_lpAVFrame = avcodec_alloc_frame();
	if (m_lpAVFrame == NULL)
		return -1;

	//计算一帧数据的最大占用空间
	int nBytes = avpicture_get_size(desPIXType, nDesWidth, nDesHight);
	m_pCodeBuf = new uint8_t[nBytes];
	if (m_pCodeBuf == NULL)
	{
		av_free(m_lpAVFrame);
		m_lpAVFrame = NULL;
		return -1;
	}

	//将m_lpAVFrame的数据按照格式关联到buffer中
	avpicture_fill((AVPicture *)m_lpAVFrame, m_pCodeBuf, desPIXType, nDesWidth, nDesHight);
	m_pSwsCxt = sws_getContext(nSrcWidth, nSrcHight, srcPIXType, nDesWidth, nDesHight, desPIXType, SWS_BICUBIC, NULL, NULL, NULL);
	if (m_pSwsCxt == NULL)
	{
		av_free(m_lpAVFrame);
		m_lpAVFrame = NULL;
		delete []m_pCodeBuf;
		m_pCodeBuf = NULL;
		return -1;
	}

	return nBytes;
}

int xhVideoParser::ConvertFormat(int nHeigth, AVFrame *pFrame, uint8_t** pData, int &nLen)
{
	//top side down the image
	pFrame->data [0] += pFrame->linesize[0] *  (nHeigth-1);
	pFrame->data [1] += pFrame->linesize[1] *  (nHeigth/2-1);
	pFrame->data [2] += pFrame->linesize[2] *  (nHeigth/2-1);
	pFrame->linesize[0] = -pFrame->linesize[0];
	pFrame->linesize[1] = -pFrame->linesize[1];
	pFrame->linesize[2] = -pFrame->linesize[2];

	if (sws_scale(m_pSwsCxt, pFrame->data, pFrame->linesize, 0, nHeigth, m_lpAVFrame->data, m_lpAVFrame->linesize) <= 0)
		return -1;

	*pData = m_lpAVFrame->data[0];
	nLen = m_lpAVFrame->linesize[0]*nHeigth;

	return 1;

}

void xhVideoParser::YUVtoUYVY(uint8_t *y_plane, uint8_t *u_plane, uint8_t *v_plane, int y_stride, 
							  int uv_stride, uint8_t* &pDstBuf, int width, int height)
{
	for (int row = 0; row < height; row = row + 1) 
	{
		for (int col = 0; col < width; col=col + 2) 
		{
			pDstBuf[0] = y_plane[row * y_stride + col];
			pDstBuf[1] = u_plane[row/2 * uv_stride + col/2];
			pDstBuf[2] = y_plane[row * y_stride + col+1];
			pDstBuf[3] = v_plane[row/2 * uv_stride + col/2];
			pDstBuf += 4;
		}
	}
}