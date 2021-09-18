#ifndef DISPLAY_FUNCTIONS_H
#define DISPLAY_FUNCTIONS_H

#include "cairo.h"
#include "ft2build.h"
#include FT_FREETYPE_H
#include "cairo-ft.h"
#include "gui_structs.h"

extern cairo_font_face_t *seven_seg;
extern cairo_font_face_t *sans;
//freetype
extern FT_Library value;
extern FT_Face ft_seven_seg;
extern FT_Face ft_sans;
extern FT_Error status;

void init_font();

void warn_lit(cdu_t *cdu, cairo_t *cr);

void left_display(double nums[5], int size, bool disp_on, bool deg, bool decimal, bool north, bool south);

void right_display(double nums[6], int size, bool disp_on, bool deg, bool decimal, bool east, bool west);

void from_pt(int frm);

void to_pt(int to_i);

#endif