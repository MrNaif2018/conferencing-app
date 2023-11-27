#ifndef CONFIG_H
#define CONFIG_H

#define FRAME_HEIGHT 720  // for transfer
#define FRAME_WIDTH 1080  // for transfer
#define FPS 60            // fps
#define PACK_SIZE 60176   // < max UDP packet size
#define ENCODE_QUALITY 90 // larger=more quality but large packet sizes

#define IMAGE_UDP_PORT 50000
#define AUDIO_UDP_PORT 55455

#define SETTINGS_FILE "config.ini"
#endif