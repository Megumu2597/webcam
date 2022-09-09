#define __STDC_CONSTANT_MACROS
#define __STDC_LIMIT_MACROS
#include <deque>
extern "C" {
#include <libavutil/imgutils.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
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

//AVRational time_base = av_make_q(1001, 30000);
AVRational time_base = av_make_q(1001, 30000);
std::deque<AVFrame*> frames;

static void on_frame_decoded(AVFrame* frame) {
  AVFrame* new_ref = av_frame_alloc();
  av_frame_ref(new_ref, frame);
  frames.push_back(new_ref);
}

std::deque<AVFrame*> frames_stream;

//framesにframeを溜め込む
static void on_frame_decoded2(AVFrame* frame) {
  AVFrame* new_ref = av_frame_alloc();
  av_frame_ref(new_ref, frame);
  frames_stream.push_back(new_ref);
}

void decode_all()
{
  /*
  const char* input_path = "/home/ubuntu/webcam/ffmpeg_codec.mp4";//PSDK_0004.mp4";
  AVFormatContext* format_context = nullptr;
  if (avformat_open_input(&format_context, input_path, nullptr, nullptr) != 0) {
    printf("avformat_open_input failed\n");
  }

  if (avformat_find_stream_info(format_context, nullptr) < 0) {
    printf("avformat_find_stream_info failed\n");
  }

  AVStream* video_stream = nullptr;
  for (int i = 0; i < (int)format_context->nb_streams; ++i) {
    if (format_context->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
      video_stream = format_context->streams[i];
      break;
    }
  }
  if (video_stream == nullptr) {
    printf("No video stream ...\n");
  }

  //time_base = video_stream->time_base;

  AVCodec* codec = avcodec_find_decoder(video_stream->codecpar->codec_id);
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

  if (avcodec_open2(codec_context, codec, nullptr) != 0) {
    printf("avcodec_open2 failed\n");
  }

  AVFrame* frame = av_frame_alloc();
  AVPacket packet = AVPacket();

  while (av_read_frame(format_context, &packet) == 0) {
    if (packet.stream_index == video_stream->index) {
      if (avcodec_send_packet(codec_context, &packet) != 0) {
        printf("avcodec_send_packet failed\n");
      }
      while (avcodec_receive_frame(codec_context, frame) == 0) {
        on_frame_decoded(frame);
      }
    }
    av_packet_unref(&packet);
  }
  */  
 const char* input_path = "/dev/video2";  // "/home/ubuntu/webcam/PSDK_0004.mp4";// ffmpeg_codec.mp4";//
  
  // https://stackoverflow.com/questions/58681845/ffmpeg-raw-video-size-parameter 参考に,optionsとraw_formatを書く.
  // make codec options
  AVDictionary* codec_options1 = nullptr;
  av_dict_set(&codec_options1, "framerate", "30", 0);
  //av_dict_set(&codec_options1, "pixel_format", "yuyv422", 0);
  av_dict_set(&codec_options1, "video_size", "640x480", 0);

  const auto raw_format = av_find_input_format("rawvideo");
        if (raw_format == nullptr) {
            printf("Could not find RAW input parser in FFmpeg");
            throw std::runtime_error("RAW not found");
        }
        printf("rawformat: %d\n",raw_format);


  //add end      
  AVFormatContext* format_context = nullptr;
  //if (avformat_open_input(&format_context, input_path, nullptr, nullptr) != 0) {
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

int main(int argc, char* argv[])
{
  av_register_all();

  decode_all();
  printf("decode finish\n");
  const char* output_path = "output_original4.mp4";
  AVIOContext* io_context = nullptr;
  if (avio_open(&io_context, output_path, AVIO_FLAG_WRITE) < 0) {
    printf("avio_open failed\n");
  }

  AVFormatContext* format_context = nullptr;
  if (avformat_alloc_output_context2(&format_context, nullptr, "mp4", nullptr) < 0) {
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

  // set picture properties
  AVFrame* first_frame = frames[0];
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
  printf("first frame: %d %d\n",first_frame->sample_aspect_ratio.num,first_frame->sample_aspect_ratio.den);

  // set timebase
  codec_context->time_base = time_base;

  // generate global header when the format require it
  if (format_context->oformat->flags & AVFMT_GLOBALHEADER) {
    codec_context->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
  }

  // make codec options
  AVDictionary* codec_options = nullptr;
  av_dict_set(&codec_options, "preset", "medium", 0);
  //av_dict_set(&codec_options, "pixel_format", "yuyv", 0);
  //av_dict_set(&codec_options, "crf", "22", 0);
  //av_dict_set(&codec_options, "profile", "high422", 0);
  //av_dict_set(&codec_options, "level", "4.0", 0);

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


  //#############CV使わずにcamをgetする方法#############################
  const char* input_path = "/dev/video2";// "/home/ubuntu/webcam/PSDK_0004.mp4";// ffmpeg_codec.mp4";//
  
    // https://stackoverflow.com/questions/58681845/ffmpeg-raw-video-size-parameter 参考に,optionsとraw_formatを書く.
    // make codec options
    AVDictionary* codec_options2 = nullptr;
    av_dict_set(&codec_options2, "framerate", "30", 0);
    //av_dict_set(&codec_options2, "pixel_format", "yuyv422", 0);

    av_dict_set(&codec_options2, "video_size", "640x480", 0);

    const auto raw_format2 = av_find_input_format("rawvideo");
    if (raw_format2 == nullptr) {
        printf("Could not find RAW input parser in FFmpeg");
        throw std::runtime_error("RAW not found");
    }
    //printf("rawformat: %d\n",raw_format2);
   
    AVFormatContext* format_context_cam = nullptr;
    //if (avformat_open_input(&format_context_cam, input_path, nullptr, nullptr) != 0) {
    if (avformat_open_input(&format_context_cam, input_path, raw_format2, &codec_options2) != 0) {
      printf("avformat_open_input failed\n");
    }

    //if (avformat_find_stream_info(format_context_cam, nullptr) < 0) {
    if (avformat_find_stream_info(format_context_cam, &codec_options2) < 0) {
      printf("avformat_find_stream_info failed\n");
    }

    AVStream* video_stream_cam = nullptr;
    //for (int i = 0; i < (int)format_context_cam->nb_streams; ++i) {
    for (int i = 0; i <5; ++i) {
      if (format_context_cam->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
        video_stream_cam = format_context_cam->streams[i];
        break;
      }
    }
    if (video_stream_cam == nullptr) {
      printf("No video stream ...\n");
    }
    //フレームのタイムスタンプの単位を取得.
    //time_base = video_stream_cam->time_base;
    //printf("codec_id: %d\n",video_stream_cam->codecpar->codec_id);
    AVCodec* codec_cam = avcodec_find_decoder(video_stream_cam->codecpar->codec_id);
    //コーデック名から作ることも可能 ffmpeg -c hogehoge
    //AVCodec* codec = avcodec_find_encoder_by_name("wrapped_avframe");
    if (codec_cam == nullptr) {
      printf("No supported decoder ...\n");
    }

    AVCodecContext* codec_context_cam = avcodec_alloc_context3(codec_cam);
    if (codec_context_cam == nullptr) {
      printf("avcodec_alloc_context3 failed\n");
    }

    if (avcodec_parameters_to_context(codec_context_cam, video_stream_cam->codecpar) < 0) {
      printf("avcodec_parameters_to_context failed\n");
    }

    //if (avcodec_open2(codec_context, codec, nullptr) != 0) {
    if (avcodec_open2(codec_context_cam, codec_cam, &codec_options2) != 0) {
      printf("avcodec_open2 failed\n");
    }
    
    //############## add end ########################
    
    int frame_count = 0;
    //AVFrame* new_frame = av_frame_alloc(); //receiveの外でframeにaccessする用.
    while(frame_count <100) {
    //while(frames.size() > 0) {
    
    //AVFrame* frame_ = frames.front();
    //frames.pop_front();
    
     // ######3.CV使わずにframeを取得したいver.!####
    AVFrame* frame = av_frame_alloc();
    AVPacket packet_cam = AVPacket();
    //format_context_camを定義
    if(av_read_frame(format_context_cam, &packet_cam) == 0) {
      //if (packet.stream_index == video_stream_cam->index) {
        if (avcodec_send_packet(codec_context_cam, &packet_cam) != 0) {
          printf("avcodec_send_packet failed \n");
        }
        while (avcodec_receive_frame(codec_context_cam, frame) == 0) {
          //もしかしたら,on_frame_decodedのようにnew_frameを作る必要があるかも. ->ありました!!!!
          //av_frame_ref(new_frame, frame);
          on_frame_decoded2(frame); //frames_streamにframeをpushする
         //printf("width %d \n",frame->width);
        }
        //dataLength_pct = packet_cam.size;
      //}
      av_packet_unref(&packet_cam);
    }
    //printf("after width %d \n",new_frame->width);
    
    //add end
    AVFrame* new_frame = frames_stream.front();
    frames_stream.pop_front();

    // 可変fpsでvideoに合わせる場合
    //int64_t pts = av_frame_get_best_effort_timestamp(frame);
    //frame->pts = av_rescale_q(pts, time_base, codec_context->time_base);
    //固定フレームレート
    new_frame->pts = av_rescale_q(frame_count++, time_base, codec_context->time_base);

    new_frame->key_frame = 0;
    new_frame->pict_type = AV_PICTURE_TYPE_NONE; 
    
    /*
    //strideを設定. frame.popで確認したら640,320,320,0...だった
    //色々設定されてなかった(decode()と同じことしてるのに謎）ので,設定する.
    frame->linesize[0] = frame_->width;
    frame->linesize[1] = frame_->width /2;
    frame->linesize[2] = frame_->width /2;
    //for (int k=0; k<8; ++k){printf("stride%d: %d\n",k,frame->linesize[k]);}
    frame->width = frame_->width; //640   //1280 これが原因でavcodec_send failsedしてる!!
    frame->height = frame_->height; //480  //720
    frame->format = (AVPixelFormat)frame_->format; //ffmpeg_codec.mp4では4:=AV_PIX_FMT_YUV422P //PSDK_0004.mp4では,0:=AV_PIX_FMT_YUV420P // /dev/video2には1:=AV_PIX_FMT_YUYV422 
    //frame->field_order = AV_FIELD_PROGRESSIVE;
    frame->color_range =  frame_->color_range; //0 //0:=AVCOL_RANGE_UNSPECIFIED;//
    frame->color_primaries = frame_->color_primaries; //2 :=AVCOL_PRI_UNSPECIFIED //0:=AVCOL_PRI_RESERVED0 
    frame->color_trc = frame_->color_trc;//2 //2:=AVCOL_TRC_UNSPECIFIED;//
    frame->colorspace = frame_->colorspace; //2 //2:=AVCOL_SPC_UNSPECIFIED;//
    frame->chroma_location =  frame_->chroma_location; //frames[0]->1 (av_image_alloc->0) //1:=AVCHROMA_LOC_LEFT;//
    frame->sample_aspect_ratio = frame_->sample_aspect_ratio; */
    


    if (avcodec_send_frame(codec_context, new_frame) != 0) {
      printf("avcodec_send_frame failed\n");
    }
    av_frame_free(&frame);
    av_frame_free(&new_frame);
    AVPacket packet = AVPacket();
    while (avcodec_receive_packet(codec_context, &packet) == 0) {
      packet.stream_index = 0;
      printf("packet1: %d %d  \n", packet.size,packet.data[40]);//data[10]);//これがdatabufferのdatabuff[10]とdata_lengthに相当

      av_packet_rescale_ts(&packet, codec_context->time_base, stream->time_base);
      if (av_interleaved_write_frame(format_context, &packet) != 0) {
        printf("av_interleaved_write_frame failed\n");
      }
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