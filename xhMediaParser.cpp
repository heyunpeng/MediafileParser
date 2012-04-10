#include "xhMediaParser.h"

#pragma comment(lib, "avcodec.lib")
#pragma comment(lib, "avformat.lib")
xhMediaParser::xhMediaParser(void):m_lpFileCxt(NULL)
{
	avcodec_init();
	avcodec_register_all();
	av_register_all();
	av_init_packet(&m_videoPkOfFile);
	av_init_packet(&m_audioPkOfFile);
}

xhMediaParser::~xhMediaParser(void)
{
	CloseMediaFile();
}

void xhMediaParser::CloseMediaFile()
{
	if (m_lpFileCxt != NULL)
	{
		av_close_input_file(m_lpFileCxt);
		m_lpFileCxt = NULL;
	}
}

int xhMediaParser::OpenMediaFile(std::string strFilePath)
{
	if (strFilePath.empty())
		return -1;

	CloseMediaFile();

	if (av_open_input_file(&m_lpFileCxt, strFilePath.c_str(), NULL, 0, NULL) != 0)
		return -1;

	if (av_find_stream_info(m_lpFileCxt) == -1)
	{
		av_close_input_file(m_lpFileCxt);
		m_lpFileCxt = NULL;
		return -1;
	}

	//The media file length of seconds
	m_mediaFmt.nTotalSeconds = m_lpFileCxt->duration/AV_TIME_BASE;

	//nb_streams is this file have stream number
	for (unsigned int i = 0; i < m_lpFileCxt->nb_streams; i++)
	{
		if (m_lpFileCxt->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO)
		{
			m_mediaFmt.nVideoIndex = i;
			AVCodecContext *lpCode = m_lpFileCxt->streams[i]->codec;
			m_mediaFmt.VFormat.cID = lpCode->codec_id;
			m_mediaFmt.VFormat.nWigth = lpCode->width;
			m_mediaFmt.VFormat.nHeight = lpCode->height;
			m_mediaFmt.VFormat.pix_fmt = lpCode->pix_fmt;
			m_mediaFmt.VFormat.tbase.num = lpCode->time_base.num;
			m_mediaFmt.VFormat.tbase.den = lpCode->time_base.den;
			m_mediaFmt.VFormat.r_frame_rare.num = m_lpFileCxt->streams[i]->r_frame_rate.num;
			m_mediaFmt.VFormat.r_frame_rare.den = m_lpFileCxt->streams[i]->r_frame_rate.den;

			if (lpCode->extradata_size != 0)
			{
				m_mediaFmt.VFormat.lpExdata = new uint8_t[lpCode->extradata_size];
				memmove(m_mediaFmt.VFormat.lpExdata, lpCode->extradata, lpCode->extradata_size);
				m_mediaFmt.VFormat.nExdataSize = lpCode->extradata_size;
			}
		}
		else if (m_lpFileCxt->streams[i]->codec->codec_type == AVMEDIA_TYPE_AUDIO)
		{
			m_mediaFmt.nAudioIndex = i;
			AVCodecContext *lpCode = m_lpFileCxt->streams[i]->codec;
			m_mediaFmt.AFormat.cID = lpCode->codec_id;
			m_mediaFmt.AFormat.frameSize = lpCode->frame_size;
			m_mediaFmt.AFormat.channels = lpCode->channels;
			m_mediaFmt.AFormat.sample_rate = lpCode->sample_rate;
			m_mediaFmt.AFormat.bits_per_sample = av_get_bits_per_sample(lpCode->codec_id);
			if (m_mediaFmt.AFormat.bits_per_sample <= 0) m_mediaFmt.AFormat.bits_per_sample = 16;
			if (lpCode->extradata_size != 0)
			{
				m_mediaFmt.AFormat.lpExdata = new uint8_t(lpCode->extradata_size);
				memmove(m_mediaFmt.AFormat.lpExdata, lpCode->extradata, lpCode->extradata_size);
				m_mediaFmt.AFormat.nExdataSize = lpCode->extradata_size;
			}
		}
	}

	return 1;
}

AVPacket* xhMediaParser::GetNextVideoFrameOfFile(int64_t &curPTS)
{
	if (m_lpFileCxt == NULL || m_mediaFmt.nVideoIndex == -1)
		return NULL;

	do 
	{
		if (av_read_frame(m_lpFileCxt, &m_videoPkOfFile) < 0 || m_videoPkOfFile.data == NULL)
			return NULL;
	} while (m_videoPkOfFile.stream_index != m_mediaFmt.nVideoIndex);

	if (AV_NOPTS_VALUE	!= m_videoPkOfFile.pts)
		curPTS = m_videoPkOfFile.pts;
	else
		curPTS = 0;

	return &m_videoPkOfFile;
}

AVPacket* xhMediaParser::GetNextAudioFrameOfFile(int64_t &curPTS)
{
	if (m_lpFileCxt == NULL || m_mediaFmt.nAudioIndex == -1)
		return NULL;

	do 
	{
		if (av_read_frame(m_lpFileCxt, &m_audioPkOfFile) < 0 || m_audioPkOfFile.data == NULL)
			return NULL;
	} while (m_audioPkOfFile.stream_index != m_mediaFmt.nAudioIndex);

	if (AV_NOPTS_VALUE	!= m_audioPkOfFile.pts)
		curPTS = m_audioPkOfFile.pts;
	else
		curPTS = 0;

	return &m_audioPkOfFile;
}

AVPacket* xhMediaParser::GetNextFrameOfFile(int64_t &curPTS, StreamType &sType)
{
	if (m_lpFileCxt == NULL)
		return NULL;

	if (av_read_frame(m_lpFileCxt, &m_audioPkOfFile) < 0 || m_audioPkOfFile.data == NULL)
		return NULL;
	if (m_mediaFmt.nVideoIndex == m_audioPkOfFile.stream_index)
		sType = VIDEO;
	else
		sType = AUDIO;

	if (AV_NOPTS_VALUE	!= m_audioPkOfFile.pts)
		curPTS = m_audioPkOfFile.pts;
	else
		curPTS = 0;

	return &m_audioPkOfFile;	
}

bool xhMediaParser::SeekFrameTimestamp(int64_t nNewTimestamp)
{
	if(!m_lpFileCxt)
		return false;

	return av_seek_frame(m_lpFileCxt, 0, nNewTimestamp, 0) >=0;
}