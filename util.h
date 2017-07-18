#ifndef UTIL_H
#define UTIL_H

#include <math.h>
#include <opencv2/opencv.hpp>
#include <stdarg.h>

union charint{
    char c[4];
    int i;
};

union char2f{
    char c[4];
    float f;
};

int char2int(char x){
    charint ci;
    ci.c[0] = x;
    ci.c[1] = 0;
    ci.c[2] = 0;
    ci.c[3] = 0;
    return ci.i;
}

float char2float(unsigned char* x){
    char2f cf;
    cf.c[0] = x[0];
    cf.c[1] = x[1];
    cf.c[2] = x[2];
    cf.c[3] = x[3];
    return cf.f;
}

int getH(int r, int g, int b){
    int max = r, min = r,h;
    if (g>max)
        max = g;
    if (b>max)
        max = b;
    if (g<min)
        min = g;
    if (b<min)
        min = b;
    if (r == max){
        h = round((g-b)*60.0/(max - min));
    }
    if (g == max){
        h = round(120+(b-r)*60.0/(max - min));
    }
    if (b == max){
        h = round(240+(r-g)*60.0/(max - min));
    }
    if (h<0)
        h += 360;
    return h;
}

float getS(int r, int g, int b){
    int max = r, min = r;
    if (g>max)
        max = g;
    if (b>max)
        max = b;
    if (g<min)
        min = g;
    if (b<min)
        min = b;
    return (max - min)*1.0/max;
}

void cvShowManyImages(char* title, int nArgs, ...)
{

    // img - Used for getting the arguments
    IplImage *img;

    // DispImage - the image in which input images are to be copied
    IplImage *DispImage;

    int size;
    int i;
    int m, n;
    int x, y;

    // w - Maximum number of images in a row
    // h - Maximum number of images in a column
    int w, h;

    // scale - How much we have to resize the image
    float scale;
    int max;

    // If the number of arguments is lesser than 0 or greater than 12
    // return without displaying
    if(nArgs <= 0)
    {
        printf("Number of arguments too small....\n");
        return;
    }
    else if(nArgs > 12)
    {
        printf("Number of arguments too large....\n");
        return;
    }
    // Determine the size of the image, and the number of rows/cols  from number of arguments
    else if (nArgs == 1)
    {
        w = h = 1;
        size = 300;
    }
    else if (nArgs == 2)
    {
        w = 2; h = 1;
        size = 300;
    }
    else if (nArgs == 3 || nArgs == 4)
    {
        w = 2; h = 2;
        size = 300;
    }
    else if (nArgs == 5 || nArgs == 6) {
        w = 3; h = 2;
        size = 200;
    }
    else if (nArgs == 7 || nArgs == 8)
    {
        w = 4; h = 2;
        size = 200;
    }
    else
    {
        w = 4; h = 3;
        size = 150;
    }

    // Create a new 3 channel image0
    DispImage = cvCreateImage( cvSize( 100+ size*w, 60 + size*h), IPL_DEPTH_8U, 1 );

    // Used to get the arguments passed
    va_list args;
    va_start(args, nArgs);

    // Loop for nArgs number of arguments
    for (i = 0, m = 20, n = 20; i < nArgs; i++, m += (20 + size))
    {
        // Get the Pointer to the IplImage
        img = va_arg(args, IplImage*);

        // Check whether it is NULL or not
        // If it is NULL, release the image, and return
        if(img == 0)
        {
            printf("Invalid arguments");
            cvReleaseImage(&DispImage);
            return;
        }

        // Find the width and height of the image
        x = img->width;
        y = img->height;

        // Find whether height or width is greater in order to resize the image
        max = (x > y)? x: y;

        // Find the scaling factor to resize the image
        scale = (float) ( (float) max / size );

        // Used to Align the images
        if( i % w == 0 && m!= 20)
        {
            m = 20;
            n+= 0 + size;
        }

        // Set the image ROI to display the current image
        //cvSetImageROI(DispImage, cvRect(m, n, (int)( x/scale ), (int)( y/scale )));
        cvSetImageROI(DispImage, cvRect(m, n, (int)( x/scale ), (int)( y/scale )));
        //      cout<<"x="<<m<<"y="<<n<<endl;

        // Resize the input image and copy the it to the Single Big Image
        cvResize(img, DispImage);

        // Reset the ROI in order to display the next image
        cvResetImageROI(DispImage);
    }

    // Create a new window, and show the Single Big Image
    //cvNamedWindow( title, 1 );
    cvShowImage( title, DispImage);

    /*cvWaitKey(0);*/
    //cvDestroyWindow(title);

    // End the number of arguments
    va_end(args);

    // Release the Image Memory
    cvReleaseImage(&DispImage);
}

#endif // UTIL_H
