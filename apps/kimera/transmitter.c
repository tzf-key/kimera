#include "transmitter.h"

void transmitter(State* state, volatile sig_atomic_t* stop) {
    // Start TCP Server. 
    SocketState tcp_socket;
    if (state->sink & TCP) {
        if (!open_tcp_server(&tcp_socket, state))
            goto cleanup;
    }

    // Start UDP Server. 
    SocketState udp_socket;
    if (state->sink & UDP) {
        if (!open_udp_server(&udp_socket, state))
            goto cleanup;
    }

    // Start UNIX Server. 
    SocketState unix_socket;
    if (state->sink & UNIX) {
        if (!open_unix_server(&unix_socket, state))
            goto cleanup;
    }

    // Start Router. 
    RouterState router;
    if (state->sink & UNIX || state->sink & TCP || state->sink & UDP) {
        if (!start_router(&router, state))
            goto cleanup;
    }

    // Start Loopback Input.
    LoopbackState loopback;
    if (!open_loopback_source(&loopback, state))
        goto cleanup;

    // Start Display Screen.
    DisplayState display;
    if (state->sink & DISPLAY) {
        if (!start_display(&display, state))
            goto cleanup;
    }

    // Start Encoder.
    EncoderState encoder;
    if (!start_encoder(&encoder, state))
        goto cleanup;

    // Add resampler.
    ResamplerState resampler;
    open_resampler(&resampler, state->out_format);

    // Start Decoder Loop.
    while (loopback_pull_frame(&loopback, state) && !(*stop)) {
        if (!resampler_push_frame(&resampler, state, loopback.frame)) {
            continue;
        }

        if (state->sink & DISPLAY) {
            if (!display_draw(&display, state, resampler.frame))
                break;
        }
        
        if (encoder_push(&encoder, resampler.frame)) {
            if (state->sink & STDOUT) {
                fwrite(
                    encoder.packet->data, sizeof(char),
                    encoder.packet->size, stdout);
                continue;
            }

            while (make_packet(&router, encoder.packet, resampler.frame)) {
                if (state->sink & TCP)
                    send_packet(&router, &tcp_socket);
                if (state->sink & UDP)
                    send_packet(&router, &udp_socket);
                if (state->sink & UNIX)
                    send_packet(&router, &unix_socket);
            }
        }
    }
    
cleanup:
    if (state->sink & TCP)
        close_tcp(&tcp_socket);

    if (state->sink & UNIX)
        close_unix(&unix_socket);
    
    if (state->sink & UNIX)
        close_udp(&udp_socket);

    if (state->sink & TCP || state->sink & UNIX || state->sink & UDP)
        close_router(&router);

    if (state->sink & DISPLAY)
        close_display(&display);

    close_resampler(&resampler);
    close_loopback(&loopback, state);
    close_encoder(&encoder);
}