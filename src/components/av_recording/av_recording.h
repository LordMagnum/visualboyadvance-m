#ifndef VBAM_COMPONENTS_AV_RECORDING_AV_RECORDING_H_
#define VBAM_COMPONENTS_AV_RECORDING_AV_RECORDING_H_

// simplified interface for recording audio and/or video from emulator

extern "C" {
/* From: http://redino.net/blog/2013/12/uint64_c-defined-including-libavformatavformat-h-vs-2008/ */

#include <cstdint>

#ifndef INT64_C
#define INT64_C(c) (c ## LL)
#endif

#ifndef UINT64_C
#define UINT64_C(c) (c ## ULL)
#endif

#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/avassert.h>
#include <libavutil/channel_layout.h>
#include <libavutil/opt.h>
#include <libavutil/mathematics.h>
#include <libavutil/timestamp.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
#include <libswresample/swresample.h>
}

#include <vector>

namespace recording {


// get supported audio/video codecs
std::vector<char *> getSupVidNames();
std::vector<char *> getSupVidExts();
std::vector<char *> getSupAudNames();
std::vector<char *> getSupAudExts();


// return codes
enum MediaRet {
        MRET_OK,            // no errors
        MRET_ERR_NOMEM,     // error allocating buffers or structures
        MRET_ERR_NOCODEC,   // error opening codec
        MRET_ERR_FERR,      // error writing output file
        MRET_ERR_RECORDING, // attempt to start recording when already doing it
        MRET_ERR_FMTGUESS,  // can't guess format from file name
        MRET_ERR_BUFSIZE    // buffer overflow (fatal)
};

class MediaRecorder
{
        public:
        MediaRecorder();
        virtual ~MediaRecorder();

        // start audio+video (also video-only codecs)
        MediaRet Record(const char *fname, int width, int height, int depth);
        // start audio only
        MediaRet Record(const char *fname);
        // stop both
        void Stop(bool initSuccess = true);
        bool IsRecording()
        {
                return isRecording;
        }
        // add a frame of video; width+height+depth already given
        // assumes a 1-pixel border on top & right
        // always assumes being passed 1/60th of a second of video
        MediaRet AddFrame(const uint8_t *vid);
        // add a frame of audio; uses current sample rate to know length
        // always assumes being passed 1/60th of a second of audio;
        // single sample, though (we need one for each channel).
        MediaRet AddFrame(const uint16_t *aud, int length);
        // set sampleRate; we need this to remove the GBA file header
        // include.
        void SetSampleRate(int newSampleRate)
        {
                sampleRate = newSampleRate;
        }

        private:
        bool isRecording;
        int sampleRate;
        AVFormatContext *oc;
        const AVOutputFormat *fmt;
        // pic info
        AVPixelFormat pixfmt;
        int pixsize, linesize;
        int tbord, rbord;
        struct SwsContext *sws;
        // stream info
        AVStream *st;
        const AVCodec *vcodec;
        AVCodecContext *enc;
        int64_t npts; // for video frame pts
        AVFrame *frameIn;
        AVFrame *frameOut;
        // audio
        bool audioOnlyRecording;
        struct SwrContext *swr;
        const AVCodec *acodec;
        AVStream *ast;
        AVCodecContext *aenc;
        int64_t samplesCount; // for audio frame pts generation
        AVFrame *audioframe;
        AVFrame *audioframeTmp;
        // audio buffer
        uint16_t *audioBuffer;
        int posInAudioBuffer;
        int samplesInAudioBuffer;
        int audioBufferSize;

        MediaRet setup_common(const char *fname);
        MediaRet setup_video_stream_info(int width, int height, int depth);
        MediaRet setup_video_stream(int width, int height);
        MediaRet setup_audio_stream();
        MediaRet finish_setup(const char *fname);
        // flush last frames to avoid
        // "X frames left in the queue on closing"
        void flush_frames();
};

}

#endif  // VBAM_COMPONENTS_AV_RECORDING_AV_RECORDING_H_
