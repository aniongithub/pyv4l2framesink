#include <stdio.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/videodev2.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>

#include <v4l2framesink/v4l2framesink.h>

struct FrameSink
{
    int v4l2sink;
    size_t frameSizeBytes;
    uint32_t pixelFormat;
    uint8_t *buffer;
};

static size_t getFrameSizeBytes(size_t width, size_t height, uint32_t pixelFormat)
{
    switch (pixelFormat)
    {
    case FRAMESINK_YUV420:
        return 3 * width * height / 2;
    default:
        return 0;
    }
}

static uint32_t convertTov4l2pixelFormat(int pixfmt)
{
    switch (pixfmt)
    {
    case FRAMESINK_YUV420:
        return V4L2_PIX_FMT_YUV420;
    case FRAMESINK_BGR24:
        return V4L2_PIX_FMT_BGR24;
    case FRAMESINK_RGB24:
        return V4L2_PIX_FMT_RGB24;
    default:
        return 0;
    }
}

static void RGB2Yuv420p(uint8_t *destination, uint8_t *rgb, size_t width, size_t height, uint8_t rOffset, uint8_t gOffset, uint8_t bOffset)
{
    size_t image_size = width * height;
    size_t upos = image_size;
    size_t vpos = upos + upos / 4;
    size_t i = 0;

    for (size_t line = 0; line < height; ++line)
    {
        if (!(line % 2))
        {
            for (size_t x = 0; x < width; x += 2)
            {
                uint8_t r = rgb[3 * i + rOffset];
                uint8_t g = rgb[3 * i + gOffset];
                uint8_t b = rgb[3 * i + bOffset];

                destination[i++] = ((66 * r + 129 * g + 25 * b) >> 8) + 16;

                destination[upos++] = ((-38 * r + -74 * g + 112 * b) >> 8) + 128;
                destination[vpos++] = ((112 * r + -94 * g + -18 * b) >> 8) + 128;

                r = rgb[3 * i + rOffset];
                g = rgb[3 * i + gOffset];
                b = rgb[3 * i + bOffset];

                destination[i++] = ((66 * r + 129 * g + 25 * b) >> 8) + 16;
            }
        }
        else
        {
            for (size_t x = 0; x < width; x += 1)
            {
                uint8_t r = rgb[3 * i + rOffset];
                uint8_t g = rgb[3 * i + gOffset];
                uint8_t b = rgb[3 * i + bOffset];

                destination[i++] = ((66 * r + 129 * g + 25 * b) >> 8) + 16;
            }
        }
    }
}

int v4l2_openFrameSink(FrameSinkHandle *hdl,
                       const char *device,
                       size_t width, size_t height,
                       uint32_t sinkPixelFormat)
{
    *hdl = nullptr;

    // Fail fast conditions
    auto sizeBytes = getFrameSizeBytes(width, height, sinkPixelFormat);
    if (sizeBytes == 0)
        return -1;

    // Open the device
    auto sink = open(device, O_WRONLY);
    if (sink < 0)
        return errno;

    // Setup video for proper format
    struct v4l2_format format = {0};
    format.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;

    auto err = ioctl(sink, VIDIOC_G_FMT, &format);
    if (err < 0)
    {
        close(sink);
        return err;
    }

    format.fmt.pix.width = width;
    format.fmt.pix.height = height;
    format.fmt.pix.pixelformat = convertTov4l2pixelFormat(sinkPixelFormat);
    format.fmt.pix.sizeimage = sizeBytes;

    err = ioctl(sink, VIDIOC_S_FMT, &format);

    if (err < 0)
        return err;

    // Everything went swimmingly
    auto result = new FrameSink();
    result->v4l2sink = sink;
    result->frameSizeBytes = sizeBytes;
    result->pixelFormat = convertTov4l2pixelFormat(sinkPixelFormat);
    result->buffer = (uint8_t *)malloc(sizeBytes);

    *hdl = result;

    return 0;
}

int v4l2_closeFrameSink(FrameSinkHandle hdl)
{
    if (hdl)
    {
        if (hdl->v4l2sink)
            close(hdl->v4l2sink);

        if (hdl->buffer)
            free(hdl->buffer);

        delete hdl;
        return 0;
    }

    // Encountered errors
    return -1;
}

int v4l2_writeFrame(FrameSinkHandle hdl, uint8_t *buf, size_t width, size_t height, uint32_t pixelFormat)
{
    uint8_t rOffset, gOffset, bOffset;
    switch (pixelFormat)
    {
    case FRAMESINK_BGR24:
        RGB2Yuv420p(hdl->buffer, buf, width, height, 2, 1, 0);
        write(hdl->v4l2sink, hdl->buffer, hdl->frameSizeBytes);
        return 0;

    case FRAMESINK_RGB24:
        RGB2Yuv420p(hdl->buffer, buf, width, height, 0, 1, 2);
        write(hdl->v4l2sink, hdl->buffer, hdl->frameSizeBytes);
        return 0;

    case FRAMESINK_YUV420:
        memcpy(hdl->buffer, buf, hdl->frameSizeBytes);
        return 0;

    default:
        return -1;
    }
}