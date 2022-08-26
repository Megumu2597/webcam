#include <opencv2/highgui/highgui.hpp>
#include <vector>
#include <stdio.h>
#include <string.h>
#include <iostream>

// g++ -o camera_to_frame_test camera.cpp -O3 -lm `pkg-config opencv4 --cflags --libs`
// ./camera_to_frame_test
int main(int argc, char** argv) {
  //cv::VideoCapture cam("/dev/video2");
  cv::VideoCapture cam("/home/ubuntu/PSDK/samples/sample_c/module_sample/camera_emu/media_file/PSDK_0005.h264");
  //
  //cv::VideoCapture cam;
  //cam.open(2,cv::CAP_ANY);
  //cam.set(cv::CAP_PROP_BUFFERSIZE, 1);
cam.set(cv::CAP_PROP_FPS, 20); // set fps before set fourcc
cam.set(cv::CAP_PROP_FOURCC, cv::VideoWriter::fourcc('H', '2', '6', '4'));
if (cam.isOpened()) printf("camera successfully opened\n");
else               { printf("cannot open camera\n"); return 1; }
cv::namedWindow("image0", cv::WINDOW_AUTOSIZE);
cv::Mat mat;
//char *dataBuffer = NULL;
char array2[4000];
//unsigned char uarray[1228800];
//unsigned char *p;
//p = &uarray[0];
std::vector<uchar> array(mat.rows*mat.cols*mat.channels());

for ( ; ; ) {
  int cou=0;
  int tmp = 0;

  auto start = std::chrono::system_clock::now();
  cam >> mat;
  printf("arraysize %d,%d,%d,%d\n",mat.rows,mat.cols,mat.channels(),mat.rows*mat.cols*mat.channels()); //921600
  //これで一応できる
  /*uchar **array = new uchar*[mat.rows];
  for (int i=0; i<mat.rows; ++i)
      array[i] = new uchar[mat.cols];

  for (int i=0; i<mat.rows; ++i)
      array[i] = mat.ptr<uchar>(i);
  */
  //std::vector<uchar> array(mat.rows*mat.cols);
  //unsigned char* array;
  if (mat.isContinuous()){
    //printf("\n continuous");
    //int *uarray = (int*) mat.data;
    unsigned char *uarray = mat.data;
    //array = mat.data;
    //std::cout << "before p[0]:" << p[0] << p[10] <<  "\n";
    //p = mat.data;
    //std::cout << "after p[0]:" << p[0] << p[10] << "\n";

    //memcpy(array2,uarray,4000);
    for (size_t i=2764795;i<2764805;i++){
      cou= cou+1;
      tmp = mat.data[i];
      printf("%ld: %d\n",i,mat.data[i]);
      //std::cout << i << "data:" << mat.data[i] << "\n";
      
    }
    size_t n = sizeof(uarray)/sizeof(uarray[0]);//
    //size_t n = (&uarray)[1] - uarray;
    printf("size: %ld\n",n); //sizeof(mat.data)/sizeof(mat.data[0]));
    //printf("array[120]: %d\n",mat.data[0]);

    

    //fpsを計算
    /*auto end = std::chrono::system_clock::now();
    std::chrono::duration<double> diff = end-start;
    std::cout << "Time to process last frame (seconds): " << diff.count() << " FPS: " << 1.0 / diff.count() << "\n";
    */
  //printf("%d%d",mat.rows,mat.cols);//1280x960=1228800 *3 = 3686400

  }
  
    //unsigned char *buffer = mat.data;
  //printf("%s\n",array[10]);
  //cv::imshow("image0", mat);
  //if (cv::waitKey(30) >= 0) break;
}
  
  
  

  return 0;

}
