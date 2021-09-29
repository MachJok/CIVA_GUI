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
extern int cdu_warn_light[3];
extern int cdu_batt_light[3];
extern int cdu_alert_light[3];

extern int waypoint_sel_num[3];
extern int auto_man_knob_dr[3];
extern int nav_from_to[3][2];

extern double sim_frame_time;
extern int sim_paused;
#endif