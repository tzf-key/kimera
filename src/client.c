#include "kimera/client.h"

void kimera_print_version() {
    printf("Kimera Version: %d.%d\n", kimera_VERSION_MAJOR, kimera_VERSION_MINOR);
    printf("Ffmpeg Version: %s\n", av_version_info());
}

void kimera_print_help() {
    printf("Usage:\n   kimera [mode] [profile] [config]\n");
    printf("    - [profile] (required)  Name of the selected profile from the configuration file.\n");
    printf("    - [mode]    (required)  Operation Mode: Transmitter (tx) or Receiver (rx).\n");
    printf("    - [config]  (optional)  Path of the configuration file containing profiles.\n");
    printf("Example:\n   kimera tx rpi4_picam_v1 profiles.yml\n");
}

void print_io_list(Interfaces interfaces) {
    if (interfaces == NONE)
        printf(" NONE");
    if (interfaces & TCP)
        printf(" TCP");
    if (interfaces & UDP)
        printf(" UDP");
    if (interfaces & UNIX)
        printf(" UNIX");
    if (interfaces & DISPLAY)
        printf(" DISPLAY");
    if (interfaces & LOOPBACK)
        printf(" LOOPBACK");
    printf("\n");
}

void kimera_print_state(State* state) {
    printf(".   CURRENT STATE\n");
    printf("├── Dimensions: %dx%d\n", state->width, state->height);
    printf("├── Framerate:  %d FPS\n", state->framerate);
    printf("├── Bitrate:    %d bps\n", state->bitrate);
    printf("└── Packet Len: %d Bytes\n", state->packet_size);

    if (state->mode == TRANSMITTER)
        printf("    .   TRANSMITTER\n");
    
    if (state->mode == RECEIVER)
        printf("    .   RECEIVER\n");

    printf("    ├── Source: ");
    print_io_list(state->source);
    printf("    ├── Sink:   ");
    print_io_list(state->sink);

    printf("    ├── Device:  %s\n", state->loopback);
    printf("    ├── Address: %s\n", state->address);
    printf("    ├── Port:    %d\n", state->port);
    printf("    └── Codec:   %s\n", state->codec);
}

void inthand(int signum) {
    if (stop == 1) {
      exit(-1);
    }
    printf("Safely exiting, press Crtl-C again to force shutdown.\n");
    stop = 1;
}

int kimera_client(
    int argc, char *argv[],
    void(*tx)(State*, volatile sig_atomic_t*),
    void(*rx)(State*, volatile sig_atomic_t*)) {
    // Register signal handler.
    signal(SIGINT, inthand);

    // Parse Command-Line Arguments
    if (argc < 2) {
        printf("Not enough arguments.\n");
        kimera_print_help();
        return -1;
    }

    if (!strcmp(argv[1], "-h") || !strcmp(argv[1], "--help")) {
        kimera_print_help();
        return 0;
    }

    if (!strcmp(argv[1], "-v") || !strcmp(argv[1], "--version")) {
        kimera_print_version();
        return 0;
    }

    if (argc < 3) {
        printf("Not enough arguments.\n");
        kimera_print_help();
        return -1;
    }

    // Declare Default Settings
    State* state = kimera_state();
    char* config_path = argv[2];

    switch (argv[1][0]) {
        case 't':
        case 'T':
            state->mode = TRANSMITTER;
            break;
        case 'r':
        case 'R':
            state->mode = RECEIVER;
            break;
        default:
            printf("Mode (%s) not valid.\n", argv[1]);
            kimera_print_help();
            kimera_free(state);
            return -1;
    }
        
    if (!kimera_parse_config_file(state, config_path)) {
        kimera_free(state);
        return -1;
    }
        
    switch (state->mode) {
        case TRANSMITTER:
            (void)(*tx)(state, &stop);
            break;
        case RECEIVER:
            (void)(*rx)(state, &stop);
            break;
        default: break;
    }

    kimera_free(state);
    return 0;
}