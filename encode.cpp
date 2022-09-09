#define __STDC_CONSTANT_MACROS
#define __STDC_LIMIT_MACROS

#include <deque>
extern "C" {
#include <libavutil/imgutils.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>

}

#include <vector>
#include <stdio.h>
#include <string.h>
#include <iostream>
#include <unistd.h>

#pragma comment(lib, "avutil.lib")
#pragma comment(lib, "avcodec.lib")
#pragma comment(lib, "avformat.lib")

//ficstarの(3)エンコード https://proc-cpuinfo.fixstars.com/2017/09/ffmpeg-api-encode/
// g++ -o encode_integ encode.cpp -lavformat  -lavcodec -lavutil -lswscale `pkg-config opencv4 --cflags --libs`
//AVRational time_base;
std::deque<AVFrame*> frames;

//framesにframeを溜め込む
static void on_frame_decoded(AVFrame* frame) {
  AVFrame* new_ref = av_frame_alloc();
  av_frame_ref(new_ref, frame);
  frames.push_back(new_ref);
}
//固定フレームレート化
AVRational time_base = av_make_q(1001, 30000); //30000/1001=29.97fpsに設定.

//void set_format_conetxt(input_path,)
void decode_all()
{
  const char* input_path = "/dev/video2";// "/home/ubuntu/webcam/PSDK_0004.mp4";// ffmpeg_codec.mp4";//
  
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
    if (count_ > 100){
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

// ファイル出力の代わりに呼ばれる
int write_packet(void *opaque, uint8_t *buf, int buf_size) {
  // do something なんもしなくてもいいかも.(ここまでこないので)
  /*DWORD bytesWritten = 0;
  if (WriteFile(writeHandle, buf, buf_size, &bytesWritten, nullptr) == 0) {
    printf("failed to write to stdin pipe\n");
  }*/
    //printf("packet22: %d %d  \n", buf_size,buffer[40]);//これがdatabufferのdatabuff[10]とdata_lengthに相当

  return 0;
}

int main()
{
  av_register_all();

  //全フレームをデコードしてframesに入れてるけど,実装はwhileで1フレームずつsend,receiveしてdecode.
  decode_all(); 
  printf("#############finish decode###############3\n");
  //CVから受け取る用.
  /*
  cv::VideoCapture cam("/dev/video2");
  cam.set(cv::CAP_PROP_FPS, 30); // set fps before set fourcc
  cam.set(cv::CAP_PROP_FOURCC, cv::VideoWriter::fourcc('H', '2', '6', '4'));
  if (cam.isOpened()) printf("camera successfully opened\n");
  else               { printf("cannot open camera\n"); return 1; }
  cv::namedWindow("image0", cv::WINDOW_AUTOSIZE);
  cv::Mat image;*/

  //ファイル出力
  const char* output_path = "output_encodecppvid2.mp4";
  AVIOContext* io_context = nullptr;
  if (avio_open(&io_context, output_path, AVIO_FLAG_WRITE) < 0) {
    printf("avio_open failed\n");
  }
  //ファイル出力しないでプログラムで受け取る
  /*int bufsize= 16 * 1024;
  unsigned char* buffer = (unsigned char*)av_malloc(bufsize);
  AVIOContext* io_context = avio_alloc_context(
      buffer, bufsize, 1, nullptr, nullptr, write_packet, nullptr);
*/
//出力したいフォーマット
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
  //get first frame for info
  /*
  cam >> image;
  AVFrame* first_frame;
  int width_ = image.cols;
  int height_ = image.rows;
  first_frame = av_frame_alloc();
  av_image_alloc(first_frame->data, first_frame->linesize, width_, height_,
                  AVPixelFormat::AV_PIX_FMT_YUV420P, 1); //これすると,frame->width,heightが0になる.
  */
  //%dとenumの内容の照合はこれでやる. https://ffmpeg.org/doxygen/trunk/pixfmt_8h_source.html
  AVFrame* first_frame = frames[0];
  //printf("format %d\n",first_frame->format); ffmpeg_codec.mp4 v.s. psdk_0004.mp4
  codec_context->width = first_frame->width; //640   //1280 これが原因でavcodec_send failsedしてる!!
  codec_context->height = first_frame->height; //480  //720
  codec_context->pix_fmt = (AVPixelFormat)first_frame->format; //ffmpeg_codec.mp4では4:=AV_PIX_FMT_YUV422P //PSDK_0004.mp4では,0:=AV_PIX_FMT_YUV420P // /dev/video2には1:=AV_PIX_FMT_YUYV422 
  codec_context->field_order = AV_FIELD_PROGRESSIVE;
  codec_context->color_range =  first_frame->color_range; //0 //0:=AVCOL_RANGE_UNSPECIFIED;//
  //enum AVColorPrimaries col_pri;
  //col_pri = AVCOL_PRI_UNSPECIFIED;
  codec_context->color_primaries = first_frame->color_primaries; //2 :=AVCOL_PRI_UNSPECIFIED //0:=AVCOL_PRI_RESERVED0 
  codec_context->color_trc = first_frame->color_trc;//2 //2:=AVCOL_TRC_UNSPECIFIED;//
  codec_context->colorspace = first_frame->colorspace; //2 //2:=AVCOL_SPC_UNSPECIFIED;//
  codec_context->chroma_sample_location =  first_frame->chroma_location; //frames[0]->1 (av_image_alloc->0) //1:=AVCHROMA_LOC_LEFT;//
  codec_context->sample_aspect_ratio = first_frame->sample_aspect_ratio; //.numと.denでprint num:0,den:1  //psdkだとnum:1,den:1
  ///codec_context->sample_aspect_ratio.num = 1;
  //std::cout << "codec_context properties" << codec_context->pix_fmt << codec_context->width << codec_context->color_range << codec_context->color_primaries << codec_context->color_trc <<  codec_context->colorspace << codec_context->chroma_sample_location << codec_context->sample_aspect_ratio <<"\n";
  //printf("asdfasdfasdfasfa %d \n",codec_context->pix_fmt);
  // set timebase
  codec_context->time_base = time_base;
  // generate global header when the format require it
  if (format_context->oformat->flags & AVFMT_GLOBALHEADER) {
    codec_context->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
    printf("flags on!\n");
  }

  // make codec options
  AVDictionary* codec_options = nullptr;
  av_dict_set(&codec_options, "preset", "medium", 0);
  av_dict_set(&codec_options, "video_size", "640x480", 0);
  
  /*
  av_dict_set(&codec_options, "crf", "22", 0);
  av_dict_set(&codec_options, "profile", "high444", 0);
  av_dict_set(&codec_options, "level", "4.0", 0);
  //for streaming
  // av_dict_set(&codec_options, "rtsp_transport", "tcp", 0);
  av_dict_set(&codec_options, "analyzeduration", "30M", 0);
  av_dict_set(&codec_options, "probesize", "30M", 0);
  */
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
  //固定フレームレート用
  int frame_count = 0;
  
  unsigned long dataLength_pct = 0;
  unsigned char *dataBuffer_pct = NULL;
  #define VIDEO_FRAME_AUD_LEN                  6
  static const uint8_t s_frameAudInfo[VIDEO_FRAME_AUD_LEN] = {0x00, 0x00, 0x00, 0x01, 0x09, 0x10};
  unsigned long dataLength = 0;
/*
  //CV使わずにcamをgetする方法
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
    //add end
    */

  int count = 0;
  //frameごとのループ処理スタート!!
  while(count < 200) {
    count++;
    //1.動画からver: 1フレーム分を取り出す
    AVFrame* frame = frames.front();
    frames.pop_front();
    
    //2.streamからCVで取得するver: cv::mat -> Avframeに変換 
    // https://gist.github.com/foowaa/1d296a9dee81c7a2a52f291c95e55680
    //auto start = std::chrono::system_clock::now();
    /*
    cam >> image;
    AVFrame* frame;
    int width = image.cols;
    int height = image.rows;
    int cvLinesizes[1];
    cvLinesizes[0] = image.step1();
    frame = av_frame_alloc();
    //printf("frame->pict_type : %d\n",frame->pict_type);
    av_image_alloc(frame->data, frame->linesize, width, height,
                   codec_context->pix_fmt,1);
    */
                  // AVPixelFormat::AV_PIX_FMT_YUV420P, 1);
    //auto end = std::chrono::system_clock::now();
    //std::chrono::duration<double> diff = end-start;
    //std::cout << "Time to process last frame (seconds): " << diff.count() << " FPS: " << 1.0 / diff.count() << "\n";
   
   /*
    SwsContext *conversion = sws_getContext(
     width, height, AVPixelFormat::AV_PIX_FMT_YUYV422, width, height,//AV_PIX_FMT_BGR24
        (AVPixelFormat)frame->format, SWS_FAST_BILINEAR, NULL, NULL, NULL);
    sws_scale(conversion, &image.data, cvLinesizes, 0, height, frame->data,frame->linesize);
    sws_freeContext(conversion);
    */

   // ######3.CV使わずにframeを取得したいver.!#####
   /*
    AVFrame* frame = av_frame_alloc();
    AVPacket packet_cam = AVPacket();
    //format_context_camを定義
    if(av_read_frame(format_context_cam, &packet_cam) == 0) {
      //if (packet.stream_index == video_stream_cam->index) {
        if (avcodec_send_packet(codec_context_cam, &packet_cam) != 0) {
          printf("avcodec_send_packet failed\n");
        }
        while (avcodec_receive_frame(codec_context_cam, frame) == 0) {
            //もしかしたら,on_frame_decodedのようにnew_frameを作る必要があるかも.
        
        }
        //dataLength_pct = packet_cam.size;
      //}
      av_packet_unref(&packet_cam);
    }
    printf("frame access: %d \n",frame->data[10]);
    */


    //可変フレームレート(videoのfpsに合わせる場合)
    int64_t pts = av_frame_get_best_effort_timestamp(frame);
    frame->pts = av_rescale_q(pts, time_base, codec_context->time_base);
    //固定フレームレート
    //frame->pts = av_rescale_q(frame_count++, time_base, codec_context->time_base);
    frame->key_frame = 0;
    frame->pict_type = AV_PICTURE_TYPE_NONE;
    //printf("before send frame %d %d\n",frame->data[10],frame->pts);

    if (avcodec_send_frame(codec_context, frame) != 0) {
      printf("avcodec_send_frame failed\n");
    }
    av_frame_free(&frame);
    AVPacket packet = AVPacket();
    //エンコーダからパケットを受け取る.
    printf("start receive %d\n",frame_count);
    while (avcodec_receive_packet(codec_context, &packet) == 0) {
      packet.stream_index = 0;
      printf("receive\n");
      dataLength_pct = packet.size;
    dataBuffer_pct = (char*)calloc(dataLength_pct + 10, sizeof(char));
      printf("packet1: %d %d  \n", packet.size,packet.data[10]);//data[10]);//これがdatabufferのdatabuff[10]とdata_lengthに相当
      av_packet_rescale_ts(&packet, codec_context->time_base, stream->time_base);
    memcpy(dataBuffer_pct,packet.data,packet.size);
    dataLength = packet.size;
      
          
    memset(&dataBuffer_pct[packet.size], s_frameAudInfo, VIDEO_FRAME_AUD_LEN);//arg2->1へn文字コピー frameInfo[frameNumber].size==data_lengthでいいのかな?
      dataLength = dataLength + VIDEO_FRAME_AUD_LEN;
        //printf("databuff_pct:%ld,  %d \n", dataLength, dataBuffer_pct[40]); 
    if (av_interleaved_write_frame(format_context, &packet) != 0) {//以後packetの所有権が取られるからアクセス不可.
        printf("av_interleaved_write_frame failed\n");
        
      }

    }
    

    //printf("packet22: %d %d  \n", packet.size,packet.data[40]);//これがdatabufferのdatabuff[10]とdata_lengthに相当
    //std::copy(packet.data[0],packet.data[packet.size],dataBuffer_pct[0]);
    //dataBuffer_pct = (char*)packet.data;
    //av_free_packet(&packet);
    //av_packet_unref(&packet);
    free(dataBuffer_pct); 
    //dataBuffer_pct = NULL;

    //usleep(0.1 * 1000000);
    //printf("receive_finish\n");//whileといいつつも繰り返してはいない.
  }

  // flush encoder: エンコーダーに残ったframeを捨てる.
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
  //ファイル出力
  //avio_closep(&io_context);
  //プログラム受け取り
  av_freep(&io_context->buffer);
  av_freep(&io_context);  
  return 0;
}