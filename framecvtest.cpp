
#define VIDEO_FRAME_AUD_LEN                  6
char *dataBuffer = NULL;
FILE *fpFile = NULL;
static const uint8_t s_frameAudInfo[VIDEO_FRAME_AUD_LEN] = {0x00, 0x00, 0x00, 0x01, 0x09, 0x10};

fpFile = fopen(transcodedFilePath, "rb+"); //バイナリモードで読み込みと書き込み
ret = fseek(fpFile, frameInfo[frameNumber].positionInFile, SEEK_SET); //先頭からoffsetだけポインタ移動,
frameBufSize = frameInfo[frameNumber].size; //たぶん4000くらい.
dataBuffer = calloc(frameBufSize, 1); 

dataLength = fread(dataBuffer, 1, frameInfo[frameNumber].size, fpFile);
if (videoStreamType == DJI_CAMERA_VIDEO_STREAM_TYPE_H264_DJI_FORMAT) {
    memcpy(&dataBuffer[frameInfo[frameNumber].size], s_frameAudInfo, VIDEO_FRAME_AUD_LEN); //arg2->1へn文字コピー
    dataLength = dataLength + VIDEO_FRAME_AUD_LEN;//基本6足してる.
}


//cv::mat -> char arrayに変換するコード書く.
