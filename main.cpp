#include <libfreenect2/libfreenect2.hpp>
#include <libfreenect2/frame_listener_impl.h>
#include <libfreenect2/registration.h>
#include <libfreenect2/packet_pipeline.h>
#include <libfreenect2/logger.h>
#include "opencv2/objdetect.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/imgproc.hpp"
#include <Python.h>

#include <iostream>
#include <stdio.h>

using namespace std;
using namespace cv;


/** Function Headers */
void detectAndDisplay( Mat frame );

/** Global variables */
String face_cascade_name = "/home/ivanfzh/Desktop/cascade/haarcascade_frontalface_alt.xml";
String eyes_cascade_name = "/home/ivanfzh/Desktop/cascade/haarcascade_eye_tree_eyeglasses.xml";
CascadeClassifier face_cascade;
CascadeClassifier eyes_cascade;
String window_name = "Capture - Face detection";

#include "util.h"
#define MIN(a, b) ((a)<(b)?(a):(b))

//#define TEST
#define RGB
//#define IR
//#define DEPTH
//#define RECOGNITION
//#define AVOID

int usepy(){
        Py_Initialize();    // 初始化

        // 将Python工作路径切换到待调用模块所在目录，一定要保证路径名的正确性
        //string path = "~/add";
        //string chdir_cmd = string("sys.path.append(\'/home/ivanfzh/Desktop/test\')");
        string chdir_cmd = string("sys.path.append(\'/home/ivanfzh/Desktop/CNNGestureRecognizer\')");
        const char* cstr_cmd = chdir_cmd.c_str();
        cout << "path: " << cstr_cmd << endl;
        PyRun_SimpleString("import sys");
        PyRun_SimpleString(cstr_cmd);

        // 加载模块
        PyObject* moduleName = PyString_FromString("trackgesture"); //模块名，不是文件名
        PyObject* pModule = PyImport_Import(moduleName);
        PyErr_Print();
        if (!pModule) // 加载模块失败
        {
            cout << "[ERROR] Python get module failed." << endl;
            return 0;
        }
        cout << "[INFO] Python get module succeed." << endl;

        // 加载函数
        PyObject* pv = PyObject_GetAttrString(pModule, "Main");
        PyErr_Print();
        if (!pv || !PyCallable_Check(pv))  // 验证是否加载成功
        {
            cout << "[ERROR] Can't find funftion (test_add)" << endl;
            return 0;
        }
        cout << "[INFO] Get function (test_add) succeed!!!" << endl;

        // 设置参数
        PyObject* args = PyTuple_New(2);   // 2个参数
        PyObject* arg1 = PyInt_FromLong(4);    // 参数一设为4
        PyObject* arg2 = PyInt_FromLong(3);    // 参数二设为3
        PyObject* arglist = PyList_New(2);
        PyList_SetItem(arglist,0,arg1);
        PyList_SetItem(arglist,1,arg2);
        PyTuple_SetItem(args, 0, arg1);
        PyTuple_SetItem(args, 1, arglist);

        // 调用函数
        PyObject* pRet = PyObject_CallObject(pv,NULL);

        // 获取参数
        if (pRet)  // 验证是否调用成功
        {
            long result = PyInt_AsLong(pRet);
            cout << "result:" << result << endl;
        }

        Py_Finalize();
        return 0;
}

#ifndef TEST
int main(int argc , char** argv)
{
    usepy();
    return 0;
    /******************************************************************************/
    //launch kinect

    libfreenect2::Freenect2 freenect2;
    libfreenect2::Freenect2Device *dev = 0;
    libfreenect2::PacketPipeline *pipeline = 0;

    if (freenect2.enumerateDevices() == 0)
    {
        std::cout << "no device connected!" << std::endl;
        return -1;
    }

    std::string serial = "";
    bool enable_rgb = true;
    bool enable_depth = true;

    if (serial == "")
    {
        serial = freenect2.getDefaultDeviceSerialNumber();
    }

    pipeline = new libfreenect2::CpuPacketPipeline();
    dev = freenect2.openDevice(serial , pipeline);

    int type = libfreenect2::Frame::Color | libfreenect2::Frame::Ir | libfreenect2::Frame::Depth;
    libfreenect2::SyncMultiFrameListener listener(type);
    libfreenect2::FrameMap frames;

    dev->setColorFrameListener(&listener);
    dev->setIrAndDepthFrameListener(&listener);


    if (!dev->start())
        return -1;
    libfreenect2::Freenect2Device::ColorCameraParams camera_params = dev->getColorCameraParams();

    while (true){
        if (!listener.waitForNewFrame(frames , 30 * 1000)) {
                    std::cout << "timeout!" << std::endl;
                    break;
        }
        libfreenect2::Frame *rgb = frames[libfreenect2::Frame::Color];
        libfreenect2::Frame *ir = frames[libfreenect2::Frame::Ir];
        libfreenect2::Frame* depth = frames[libfreenect2::Frame::Depth];

#ifdef RGB
        unsigned char* rgbdata = rgb->data;
        CvSize size;
        size.height = 1080; size.width = 1920;

        char* BGRData = (char*)malloc(size.height*size.width*sizeof(char));
        for (int i = 0; i != size.height; ++i){
            for (int j = 0; j != size.width; ++j){
                int idx = i*size.width+j;
                char B = rgbdata[idx * 4];
                char G = rgbdata[idx * 4 + 1];
                char R = rgbdata[idx * 4 + 2];

                int h = getH(char2int(R),char2int(G),char2int(B));
                float s = getS(char2int(R),char2int(G),char2int(B));
                float grey = char2int(R)*0.299 + char2int(G)*0.587 + char2int(B)*0.114;

                if ((abs(h-15)<15 || abs(h-345)<15) && s > 0.6)
                    BGRData[idx] = 255;
                else
                    BGRData[idx] = 0;

                BGRData[idx]=grey;
            }
        }

        IplImage* rgbimage = cvCreateImage(size,IPL_DEPTH_8U,1);

        cvSetData(rgbimage,BGRData, size.width);

        //cvShowImage("Red Channel",rgbimage);
        cvWaitKey(100);
        Mat rgbmat;
        cv::Mat(rgb->height, rgb->width, CV_8UC4, rgb->data).copyTo(rgbmat);
        //-- 1. Load the cascades
        if( !face_cascade.load( face_cascade_name ) ){ printf("--(!)Error loading face cascade\n"); return -1; };
        if( !eyes_cascade.load( eyes_cascade_name ) ){ printf("--(!)Error loading eyes cascade\n"); return -1; };


        //-- 3. Apply the classifier to the frame
        detectAndDisplay( rgbmat );

        int c = waitKey(10);
        if( (char)c == 27 ) { break; } // escape


#endif

#ifdef IR
        unsigned char* irdata = ir->data;
        CvSize size;
        size.height = 424; size.width = 512;
        IplImage* irimage = cvCreateImage(size,IPL_DEPTH_8U,1);
        char* IRData = (char*)malloc(size.height*size.width*sizeof(char));

        for (int i = 0; i != size.height; ++i){
            for (int j = 0; j != size.width; ++j){
                int idx = i*size.width + j;
                int tmp = round(char2float(&(irdata[idx*4])) / 256.0);
                IRData[idx] = (char)tmp;
            }
        }

        cvSetData(irimage,IRData, size.width);
        cvShowImage("IR",irimage);
        cvWaitKey(100);
#endif

#ifdef DEPTH
        unsigned char* depthdata = depth->data;
        CvSize size;
        size.height = 424; size.width = 512;
        IplImage* depthImage = cvCreateImage(size,IPL_DEPTH_8U,1);
        char* DEPTHData = (char*)malloc(size.height*size.width*sizeof(char));

        for (int i = 0; i != size.height; ++i){
            for (int j = 0; j != size.width; ++j){
                int idx = i*size.width + j;
                char2f cf;
                cf.c[0] = depthdata[idx*4];
                cf.c[1] = depthdata[idx*4+1];
                cf.c[2] = depthdata[idx*4+2];
                cf.c[3] = depthdata[idx*4+3];
                int tmp = 255 - MIN(round(cf.f / 8.0),255);
                DEPTHData[idx] = (char)tmp;
            }
        }

        cvSetData(depthImage, DEPTHData, size.width);
        cvShowImage("DEPTH", depthImage);
        cvWaitKey(100);

#endif

#ifdef RECOGNITION
        libfreenect2::Registration *registration = new libfreenect2::Registration(dev->getIrCameraParams() , dev->getColorCameraParams());
        libfreenect2::Frame undistorted(512 , 424 , 4) , registered(512 , 424 , 4);
        registration->apply(rgb , depth , &undistorted , &registered);
        unsigned char* registereddata = registered.data;
        unsigned char* undistorteddata = undistorted.data;

        CvSize size;
        size.height = 424; size.width = 512;

        char* REGISTERData = (char*)malloc(size.height*size.width*sizeof(char));

        for (int i = 0; i != size.height; ++i){
            for (int j = 0; j != size.width; ++j){
                int idx = i*size.width+j;
                char B = registereddata[idx * 4];
                char G = registereddata[idx * 4 + 1];
                char R = registereddata[idx * 4 + 2];

                int h = getH(char2int(R),char2int(G),char2int(B));
                float s = getS(char2int(R),char2int(G),char2int(B));

                if ((abs(h - 360) < 15 || abs(h - 0) < 15) && s > 0.6)
                    REGISTERData[idx] = 255;
                else
                    REGISTERData[idx] = 0;
            }
        }

        IplImage* biimage = cvCreateImage(size,IPL_DEPTH_8U,1);
        //IplImage* depthImage = cvCreateImage(size,IPL_DEPTH_8U,1);

        cvSetData(biimage, REGISTERData, size.width);
        //cvSetData(depthImage, DEPTHData, size.width);

        CvMemStorage* storage = cvCreateMemStorage(0);
        CvContourScanner scanner = NULL;
        scanner = cvStartFindContours(biimage, storage, sizeof(CvContour), CV_RETR_CCOMP,CV_CHAIN_APPROX_NONE);

        CvRect rect;
        CvSeq* contour = NULL;
        double minarea = 180.0, tmparea = 0.0, maxarea = 0.0;
        int rec_x = 0, rec_y = 0;

        while ((contour = cvFindNextContour(scanner)) != NULL){
            tmparea = fabs(cvContourArea(contour));
            rect = cvBoundingRect(contour);

            if (tmparea > minarea && tmparea > maxarea){
                rec_x = rect.x + rect.width / 2;
                rec_y = rect.y + rect.height / 2;
            }
        }

        //fetch depth data

        if (rec_x != 0){
            int idx = rec_x + rec_y*size.width;
            float dep = char2float(&undistorteddata[idx*4]);
            if (dep != 0){
                int x = (rec_x - 282) * dep / camera_params.fx / 3.45;
                std::cout << "depth is" << dep - 50 << " ; x is " << x << std::endl;
            }
        }

#endif

#ifdef AVOID


#endif

        listener.release(frames);
    }

}
#endif

#ifdef TEST
int main(){
    int b = 225, g = 0, r = 0;
    int a = getH(r,g,b);
    std::cout << a;
}
#endif


/** @function detectAndDisplay */
void detectAndDisplay( Mat frame )
{
    std::vector<Rect> faces;
    Mat frame_gray;

    cvtColor( frame, frame_gray, COLOR_BGR2GRAY );
    equalizeHist( frame_gray, frame_gray );

    //-- Detect faces
    face_cascade.detectMultiScale( frame_gray, faces, 1.1, 2, 0|CASCADE_SCALE_IMAGE, Size(30, 30) );

    for( size_t i = 0; i < faces.size(); i++ )
    {
        Point center( faces[i].x + faces[i].width/2, faces[i].y + faces[i].height/2 );
        ellipse( frame, center, Size( faces[i].width/2, faces[i].height/2), 0, 0, 360, Scalar( 255, 0, 255 ), 4, 8, 0 );

        Mat faceROI = frame_gray( faces[i] );
        std::vector<Rect> eyes;

        //-- In each face, detect eyes
        eyes_cascade.detectMultiScale( faceROI, eyes, 1.1, 2, 0 |CASCADE_SCALE_IMAGE, Size(30, 30) );

        for( size_t j = 0; j < eyes.size(); j++ )
        {
            Point eye_center( faces[i].x + eyes[j].x + eyes[j].width/2, faces[i].y + eyes[j].y + eyes[j].height/2 );
            int radius = cvRound( (eyes[j].width + eyes[j].height)*0.25 );
            circle( frame, eye_center, radius, Scalar( 255, 0, 0 ), 4, 8, 0 );
        }
    }
    //-- Show what you got
    imshow( window_name, frame );
}
