








#include "config.h"
#include "AAC.h"

AVCodec *pCodec = NULL;
AVCodecContext *pCodecCtx = NULL;
int i, ret, got_output;

FILE *fp_in;
FILE *fp_out;

AVFrame *pFrame;
uint8_t* frame_buf;
int size = 0;

AVPacket pkt;
int y_size = 0;
int framecnt = 0;

char filename_in[] = "tdjm.pcm";
AVCodecID codec_id = AV_CODEC_ID_AAC;
char filename_out[] = "tdjm.aac";

int framenum = 1000;


// 初始化音频器
int init_audio(){

	// 常规注册ffmepg
	avcodec_register_all();


	// 常规初始化
	pCodec = avcodec_find_encoder(AV_CODEC_ID_AAC);
	if(!pCodec){
		printf("Codec not found");
		return -1;
	}

	pCodecCtx = avcodec_alloc_context3(pCodec);
	if(!pCodecCtx){
		printf("CodecContext not found");
		return -1;
	}

	pCodecCtx->codec_id = AV_CODEC_ID_AAC;
	pCodecCtx->codec_type = AVMEDIA_TYPE_AUDIO;
	pCodecCtx->sample_fmt = AV_SAMPLE_FMT_S16;
	pCodecCtx->sample_rate= 44100;
	pCodecCtx->channel_layout=AV_CH_LAYOUT_STEREO;
	pCodecCtx->channels = av_get_channel_layout_nb_channels(pCodecCtx->channel_layout);
	pCodecCtx->bit_rate = 64000;  
	

	if (avcodec_open2(pCodecCtx, pCodec, NULL) < 0) {
        printf("Could not open codec\n");
        return -1;
    }


	// 初始化一帧，用于存储数据
	pFrame = av_frame_alloc();
	pFrame->nb_samples = pCodecCtx->frame_size;
	pFrame->format = pCodecCtx->sample_fmt;
	size = av_samples_get_buffer_size(NULL, pCodecCtx->channels, pCodecCtx->frame_size, pCodecCtx->sample_fmt, 1);
	frame_buf = (uint8_t *)av_malloc(size);
	avcodec_fill_audio_frame(pFrame, pCodecCtx->channels, pCodecCtx->sample_fmt, (const uint8_t*)frame_buf, size, 1);

	return 1;
}

// 获取一帧的大小，用于存入数据
// 此方法应该在初始化之后操作
int get_frame_size(){
	return size;
}


// pcm转aac
int pcm_to_aac(uint8_t* pcm_data){
	memcpy(frame_buf, pcm_data, size);
	av_init_packet(&pkt);
    pkt.data = NULL;    // packet data will be allocated by the encoder
    pkt.size = 0;

	pFrame->pts = i;
    ret = avcodec_encode_audio2(pCodecCtx, &pkt, pFrame, &got_output);
    if (ret < 0) {
        printf("Error encoding frame\n");
        return -1;
    }
    if (got_output) {
        printf("Succeed to encode frame: %5d\tsize:%5d\n",framecnt,pkt.size);
		framecnt++;
        // fwrite(pkt.data, 1, pkt.size, fp_out);
		// 此处输出encoder之后的aac数据
        av_free_packet(&pkt);
    }
}


// 关闭音频
int close_audio(){
	for (got_output = 1; got_output; i++) {
        ret = avcodec_encode_audio2(pCodecCtx, &pkt, NULL, &got_output);
        if (ret < 0) {
            printf("Error encoding frame\n");
            return -1;
        }
        if (got_output) {
            printf("Flush Encoder: Succeed to encode 1 frame!\tsize:%5d\n",pkt.size);
			// fwrite(pkt.data, 1, pkt.size, fp_out);
			// 此处输出encoder之后的aac数据
            av_free_packet(&pkt);
        }
    }

    fclose(fp_out);
    avcodec_close(pCodecCtx);
    av_free(pCodecCtx);
    av_freep(&pFrame->data[0]);
    av_frame_free(&pFrame);
	pCodecCtx = NULL;
	pFrame = NULL;
}



int main(int argc, char* argv[]){
	return 1;
}