extern "C" {
#include <stdio.h>
#include <stdlib.h>
}

// g++ -o readh264 readh264.cpp 
int main(int argc,char *argv[])
{
	FILE *fp;
	//int outbf[10] = {1,2,3,4,5,6,7,8,9,10};
	int inbuf[1024];
	//int i;
	char *dataBuffer = NULL;


	/* バイナリ書き込み読み込みモードでファイルをオープン */
	//if((fp = fopen("/home/ubuntu/PSDK/samples/sample_c/module_sample/camera_emu/media_file/PSDK_0005.h264", "rb+")) == NULL ) {
	if((fp = fopen("/home/ubuntu/webcam/ffmpeg_codec.h264", "rb+")) == NULL ) {
		printf("ファイルオープンエラー\n");
		exit(EXIT_FAILURE);
	}

	/* ファイルにデータを書き込み */
	//fwrite(outbf, sizeof(int), 10, fp);

	/* ファイルの先頭に移動 */
	fseek(fp, 0L, SEEK_SET);

	dataBuffer = (char*)calloc(1024, 1); //void*->char*にキャスト
	fread(dataBuffer,1,1024,fp);
	//fread(inbuf, 1, 1024, fp);
	/* 書き込んだデータを読み込んでみる */

	/* ファイルクローズ */
	fclose(fp);

	/* 読み込みデータの確認 */
	for (int i=0; i<10; i++)
		//printf("%d\n",inbuf[i]); //こっちだと,めちゃでかいし,[0:2]で値変化する
		printf("%d\n",dataBuffer[i]); //こっちだと,255以下でcam_emuと同じもので、[0:2]で値変化しない.
	printf("\n");

	return 0;
}



/*

char *dataBuffer;
FILE *fpFile;

fpFile = fopen("/home/ubuntu/PSDK/samples/sample_c/module_sample/camera_emu/media_file/PSDK_0005.h264", "rb+");
dataBuffer = calloc(2048, 1); 
dataLength = fread(dataBuffer, 1, 2048, fpFile);
printf(databuffer[1]);*/