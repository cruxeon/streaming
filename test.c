//Author: cruxeon
//Learning to open and read video files

#include <libavutil/opt.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <h264_stream.h>



//Global Variables
static AVFormatContext *ifmt_ctx;
static AVFormatContext *ofmt_ctx;


//Function to open a video file and check for decoders
static int open_input_file(const char *filename) {

    int ret;
    unsigned int i;
    
    //Retrieve header information
    ifmt_ctx = NULL;
    if ((ret = avformat_open_input(&ifmt_ctx, filename, NULL, NULL)) < 0) {
        av_log(NULL, AV_LOG_ERROR, "Cannot open input file\n");
        return ret;
    }

    //Retrieve stream information
    if ((ret = avformat_find_stream_info(ifmt_ctx, NULL)) < 0) {
        av_log(NULL, AV_LOG_ERROR, "Cannot find stream information\n");
        return ret;
    }
    
    //Check if decoder present for every video stream
    for (i = 0; i < ifmt_ctx->nb_streams; i++) {
        AVStream *stream;
        AVCodecContext *codec_ctx;

        stream = ifmt_ctx->streams[i];
        codec_ctx = stream->codec;

        if (codec_ctx->codec_type == AVMEDIA_TYPE_VIDEO) {
           
            ret = avcodec_open2(codec_ctx, avcodec_find_decoder(codec_ctx->codec_id), NULL);
            if (ret < 0) {
                av_log(NULL, AV_LOG_ERROR, "Failed to open decoder for stream #%u\n", i);
                return ret;
            }
        }
    }
     
    //Show details of the input format
    av_dump_format(ifmt_ctx, 0, filename, 0);
    return 0;
}


//Function to open an output file and write codecs, header and stream information
static int open_output_file(const char *filename) {

    AVStream *out_stream;
    AVStream *in_stream;
    AVCodecContext *dec_ctx, *enc_ctx;
    AVCodec *encoder;

    int ret;
    unsigned int i;

    ofmt_ctx = NULL;
    avformat_alloc_output_context2(&ofmt_ctx, NULL, NULL, filename);
    if (!ofmt_ctx) {
        av_log(NULL, AV_LOG_DEBUG, "Could not deduce output format from file extension: using MPEG\n");
        avformat_alloc_output_context2(&ofmt_ctx, NULL, "mpeg", filename);
    }
    if (!ofmt_ctx)
        return AVERROR_UNKNOWN;

    
    for (i = 0; i < ifmt_ctx->nb_streams; i++) {
        
        //Allocating output stream for each input stream   
        out_stream = avformat_new_stream(ofmt_ctx, NULL);
        if (!out_stream) {
            av_log(NULL, AV_LOG_ERROR, "Failed allocating output stream\n");
            return AVERROR_UNKNOWN;
        }

        in_stream = ifmt_ctx->streams[i];
        dec_ctx = in_stream->codec;
        enc_ctx = out_stream->codec;

        if (dec_ctx->codec_type == AVMEDIA_TYPE_VIDEO) {
        
            //Encoding to same codec as input --Change as needed--
            encoder = avcodec_find_encoder(dec_ctx->codec_id);
            if (!encoder) {
                av_log(NULL, AV_LOG_FATAL, "Necessary encoder not found\n");
                return AVERROR_INVALIDDATA;
            }
            
            //Encoding to same video properties as input --Change as needed--
            if (dec_ctx->codec_type == AVMEDIA_TYPE_VIDEO) {
                enc_ctx->height = dec_ctx->height;
                enc_ctx->width = dec_ctx->width;
                enc_ctx->sample_aspect_ratio = dec_ctx->sample_aspect_ratio;
                enc_ctx->pix_fmt = encoder->pix_fmts[0];
                enc_ctx->time_base = dec_ctx->time_base;
                
                //additional information to create h264 encoder
                enc_ctx->bit_rate = dec_ctx->bit_rate;
                enc_ctx->gop_size = 10;
                enc_ctx->qmin = 10;
                enc_ctx->qmax = 51;

                if (dec_ctx->codec_id == AV_CODEC_ID_H264) {
                    av_opt_set(enc_ctx->priv_data, "preset", "slow", 0);
                }
            }
            
            ret = avcodec_open2(enc_ctx, encoder, NULL);
            if (ret < 0) {
                av_log(NULL, AV_LOG_ERROR, "Cannot open video encoder for stream #%u\n", i);
                return ret;
            }

        } else if (dec_ctx->codec_type == AVMEDIA_TYPE_UNKNOWN) {
            av_log(NULL, AV_LOG_FATAL, "Elementary stream #%d is of unknown type\n", i);
            return AVERROR_INVALIDDATA;

        } else {

            //Remux remaining streams
            ret = avcodec_copy_context(ofmt_ctx->streams[i]->codec, ifmt_ctx->streams[i]->codec);
            if (ret < 0) {
                av_log(NULL, AV_LOG_ERROR, "Copying stream context failed\n");
                return ret;
            }
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


//Function to encode and write frames
static int encode_write_frame(AVFrame *frame, unsigned int stream_index, int *got_frame) {
    
    int ret;
    int got_frame_local;
    AVPacket enc_pkt;

    if (!got_frame)
        got_frame = &got_frame_local;

    //Encoding the frame
    //av_log(NULL, AV_LOG_INFO, "Encoding frame\n");
    enc_pkt.data = NULL;
    enc_pkt.size = 0;
    av_init_packet(&enc_pkt);
    ret = avcodec_encode_video2(ofmt_ctx->streams[stream_index]->codec, &enc_pkt,
                                    frame, got_frame);
    if (ret < 0)
        return ret;
    if (!(*got_frame))
        return 0;

    //Prepare packet for muxing
    enc_pkt.stream_index = stream_index;
    av_packet_rescale_ts(&enc_pkt,
                            ofmt_ctx->streams[stream_index]->codec->time_base,
                            ofmt_ctx->streams[stream_index]->time_base);
    
    unsigned char msg[5] = "hello";
     
    //Write SEI NAL unit
    h264_stream_t* h = h264_new();

    h->nal->nal_ref_idc = NAL_REF_IDC_PRIORITY_HIGHEST;
    h->nal->nal_unit_type = NAL_UNIT_TYPE_SEI;

    h->sei->payloadType = SEI_TYPE_USER_DATA_UNREGISTERED;
    h->sei->payloadSize = 5;
    h->sei->payload = msg;

    unsigned int size = 1000;
    unsigned char buf[size];
    int len = write_nal_unit(h, buf, size);
    av_log(NULL, AV_LOG_DEBUG, "size written: %d\n", len);


    //Mux encoded frame
    //av_log(NULL, AV_LOG_INFO, "Muxing frame\n");
    ret = av_interleaved_write_frame(ofmt_ctx, &enc_pkt);
    return ret;
}


//Function to flush encoder and write to output frame
static int flush_encoder(unsigned int stream_index) {

    int ret;
    int got_frame;
    
    if (!(ofmt_ctx->streams[stream_index]->codec->codec->capabilities & AV_CODEC_CAP_DELAY))
        return 0;
    
    while (1) {

        av_log(NULL, AV_LOG_INFO, "Flushing stream #%u encoder\n", stream_index);
        ret = encode_write_frame(NULL, stream_index, &got_frame);
        if (ret < 0)
            break;
        if (!got_frame)
            return 0;
    }

    return ret;
}


int main(int argc, char **argv) {
    
    AVPacket packet = { .data = NULL, .size = 0 };
    AVFrame *frame = NULL;
    enum AVMediaType type;
    unsigned int stream_index;
    int got_frame;
    
    int ret;
    unsigned int i;

    if (argc != 3) {
        av_log(NULL, AV_LOG_ERROR, "Usage: %s <input file> <output file>\n", argv[0]);
        return 1;
    }

    //Register all available file formats
    av_register_all();
    avformat_network_init();
    
    if ((ret = open_input_file(argv[1])) < 0)
        goto end;

    if ((ret = open_output_file(argv[2])) < 0)
        goto end;
    
    
    //Read all packets
    while (1) {
        
        if ((ret = av_read_frame(ifmt_ctx, &packet)) < 0)
            break;

        stream_index = packet.stream_index;
        type = ifmt_ctx->streams[packet.stream_index]->codec->codec_type;
        av_log(NULL, AV_LOG_DEBUG, "Demuxer gave frame of stream_index %u\n", stream_index);

        if (type == AVMEDIA_TYPE_VIDEO) {
            
            frame = av_frame_alloc();
            if (!frame) {
                ret = AVERROR(ENOMEM);
                break;
            }

            av_packet_rescale_ts(&packet,
                                    ifmt_ctx->streams[stream_index]->time_base,
                                    ifmt_ctx->streams[stream_index]->codec->time_base);

            //Decoding video frames
            ret = avcodec_decode_video2(ifmt_ctx->streams[stream_index]->codec,
                                        frame, &got_frame, &packet);
            if (ret < 0) {
                av_frame_free(&frame);
                av_log(NULL, AV_LOG_ERROR, "Decoding failed\n");
                break;
            }
            
            //Encoding video frames
            if (got_frame) {
                frame->pts = av_frame_get_best_effort_timestamp(frame);
                ret = encode_write_frame(frame, stream_index, NULL);
                av_frame_free(&frame);
                if (ret < 0)
                    goto end;
            } else {
                av_frame_free(&frame);
            }

        } else {

            //Remux this frame without reencoding
            av_packet_rescale_ts(&packet,
                                    ifmt_ctx->streams[stream_index]->time_base,
                                    ofmt_ctx->streams[stream_index]->time_base);

            ret = av_interleaved_write_frame(ofmt_ctx, &packet);
            if (ret < 0)
                goto end;
        
        }
        av_packet_unref(&packet);
    }


    //Flush encoders
    for (i = 0; i < ifmt_ctx->nb_streams; i++) {
        ret = flush_encoder(i);
        av_log(NULL, AV_LOG_DEBUG, "Flushing encoder (i)\n");
        if (ret < 0) {
            av_log(NULL, AV_LOG_ERROR, "Flushing encoder failed\n");
            goto end;
        }
    }

    av_write_trailer(ofmt_ctx);
end:
    av_packet_unref(&packet);
    av_frame_free(&frame);

    for (i = 0; i < ifmt_ctx->nb_streams; i++) {
        //Freeing input codec data
        avcodec_close(ifmt_ctx->streams[i]->codec);

        //Freeing output codec data
        if (ofmt_ctx && ofmt_ctx->nb_streams > i
                && ofmt_ctx->streams[i] && ofmt_ctx->streams[i]->codec)
            avcodec_close(ofmt_ctx->streams[i]->codec);
    }
    
    //Closing i/o AVFormatContexts
    avformat_close_input(&ifmt_ctx);
    if (ofmt_ctx && !(ofmt_ctx->oformat->flags & AVFMT_NOFILE))
        avio_closep(&ofmt_ctx->pb);
    avformat_free_context(ofmt_ctx);

    if (ret < 0)
        av_log(NULL, AV_LOG_ERROR, "Error occurred: %s\n", av_err2str(ret));

    return ret ? 1 : 0;
}
