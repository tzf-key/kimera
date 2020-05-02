#ifndef CONFIG_H
#define CONFIG_H

#define SOCKNAME_VS "video_stream"
#define SOCKNAME_AS "audio_stream"
#define APP_DOMAIN "com.example.camera"
#define LOOPBACK_DEVICE "/dev/video0"
#define VIDEO_CODEC AV_CODEC_ID_HEVC
#define HEADER_SIZE 12
#define NO_PTS UINT64_C(-1)
#define SERVER_IP "192.168.0.15"
#define SERVER_PORT 8080

#define WIDTH 1920
#define HEIGHT 1080
#define FRAME_BUFFER_SIZE_YUV420 (WIDTH*HEIGHT*3/2)

#endif