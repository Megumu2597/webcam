#define __STDC_CONSTANT_MACROS
#define __STDC_LIMIT_MACROS
#include <deque>
extern "C" {
#include <libavutil/imgutils.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>

#include <libavutil/opt.h>
#include <libavutil/parseutils.h>
#include <libavutil/avutil.h>
}
#include <opencv2/highgui/highgui.hpp>
#include <vector>
#include <stdio.h>
#include <string.h>
#include <iostream>
#include <unistd.h>

#pragma comment(lib, "avutil.lib")
#pragma comment(lib, "avcodec.lib")
#pragma comment(lib, "avformat.lib")

//コードの詳細は, https://proc-cpuinfo.fixstars.com/2017/09/ffmpeg-api-encode/ を参照.

//ここで動画のfps決めてる
//AVRational time_base = av_make_q(1001, 30000);
int output_fps = 24;
AVRational time_base = av_make_q(1001, output_fps * 1000);
std::deque<AVFrame*> frames;

static void on_frame_decoded(AVFrame* frame) {
  AVFrame* new_ref = av_frame_alloc();
  av_frame_ref(new_ref, frame);
  frames.push_back(new_ref);
}

std::deque<AVFrame*> frames_stream;
const char* input_path = "/dev/video2";// "/home/ubuntu/webcam/PSDK_0004.mp4";// ffmpeg_codec.mp4";//
//const char* input_path = "output_video2_1280.mp4";//PSDK_0004.mp4";// ffmpeg_codec.mp4";//
//const char* output_path = "output_original_video2_420.mp4";
const char* output_path = "output_original_video2_420.mp4";

//framesにframeを溜め込む
static void on_frame_decoded2(AVFrame* frame) {
  AVFrame* new_ref = av_frame_alloc();
  av_frame_ref(new_ref, frame);
  frames_stream.push_back(new_ref);
}

void decode_all()
{ 
  // https://stackoverflow.com/questions/58681845/ffmpeg-raw-video-size-parameter 参考に,optionsとrawformatを書く.
  // make codec options　入力を読み取る時のオプションを設定する.
  AVDictionary* codec_options1 = nullptr;
  av_dict_set(&codec_options1, "framerate", "30", 0);
  av_dict_set(&codec_options1, "pixel_format", "yuv420p", 0);
  //av_dict_set(&codec_options1, "pixel_format", "yuyv422", 0);
  av_dict_set(&codec_options1, "video_size", "640x480", 0);
  //av_dict_set(&codec_options1, "video_size", "1920x1080", 0);

  const auto raw_format = av_find_input_format("rawvideo");//rawvideo,video4linux2
  std::cout << raw_format << std::endl;

        if (raw_format == nullptr) {
            printf("Could not find RAW input parser in FFmpeg");
            throw std::runtime_error("RAW not found");
        }
        printf("rawformat: %d\n",raw_format);


  //add end      
  AVFormatContext* format_context = nullptr;
  //if (avformat_open_input(&format_context, input_path, nullptr, &codec_options1) != 0) {
  if (avformat_open_input(&format_context, input_path, raw_format, &codec_options1) != 0) {
    printf("avformat_open_input failed\n");
  }

  //if (avformat_find_stream_info(format_context, nullptr) < 0) {
  if (avformat_find_stream_info(format_context, &codec_options1) < 0) {
    printf("avformat_find_stream_info failed\n");
  }

  AVStream* video_stream = nullptr;
  //for (int i = 0; i < (int)format_context->nb_streams; ++i) {
  for (int i = 0; i <5; ++i) {
    if (format_context->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
      video_stream = format_context->streams[i];
      break;
    }
  }
  if (video_stream == nullptr) {
    printf("No video stream ...\n");
  }
  //フレームのタイムスタンプの単位を取得.
  //time_base = video_stream->time_base;
  printf("codec_id: %d\n",video_stream->codecpar->codec_id);
  AVCodec* codec = avcodec_find_decoder(video_stream->codecpar->codec_id);
  //コーデック名から作ることも可能 ffmpeg -c hogehoge
  //AVCodec* codec = avcodec_find_encoder_by_name("wrapped_avframe");
  if (codec == nullptr) {
    printf("No supported decoder ...\n");
  }

  AVCodecContext* codec_context = avcodec_alloc_context3(codec);
  if (codec_context == nullptr) {
    printf("avcodec_alloc_context3 failed\n");
  }

  if (avcodec_parameters_to_context(codec_context, video_stream->codecpar) < 0) {
    printf("avcodec_parameters_to_context failed\n");
  }

  //if (avcodec_open2(codec_context, codec, nullptr) != 0) {
  if (avcodec_open2(codec_context, codec, &codec_options1) != 0) {
    printf("avcodec_open2 failed\n");
  }

  AVFrame* frame = av_frame_alloc();
  AVPacket packet = AVPacket();
  printf("before read _frame\n");
  int count_ = 0;
  while (av_read_frame(format_context, &packet) == 0) {
    //exit(1);
  //printf("before read _frame2\n");
  count_++;
    if (packet.stream_index == video_stream->index) {
  //printf("before read _frame3\n");

      if (avcodec_send_packet(codec_context, &packet) != 0) {
        printf("avcodec_send_packet failed\n");
      }
      //printf("yaruzo\n");
      while (avcodec_receive_frame(codec_context, frame) == 0) {
        on_frame_decoded(frame);
        //printf("11111111\n");
      //printf("format %d\n",frame->format);

      }
    }
    av_packet_unref(&packet);
    if (count_ > 30){
      break;
    }
  }
  


  // flush decoder
  if (avcodec_send_packet(codec_context, nullptr) != 0) {
    printf("avcodec_send_packet failed");
  }
  while (avcodec_receive_frame(codec_context, frame) == 0) {
    on_frame_decoded(frame);
  }

  av_frame_free(&frame);
  avcodec_free_context(&codec_context);
  avformat_close_input(&format_context);
}

void list_obj_opt(void *obj){
	printf("Output some option info about object:\n");
	printf("Object name:%s\n",(*(AVClass **) obj)->class_name);
	printf("=====================================\n");
  printf("Encoding param:\n");
	av_opt_show2(obj,stderr,AV_OPT_FLAG_ENCODING_PARAM,NULL);
	printf("Video param:\n");
	av_opt_show2(obj,stderr,AV_OPT_FLAG_VIDEO_PARAM,NULL);
	printf("Audio param:\n");
	av_opt_show2(obj,stderr,AV_OPT_FLAG_AUDIO_PARAM,NULL);
	printf("Decoding param:\n");
	av_opt_show2(obj,stderr,AV_OPT_FLAG_DECODING_PARAM,NULL);
	
	printf("====================================\n");
}

int main(int argc, char* argv[])
{
  printf("input_path:: %s \n",input_path);
  av_register_all();
  std::cout << input_path << std::endl;

  //エンコードする時用の情報を取得.
  decode_all();
  printf("decode finish\n");
  AVIOContext* io_context = nullptr;
  if (avio_open(&io_context, output_path, AVIO_FLAG_WRITE) < 0) {
    printf("avio_open failed\n");
  }

  AVFormatContext* format_context = nullptr;
  if (avformat_alloc_output_context2(&format_context, nullptr, "mp4", nullptr) < 0) {
  //if (avformat_alloc_output_context2(&format_context, nullptr, "h264", nullptr) < 0) {
    printf("avformat_alloc_output_context2 failed\n");
  }

  format_context->pb = io_context;

  AVCodec* codec = avcodec_find_encoder(AV_CODEC_ID_H264);
  if (codec == nullptr) {
    printf("encoder not found ...\n");
  }

  AVCodecContext* codec_context = avcodec_alloc_context3(codec);
  if (codec_context == nullptr) {
    printf("avcodec_alloc_context3 failed\n");
  }

  // set picture properties for encoder デコーダで撮った最初のフレームから情報をとって,エンコーダの設定に用いる.
  AVFrame* first_frame = frames[0];
  bool frames_empty = frames.empty();
  printf("frame empty: %d \n",frames_empty); //フレームをデコードできてるか確認.
  codec_context->pix_fmt = (AVPixelFormat)first_frame->format;//0=AV_PIX_FMT_YUV420P
  codec_context->width = first_frame->width;//640
  codec_context->height = first_frame->height;//480
  codec_context->field_order = AV_FIELD_PROGRESSIVE;
  codec_context->color_range = first_frame->color_range;
  codec_context->color_primaries = first_frame->color_primaries;//2:=AVCOL_PRI_UNSPECIFIED
  codec_context->color_trc = first_frame->color_trc;//2
  codec_context->colorspace = first_frame->colorspace;
  codec_context->chroma_sample_location = first_frame->chroma_location;//0:だめそう
  codec_context->sample_aspect_ratio = first_frame->sample_aspect_ratio;//num:0,den:1
  printf("first frame: %d %d\n",first_frame->format,first_frame->height);

  // set timebase
  codec_context->time_base = time_base;

  // generate global header when the format require it
  if (format_context->oformat->flags & AVFMT_GLOBALHEADER) {
    codec_context->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
  }

  // make codec options エンコーダーのオプションを設定をする.ここで出力いじれそう. 参考:https://trac.ffmpeg.org/wiki/Encode/H.264
  AVDictionary* codec_options = nullptr;
  av_dict_set(&codec_options, "preset", "medium", 0);
  //av_dict_set(&codec_options, "pixel_format", "yuyv", 0);
  av_dict_set(&codec_options, "x264-params", "keyint=10:min-keyint=10", 0);
  //av_dict_set(&codec_options, "crf", "22", 0);
  //av_dict_set(&codec_options, "profile", "high422", 0);
  //av_dict_set(&codec_options, "level", "4.0", 0);

  //オプション確認したい.
  //list_obj_opt(codec_context);

  if (avcodec_open2(codec_context, codec_context->codec, &codec_options) != 0) {
    printf("avcodec_open2 failed\n");
  }
  AVStream* stream = avformat_new_stream(format_context, codec);
  if (stream == NULL) {
    printf("avformat_new_stream failed");
  }

  stream->sample_aspect_ratio = codec_context->sample_aspect_ratio;
  stream->time_base = codec_context->time_base;

  if (avcodec_parameters_from_context(stream->codecpar, codec_context) < 0) {
    printf("avcodec_parameters_from_context failed");
  }

  if (avformat_write_header(format_context, nullptr) < 0) {
    printf("avformat_write_header failed\n");
  }


  //#############CV使わずにdecoderを作ってcamをgetする方法#############################
  //const char* input_path = "/dev/video0";// "/home/ubuntu/webcam/PSDK_0004.mp4";// ffmpeg_codec.mp4";//
  
    // optionに関しては, https://stackoverflow.com/questions/58681845/ffmpeg-raw-video-size-parameter を参考に,optionsとraw_formatを書く.
    // make codec options　入力を開く時の設定を決める.
    AVDictionary* codec_options2 = nullptr;
    av_dict_set(&codec_options2, "framerate", "30", 0);
    //av_dict_set(&codec_options2, "pixel_format", "yuyv422", 0);
    av_dict_set(&codec_options2, "pixel_format", "yuv420p", 0);

    av_dict_set(&codec_options2, "video_size", "640x480", 0);
    //av_dict_set(&codec_options2, "video_size", "1920x1080", 0);

    const auto raw_format2 = av_find_input_format("rawvideo"); //rawvideo,video4linux2

    if (raw_format2 == nullptr) {
        printf("Could not find RAW input parser in FFmpeg");
        throw std::runtime_error("RAW not found");
    }
    //printf("rawformat: %d\n",raw_format2);
   
    AVFormatContext* format_context_dec = nullptr;
    //if (avformat_open_input(&format_context_dec, input_path, nullptr,  &codec_options2) != 0) {
    if (avformat_open_input(&format_context_dec, input_path, raw_format2, &codec_options2) != 0) {
      printf("avformat_open_input failed\n");
    }

    //if (avformat_find_stream_info(format_context_dec, nullptr) < 0) {
    if (avformat_find_stream_info(format_context_dec, &codec_options2) < 0) {
      printf("avformat_find_stream_info failed\n");
    }

    AVStream* video_stream_dec = nullptr;
    //for (int i = 0; i < (int)format_context_dec->nb_streams; ++i) {
    for (int i = 0; i <5; ++i) {
      if (format_context_dec->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
        video_stream_dec = format_context_dec->streams[i];
        break;
      }
    }
    if (video_stream_dec == nullptr) {
      printf("No video stream ...\n");
    }
    //フレームのタイムスタンプの単位を取得.
    //time_base = video_stream_dec->time_base;
    //printf("codec_id: %d\n",video_stream_dec->codecpar->codec_id);
    AVCodec* codec_dec = avcodec_find_decoder(video_stream_dec->codecpar->codec_id);
    //コーデック名から作ることも可能 ffmpeg -c hogehoge
    //AVCodec* codec = avcodec_find_encoder_by_name("wrapped_avframe");
    if (codec_dec == nullptr) {
      printf("No supported decoder ...\n");
    }

    AVCodecContext* codec_context_dec = avcodec_alloc_context3(codec_dec);
    if (codec_context_dec == nullptr) {
      printf("avcodec_alloc_context3 failed\n");
    }

    if (avcodec_parameters_to_context(codec_context_dec, video_stream_dec->codecpar) < 0) {
      printf("avcodec_parameters_to_context failed\n");
    }

    //if (avcodec_open2(codec_context, codec, nullptr) != 0) {
    if (avcodec_open2(codec_context_dec, codec_dec, &codec_options2) != 0) {
      printf("avcodec_open2 failed\n");
    }
    
    //############## add end ########################
    
    int frame_count = 0;
    unsigned long dataLength_pct = 0;
    unsigned char *dataBuffer_pct = NULL;
    #define VIDEO_FRAME_AUD_LEN                  6
    static const uint8_t s_frameAudInfo[VIDEO_FRAME_AUD_LEN] = {0x00, 0x00, 0x00, 0x01, 0x09, 0x10};
    unsigned long dataLength = 0;
    
    //AVFrame* new_frame = av_frame_alloc(); //receiveの外でframeにaccessする用.


    //loop for frame_count times START!!!!!!!!
    //ここで動画の長さ決めてる
    int video_length = 10;
    while(frame_count < video_length * output_fps) {
    //while(frames.size() > 0) {
    
      //AVFrame* frame_ = frames.front();
      //frames.pop_front();
      //printf("linesize: %d %d %d\n",frame_->linesize[0],frame_->linesize[1],frame_->linesize[2]);
      //av_frame_free(&frame_);
      

      // ######3.CV使わずにデコーダーでframeを取得したいver.!####
      AVFrame* frame = av_frame_alloc();
      AVPacket packet_dec = AVPacket();
      //format_context_decを定義
      if(av_read_frame(format_context_dec, &packet_dec) == 0) {
        //if (packet.stream_index == video_stream_dec->index) {
          if (avcodec_send_packet(codec_context_dec, &packet_dec) != 0) {
            printf("avcodec_send_packet failed \n");
          }
          while (avcodec_receive_frame(codec_context_dec, frame) == 0) {
            //もしかしたら,on_frame_decodedのようにnew_frameを作る必要があるかも. ->ありました!!!!
            //av_frame_ref(new_frame, frame);
            on_frame_decoded2(frame); //frames_streamにframeをpushする
          //printf("width %d \n",frame->width);
          }
          //dataLength_pct = packet_dec.size;
        //}
        av_packet_unref(&packet_dec);
      }
      
      
      //std::size_t size = frames_stream.size();
      //std::cout << size << std::endl;
      //bool frames_empty = frames_stream.empty();
      //std::cout << std::boolalpha << b << std::endl;
      if (!frames_stream.empty()){
        //上でデコードされたframeを取得する
        AVFrame* new_frame = frames_stream.front();
        frames_stream.pop_front();
        //add end

        printf("frame count: %d\n",frame_count);
        printf("time_base: %d %d\n",time_base.den,time_base.num);
        printf("codec time_base: %d %d\n",codec_context->time_base.den,codec_context->time_base.num);
        //printf("newframe pts: %d\n",new_frame->data[40]);
        // 可変fpsでvideoに合わせる場合
        //int64_t pts = av_frame_get_best_effort_timestamp(new_frame);
        //frame->pts = av_rescale_q(pts, time_base, codec_context->time_base);
        //固定フレームレート
        new_frame->pts = av_rescale_q(frame_count++, time_base, codec_context->time_base);

        new_frame->key_frame = 0;
        new_frame->pict_type = AV_PICTURE_TYPE_NONE; 
        printf("send frame width %d at %d step \n",new_frame->width, frame_count);

        if (avcodec_send_frame(codec_context, new_frame) != 0) {
          printf("avcodec_send_frame failed\n");
        }
        av_frame_free(&frame);
        av_frame_free(&new_frame);
        AVPacket packet = AVPacket();
        while (avcodec_receive_packet(codec_context, &packet) == 0) {
          printf("aaa\n");
          packet.stream_index = 0;
          dataLength_pct = packet.size;
          dataBuffer_pct = (unsigned char*)calloc(dataLength_pct + 10, sizeof(char));//origin dataBuffer_pct = (char*)calloc(dataLength_pct + 10, sizeof(char));
          printf("packet size: %d, data[40] %d  \n", packet.size,packet.data[40]);//data[10]);//これがdatabufferのdatabuff[10]とdata_lengthに相当
          av_packet_rescale_ts(&packet, codec_context->time_base, stream->time_base);
          memcpy(dataBuffer_pct,packet.data,packet.size);
          dataLength = packet.size;
          memcpy(&dataBuffer_pct[packet.size], s_frameAudInfo, VIDEO_FRAME_AUD_LEN); // origin memset(&dataBuffer_pct[packet.size], s_frameAudInfo, VIDEO_FRAME_AUD_LEN);//arg2->1へn文字コピー frameInfo[frameNumber].size==data_lengthでいいのかな?
          dataLength = dataLength + VIDEO_FRAME_AUD_LEN;
          //databuffer 完成.
          if (av_interleaved_write_frame(format_context, &packet) != 0) {
            printf("av_interleaved_write_frame failed\n");
          }
        }
        free(dataBuffer_pct); 
      }
    }

    // flush encoder
    if (avcodec_send_frame(codec_context, nullptr) != 0) {
      printf("avcodec_send_frame failed\n");
    }
    AVPacket packet = AVPacket();
    while (avcodec_receive_packet(codec_context, &packet) == 0) {
      packet.stream_index = 0;
      av_packet_rescale_ts(&packet, codec_context->time_base, stream->time_base);
      //ファイルに保存する
      if (av_interleaved_write_frame(format_context, &packet) != 0) {
        printf("av_interleaved_write_frame failed\n");
      }
    }

    if (av_write_trailer(format_context) != 0) {
      printf("av_write_trailer failed\n");
    }

    avcodec_free_context(&codec_context);
    avformat_free_context(format_context);
    avio_closep(&io_context);

    return 0;
}
