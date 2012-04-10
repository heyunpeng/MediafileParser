#include "xhAudioParser.h"
#include <exception>

xhAudioParser::xhAudioParser(void):m_lpADBuffer(NULL),m_lpAudioCodecCtx(NULL),m_lpADCode(NULL)
{
}

xhAudioParser::~xhAudioParser(void)
{
	CloseAudioDecodeEnv();
}

int xhAudioParser::CreateAudioDecodeEnv(MediaFormat *lpMFormat)	//decode audio stream
{
	if (lpMFormat == NULL)
		return -1;

	CloseAudioDecodeEnv();

	m_lpAudioCodecCtx = avcodec_alloc_context();
	m_lpAudioCodecCtx->codec_id = lpMFormat->AFormat.cID;
	m_lpAudioCodecCtx->codec_type = AVMEDIA_TYPE_AUDIO;

	m_lpADCode = avcodec_find_decoder(lpMFormat->AFormat.cID);
	if (m_lpADCode == NULL)
	{
		av_free(m_lpAudioCodecCtx);
		m_lpAudioCodecCtx = NULL;
		return -1;
	}

	if (avcodec_open(m_lpAudioCodecCtx, m_lpADCode) < 0)
	{
		av_free(m_lpADCode);
		m_lpADCode = NULL;
		av_free(m_lpAudioCodecCtx);
		m_lpAudioCodecCtx = NULL;
		return -1;
	}

	m_lpADBuffer = new int16_t[AVCODEC_MAX_AUDIO_FRAME_SIZE];

	return 1;
}
void xhAudioParser::CloseAudioDecodeEnv()
{
	if (m_lpADBuffer)
	{
		delete []m_lpADBuffer;
		m_lpADBuffer = NULL;
	}

	if (m_lpAudioCodecCtx)
	{
		avcodec_close(m_lpAudioCodecCtx);
		av_free(m_lpAudioCodecCtx);
		m_lpAudioCodecCtx = NULL;
	}

	if (m_lpADCode)
	{
		av_free(m_lpADCode);
		m_lpADCode = NULL;
	}
}

int16_t* xhAudioParser::DecodeAudio(AVPacket *packet, int &nPkSize)
{
	if (m_lpAudioCodecCtx == NULL)
		return NULL;

	int nBufferSize = AVCODEC_MAX_AUDIO_FRAME_SIZE;
	if (avcodec_decode_audio3(m_lpAudioCodecCtx, m_lpADBuffer, &nBufferSize, packet) < 0)
		return NULL;

	nPkSize = nBufferSize;
	return m_lpADBuffer;
}