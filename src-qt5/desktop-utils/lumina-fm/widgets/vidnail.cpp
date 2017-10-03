#ifndef VIDNAIL_H
#define VIDNAIL_H

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
}

class VidNail;

struct vFrame {
    vFrame() : *width(0), *height(0) {}
    vFrame(int *width, int *height : width(width), height(height) {}
    int *width;
    int *height;
};

public:

    QString getCodec();
    void goto(int timeInSeconds);
    void readVideoFrame();
    void getScaledVideoFrame(int scaledSize, vFrame& vFrame);

    int getWidth();
    int getHeight();
    int getLength();

    void makeThumbnail(const QString& videoFile, QImage &image);
    void setThumbnailSize(int size);
    void setPercentage(int percent);
    void setTime(const QString& Time);

    void writeVidNail(vFrame& frame, QImage& image);


 private:

    bool readVideoPacket();
    bool getVideoPacket();
    void scaleVideo(int scaledSize, int& scaledWidth, int& scaledHeight);
    void createVFrame(AVFrame *vFrame, quint8 *frameBuffer, int width, int height);
    void calculateDimensions(int size);
    void generateThumbnail(const QString& videoFile, ImageWriter& imageWriter, QImage& 
image);
    QString getMimeType(const QString& videoFile);
    QString getExtension(const QString& videoFilename);


 private:
     int                    videoStream;
     AVFormatContext        *inputVideoFormatContext;
     AVCodecContext         *inputvideoCodecContext;
     AVCodec                *inputVideoCodec;
     AVStream               *inputVideoStream;
     AVFrame                *inputVideoFrame;
     quint8                 *inputFrameBuffer;
     AVPacket               *videoPacket;


#endif // VIDNAIL_H
