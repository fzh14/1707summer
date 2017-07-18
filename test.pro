TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += \
    main.cpp

INCLUDEPATH += /home/guo/freenect2/include/
LIBS += -L/home/guo/freenect2/lib/ -lfreenect2
LIBS += -L/usr/local/lib/ -lopencv_core -lopencv_gpu -lopencv_highgui -lopencv_imgproc -lopencv_photo -lopencv_videoio -lopencv_objdetect
HEADERS += \
    util.h

DISTFILES += \
    haarcascade_eye_tree_eyeglasses.xml \
    haarcascade_frontalface_alt.xml \
    pytest.py

INCLUDEPATH +=  /usr/include/python2.7\
                /usr/include/x86_64-linux-gnu/python2.7\
                -fno-strict-aliasing -Wdate-time -D_FORTIFY_SOURCE=2 -g -fstack-protector-strong -Wformat -Werror=format-security  -DNDEBUG -g -fwrapv -O2 -Wall -Wstrict-prototypes

LIBS += -L/usr/lib/python2.7/config-x86_64-linux-gnu -L/usr/lib -lpython2.7\
        -lpthread -ldl  -lutil -lm  -Xlinker -export-dynamic -Wl,-O1 -Wl,-Bsymbolic-functions
