#pragma once

#include <stdlib.h>
#include <stdint.h>

#define EXPORT __attribute__((visibility("default")))

#ifdef __cplusplus
extern "C"
{
#endif

#define FRAMESINK_BGR24 0
#define FRAMESINK_RGB24 1
#define FRAMESINK_YUV420 2

    struct FrameSink;
    typedef struct FrameSink *FrameSinkHandle;

    EXPORT int v4l2_openFrameSink(FrameSinkHandle *hdl,
                                  const char *device,
                                  size_t width, size_t height,
                                  uint32_t pixelFormat);
    EXPORT int v4l2_closeFrameSink(FrameSinkHandle hdl);

    EXPORT int v4l2_writeFrame(FrameSinkHandle hdl, uint8_t *buf, size_t width, size_t height, uint32_t pixelFormat);

#ifdef __cplusplus
}
#endif