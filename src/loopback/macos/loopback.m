#include "kimera/loopback/macos/loopback.h"

#import "kimera/loopback/macos/CameraAdapter.h"

LoopbackState* alloc_loopback() {
    LoopbackState* state = malloc(sizeof(LoopbackState));
    state->frame = NULL;
    state->state = NULL;
    return state;
}

void free_loopback(LoopbackState* loopback, State* state) {
    [(id)loopback->state stopCapture];
    if (loopback->frame)
        av_frame_free(&loopback->frame);
    free(loopback);
}

bool open_loopback_sink(LoopbackState* loopback, State* state) {
    printf("[LOOPBACK] Sink isn't supported yet on macOS.\n");
    return false;
}

bool loopback_push_frame(LoopbackState* loopback, AVFrame* in) {
    printf("[LOOPBACK] Sink isn't supported yet on macOS.\n");
    return false;
}

bool open_loopback_source(LoopbackState* loopback, State* state) {
    loopback->state = [CameraAdapter new];

    if (![(id)loopback->state startCapture:state]) {
        return false;
    }
    
    loopback->frame = av_frame_alloc();
    loopback->frame->width = [(id)loopback->state getFrameWidth];
    loopback->frame->height = [(id)loopback->state getFrameHeight];
    loopback->frame->format = state->in_format;
    loopback->frame->pts = 0;

    if (av_frame_get_buffer(loopback->frame, 0) < 0){
        printf("[LOOPBACK] Couldn't allocate frame.\n");
        return false;
    }

    return true;
}

bool loopback_pull_frame(LoopbackState* loopback, State* state) {
    if (av_frame_make_writable(loopback->frame) < 0) {
        printf("[LOOPBACK] Frame is not writable.\n");
        return false;
    }

    if (![(id)loopback->state pullFrame: loopback->frame]) {
        return false;
    }

    loopback->frame->pts += 1;
    return true;
}