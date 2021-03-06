#include "kimera/loopback/linux/loopback.h"

LoopbackState* alloc_loopback() {
    LoopbackState* state = malloc(sizeof(LoopbackState));
    state->frame = NULL;
    state->v4l2  = NULL;
    state->xcb   = NULL;
    return state;
}

void close_loopback(LoopbackState* loopback, State* state) {
    if (state->source & DISPLAY)
        return free_xcb(loopback);
    if (state->source & LOOPBACK)
        return free_v4l2(loopback, state);

    if (loopback->frame)
        av_frame_free(&loopback->frame);

    free(loopback);
}

inline bool open_loopback_sink(LoopbackState* loopback, State* state) {
    return init_v4l2_sink(loopback, state);
}

inline bool loopback_push_frame(LoopbackState* loopback, AVFrame* frame) {
    return push_v4l2_frame(loopback, frame);
}

inline bool open_loopback_source(LoopbackState* loopback, State* state) {
    if (state->source & DISPLAY)
        return init_xcb_source(loopback, state);
    if (state->source & LOOPBACK)
        return init_v4l2_source(loopback, state);
    return false;
}

inline bool loopback_pull_frame(LoopbackState* loopback, State* state) {
    if (state->source & DISPLAY)
        return pull_xcb_frame(loopback, state);
    if (state->source & LOOPBACK)
        return pull_v4l2_frame(loopback);
    return false;
}