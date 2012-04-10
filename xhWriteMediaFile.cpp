#include "xhWriteMediaFile.h"
#include "xhCommentHeader.h"
#include <boost\algorithm\string_regex.hpp>

xhWriteMediaFile::xhWriteMediaFile(void):m_lpFileCxt(NULL)
{
	av_register_all();
}

xhWriteMediaFile::~xhWriteMediaFile(void)
{
}

AVStream* xhWriteMediaFile::AddAudioStream(AVFormatContext* lpFileCxt, MediaFormat::AudioFormat* lpAFormat)
{
	AVStream* lpAVstream = av_new_stream(lpFileCxt, 0);
	if (lpAVstream == NULL)
		return NULL;
	AVCodecContext* lpAVCodeCxt = lpAVstream->codec;
	lpAVCodeCxt->codec_id = lpAFormat->cID;
	lpAVCodeCxt->codec_type = AVMEDIA_TYPE_AUDIO;
	lpAVCodeCxt->channels = lpAFormat->channels;
	lpAVCodeCxt->sample_rate = lpAFormat->sample_rate;
	lpAVCodeCxt->sample_rate = lpAFormat->bits_per_sample;
	return lpAVstream;
}

AVStream* xhWriteMediaFile::AddVideoStream(AVFormatContext* lpFileCxt, MediaFormat::VideoFormat* lpVFormat)
{
	AVStream* lpAVStream = av_new_stream(lpFileCxt, 0);
	if (lpAVStream == NULL)
		return NULL;
	AVCodecContext* lpAVCodeCxt = lpAVStream->codec;
	lpAVCodeCxt->codec_id = lpVFormat->cID;
	lpAVCodeCxt->codec_type = AVMEDIA_TYPE_VIDEO;
	lpAVCodeCxt->width = lpVFormat->nWigth;
	lpAVCodeCxt->height = lpVFormat->nHeight;
	lpAVCodeCxt->time_base.den = lpVFormat->tbase.den;
	lpAVCodeCxt->time_base.num = lpVFormat->tbase.num;
	lpAVCodeCxt->pix_fmt = lpVFormat->pix_fmt;

	if (lpVFormat->nExdataSize != 0)
		lpAVCodeCxt->flags |= CODEC_FLAG_GLOBAL_HEADER;
	
	return lpAVStream;
}

bool xhWriteMediaFile::OpenMediaFile(std::string strFilePath, MediaFormat* lpMediaFmt)
{
	if (strFilePath.size() < 3 || lpMediaFmt == NULL)
		return false;
	
	boost::iterator_range<std::string::iterator> iterIdx = boost::algorithm::find_last(strFilePath, ".");
	if (iterIdx.begin() == iterIdx.end())
		return false;
	int nPos = iterIdx.begin() - strFilePath.begin() + 1;

	AVOutputFormat* lpOut = av_guess_format(strFilePath.c_str()+nPos, NULL, NULL);
	if (lpOut == NULL)
		return false;

	m_lpFileCxt = avformat_alloc_context();
	if (m_lpFileCxt == NULL)
		return false;
	m_lpFileCxt->oformat = lpOut;

	strcpy(m_lpFileCxt->filename, strFilePath.c_str());

	AVStream *lpVideoStream = NULL;
	AVStream *lpAudioStream = NULL;
	if (lpMediaFmt->VFormat.cID != CODEC_ID_NONE)
		lpVideoStream = AddVideoStream(m_lpFileCxt, &lpMediaFmt->VFormat);
	if (lpMediaFmt->AFormat.cID != CODEC_ID_NONE)
		lpAudioStream = AddAudioStream(m_lpFileCxt, &lpMediaFmt->AFormat);

	if (av_set_parameters(m_lpFileCxt, NULL) < 0)
	{
		avformat_free_context(m_lpFileCxt);
		m_lpFileCxt = NULL;
		return false;
	}
	
	if (url_fopen(&m_lpFileCxt->pb, strFilePath.c_str(), URL_WRONLY) < 0)
	{
		avformat_free_context(m_lpFileCxt);
		m_lpFileCxt = NULL;
	}

	av_write_header(m_lpFileCxt);
	return true;
}

void xhWriteMediaFile::CloseMediaFile()
{
	if (m_lpFileCxt == NULL)
		return;

	av_write_trailer(m_lpFileCxt);
	url_fclose(m_lpFileCxt->pb);
	avformat_free_context(m_lpFileCxt);
	m_lpFileCxt = NULL;
}

int xhWriteMediaFile::WriteFrameToFile(AVPacket* lpPkt)
{
	if (m_lpFileCxt == NULL)
		return -1;

	return av_write_frame(m_lpFileCxt, lpPkt);
}