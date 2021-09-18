#ifndef GUI_VARIABLES_H
#define GUI_VARIABLES_H

#include <linux/limits.h>
#define WIDTH 1280

#define HEIGHT 1000

extern float current_width;
extern float current_height;
extern int x, y;
extern char current_pos_char[3][14];
extern char waypoint_pos_char[3][14];

extern double sim_frame_time;
extern int sim_paused;
#endif