//Author: cruxeon
//Learning to open and read video files

#include <libavutil/opt.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <h264_stream.h>

#define NSTREAMS 5

//Global Variables
static AVFormatContext *ofmt_ctx;


//Function to open an output file and write codecs, header and stream information
static int open_output_file(const char *filename) {

    AVStream *out_stream;
    AVCodecContext *enc_ctx;
    AVCodec *encoder;

    int ret;
    unsigned int i;

    ofmt_ctx = NULL;
    avformat_alloc_output_context2(&ofmt_ctx, NULL, NULL, filename);
    if (!ofmt_ctx)
        return AVERROR_UNKNOWN;


    for (i = 0; i < NSTREAMS; i++) {

        //Allocating multiple video output streams
        out_stream = avformat_new_stream(ofmt_ctx, NULL);
        if (!out_stream) {
            av_log(NULL, AV_LOG_ERROR, "Failed allocating output stream\n");
            return AVERROR_UNKNOWN;
        }

        enc_ctx = out_stream->codec;


        //Encoding to same codec as input --Change as needed--
        encoder = avcodec_find_encoder(AV_CODEC_ID_H264);
        if (!encoder) {
            av_log(NULL, AV_LOG_FATAL, "Necessary encoder not found\n");
            return AVERROR_INVALIDDATA;
        }

        enc_ctx->height = 1080;
        enc_ctx->width = 1920;
        enc_ctx->pix_fmt = encoder->pix_fmts[0];
        enc_ctx->gop_size = 10;
        enc_ctx->qmin = 10;
        enc_ctx->qmax = 51;
        av_opt_set(enc_ctx, "preset", "slow", 0);


        ret = avcodec_open2(enc_ctx, encoder, NULL);
        if (ret < 0) {
            av_log(NULL, AV_LOG_ERROR, "Cannot open video encoder for stream #%u\n", i);
            return ret;
        }

        if (ofmt_ctx->oformat->flags & AVFMT_GLOBALHEADER)
            enc_ctx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
    }

    //Show details of output format
    av_dump_format(ofmt_ctx, 0, filename, 1);

    //Open output file, if needed
    if (!(ofmt_ctx->oformat->flags & AVFMT_NOFILE)) {
        ret = avio_open(&ofmt_ctx->pb, filename, AVIO_FLAG_WRITE);
        if (ret < 0) {
            av_log(NULL, AV_LOG_ERROR, "Could not open output file '%s'\n", filename);
            return ret;
        }
    }
    
    ret = avformat_write_header(ofmt_ctx, NULL);
    if (ret < 0) {
        av_log(NULL, AV_LOG_ERROR, "Unable to write stream header to output file\n");
        return ret;
    }

    return 0;
}


int main(int argc, char **argv) {

    int ret;
    
    //Register all available file formats
    av_register_all();

    ret = open_output_file(argv[1]);
    av_write_trailer(ofmt_ctx);
     
    return ret ? 1 : 0;
}
