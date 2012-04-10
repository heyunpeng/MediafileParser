#pragma once

extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
};

#pragma comment (lib, "avutil.lib")
#pragma comment (lib, "swscale.lib")
#pragma comment (lib, "avcodec.lib")

#ifdef _H_MEDIAPARSER_MODULE_SDK
typedef struct __declspec(dllexport) _MediaFormat
#else
typedef struct __declspec(dllimport) _MediaFormat
#endif
{
#ifdef _H_MEDIAPARSER_MODULE_SDK
	typedef struct __declspec(dllexport) _timebase
#else
	typedef struct __declspec(dllimport) _timebase
#endif
	{
		int num;
		int den;
		_timebase():num(1),den(1000)
		{
		}
	}timebase;
#ifdef _H_MEDIAPARSER_MODULE_SDK
	typedef struct __declspec(dllexport) _AudioFormat
#else
	typedef struct __declspec(dllimport) _AudioFormat
#endif
	{
		CodecID cID;
		int channels;				//audio channel number
		int sample_rate;			//audio sample rate
		int bits_per_sample;		//bit length of sample
		int frameSize;				//size of each frame
		uint8_t *lpExdata;
		int nExdataSize;
		_AudioFormat():cID(CODEC_ID_NONE),sample_rate(0),bits_per_sample(0),
			lpExdata(NULL),nExdataSize(0),channels(0),frameSize(0)
		{
		}
		_AudioFormat& operator = (_AudioFormat &tp)
		{
			if (tp.lpExdata == NULL)
			{
				memmove(this, &tp, sizeof(tp));
				return *this;
			}
			cID = tp.cID;
			channels = tp.channels;
			sample_rate = tp.sample_rate;
			lpExdata = new uint8_t[tp.nExdataSize];
			memmove(lpExdata, tp.lpExdata, tp.nExdataSize);
			nExdataSize = tp.nExdataSize;
			return *this;
		}
	}AudioFormat;
#ifdef _H_MEDIAPARSER_MODULE_SDK
	typedef struct __declspec(dllexport) _VideoFormat
#else
	typedef struct __declspec(dllimport) _VideoFormat
#endif
	{
		CodecID cID;					//video decode type
		unsigned short nWigth;
		unsigned short nHeight;
		PixelFormat pix_fmt;			//video decode format,excemple:YUV 4:2:0
		timebase tbase;					//采样频率
		timebase r_frame_rare;			//帧率
		uint8_t *lpExdata;
		int nExdataSize;
		_VideoFormat():nWigth(0), nHeight(0),
			lpExdata(NULL),nExdataSize(0),cID(CODEC_ID_NONE),pix_fmt(PIX_FMT_YUV420P)
		{}
		~_VideoFormat()
		{
			if (lpExdata)
			{
				delete []lpExdata;
			}
		}
		_VideoFormat& operator = (_VideoFormat &tp)
		{
			if (tp.lpExdata == NULL)
			{
				memmove(this, &tp, sizeof(tp));
				return *this;
			}

			cID = tp.cID;
			nWigth = tp.nWigth;
			nHeight = tp.nHeight;
			pix_fmt = tp.pix_fmt;
			tbase = tp.tbase;
			r_frame_rare = tp.r_frame_rare;
			lpExdata = new uint8_t[tp.nExdataSize];
			memmove(lpExdata, tp.lpExdata, tp.nExdataSize);
			nExdataSize = tp.nExdataSize;
			return *this;
		}
	}VideoFormat, *lpVideoFromat;
	VideoFormat VFormat;
	AudioFormat AFormat;
	int64_t nTotalSeconds;			//video length of second
	int nVideoIndex;
	int nAudioIndex;
	_MediaFormat():nVideoIndex(-1),nAudioIndex(-1),nTotalSeconds(0)
	{
	}
}MediaFormat, *lpMediaFormat;