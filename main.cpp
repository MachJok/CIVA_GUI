#include "main.h"
#include "source_includes.h"
#include "gui_structs.h"
#include "XPLMDisplay.h"
#include "XPLMProcessing.h"
#include "acfutils/assert.h"
#include "acfutils/core.h"
#include "acfutils/helpers.h"
#include "acfutils/log.h"
#include "acfutils/mt_cairo_render.h"
#include "acfutils/sysmacros.h"
#include "cairo.h"
#include "gui_variables.h"
#include <cstdio>
#include <cstring>
#include <string>
#include <map>
#include "gui_functions.h"
#include "gui_datarefs.h"

#if IBM
	#include <windows.h>
#endif
#if LIN
	#include <GL/gl.h>
#elif __GNUC__
	#include <OpenGL/gl.h>
#else
	#include <GL/gl.h>
#endif

#ifndef XPLM300
	#error This is made to be compiled against the XPLM300 SDK
#endif

#define WIDTH 1280
#define HEIGHT 1000
#define MIN_WIDTH (WIDTH/4)
#define MIN_HEIGHT (HEIGHT/4)
cdu_t cdu_data[3]{0};
color_t text_color;
char waypoint_pos_char[3][14]{0};
char current_pos_char[3][14]{0};
std::map<XPLMWindowID, int> window_map;

mt_cairo_render_t *mtcr[3];
cairo_t *cr;
cairo_matrix_t cr_matrix[3]{0};
cairo_surface_t *cdu_bg;
cairo_surface_t *warn_light_on;
cairo_surface_t *warn_light_off;
cairo_surface_t *key_pad_on;
cairo_surface_t *key_pad_off;
cairo_surface_t *bat_on;
cairo_surface_t *bat_off;
cairo_surface_t *alert_on;
cairo_surface_t *alert_off;
cairo_surface_t *insert_on;
cairo_surface_t *insert_off;
cairo_surface_t *remote_on;
cairo_surface_t *remote_off;
cairo_surface_t *hold_on;
cairo_surface_t *hold_off;
cairo_surface_t *wyptchg_on;
cairo_surface_t *wyptchg_off;
cairo_surface_t *wpt_num_digits;
cairo_surface_t *wpt_bkg;
cairo_surface_t *data_selector_knob;
cairo_surface_t *auto_man_knob;
cairo_surface_t *clear_off;
cairo_surface_t *clear_on;
cairo_surface_t *dimmer;

cairo_font_face_t *seven_seg;
cairo_font_face_t *sans;
//freetype
FT_Library value;
FT_Face ft_seven_seg;
FT_Face ft_sans;
FT_Error status;

const char cdu_path[] = "images/cdu_base.png";
const char warn_light_on_path[] = "images/btn_lit/warn.png";
const char warn_light_off_path[] = "images/btn_unlit/warn.png";
const char key_pad_on_path[] = "images/btn_lit/key_pad.png";
const char key_pad_off_path[] = "images/btn_unlit/key_pad.png";
const char bat_on_path[] = "images/btn_lit/bat.png";
const char bat_off_path[] = "images/btn_unlit/bat.png";
const char alert_on_path[] = "images/btn_lit/alert.png";
const char alert_off_path[] = "images/btn_unlit/alert.png";
const char insert_on_path[] = "images/btn_lit/insert.png";
const char insert_off_path[] = "images/btn_unlit/insert.png";
const char remote_on_path[] = "images/btn_lit/remote.png";
const char remote_off_path[] = "images/btn_unlit/remote.png";
const char hold_on_path[] = "images/btn_lit/hold.png";
const char hold_off_path[] = "images/btn_unlit/hold.png";
const char wyptchg_on_path[] = "images/btn_lit/wypt_chg.png";
const char wyptchg_off_path[] = "images/btn_unlit/wypt_chg.png";
const char wpt_num_digits_path[] = "images/wpt_num.png";
const char wpt_bkg_path[] = "images/wpt_window.png";
const char data_selector_knob_path[] = "images/knobs/dataselect.png";
const char auto_man_knob_path[] = "images/knobs/auto-man-test.png";
const char clear_off_path[] = "images/btn_unlit/clear.png";
const char clear_on_path[] = "images/btn_lit/clear.png";
const char dimmer_path[] = "images/knobs/dimmer.png";
const char* seven_seg_fnt_path = "Fonts/DSEG7Classic-Italic-1.ttf";
const char* sans_fnt_path = "Fonts/NotoSansCJK-Bold.ttc";
const char* seven_seg_full_path;
const char* sans_full_path;

vect3_t color = {1,0,1};
int window_l{0}, window_t{0}, window_r{0}, window_b{0}, w_curr[3], h_curr[3], 
	w_old[3], h_old[3], x, y, first_render_loop{0},sim_paused{0}, 
	window_index{0}, cdu_warn_light[3]{0}, cdu_batt_light[3]{0};
char plugindir[MAX_PATH]{};
char* p;

vect2_t pos[3]{0}, size[3]{0};

// An opaque handle to the window we will create
static XPLMWindowID	g_window[3]{NULL};
static XPLMWindowID current_window{NULL};
static float g_pop_button_lbrt[4]{0}, g_position_button_lbrt[4]{0}; // left, bottom, right, top
XPLMCreateWindow_t params;

static float curr_width{0}, curr_height{0};

bool second_update{false}, init_renderer_complete[3]{false}, loaded_datarefs{false};
double sim_frame_time{0}, current_time{0}, sx[3]{1}, sy[3]{1}, old_sx[3]{1}, old_sy[3]{1}, rel_sx[3]{1}, rel_sy[3]{1};
double mat_xx[3], mat_xy[3], mat_yx[3], mat_yy[3], mat_x0[3], mat_y0[3];

int g_menu_container_idx; // The index of our menu item in the Plugins menu
XPLMMenuID g_menu_id; // The menu container we'll append all our menu items to

void image_path_setup();
cairo_surface_t* load_image(const char* img_path);

char* load_font(const char* font_path);

void init_font();

void n_s_light(bool on, bool north, bool test, cairo_t *cr);

void e_w_light(bool on, bool east, bool test, cairo_t *cr);

void left_display(cdu_t *cdu, cairo_t *cr);

void right_display(cdu_t *cdu, cairo_t *cr);

void from_display(cdu_t *cdu, cairo_t *cr); //add flashing function

void to_display(cdu_t *cdu, cairo_t *cr); //add flashing function

void warn_lit(cdu_t *cdu, cairo_t *cr);

void numpad_lit(cdu_t *cdu, cairo_t *cr);

void bat_lit(cdu_t *cdu, cairo_t *cr);

void alert_lit(cdu_t *cdu, cairo_t *cr);

void insert_lit(cdu_t *cdu, cairo_t *cr);

void remote_lit(cdu_t *cdu, cairo_t *cr);

void hold_lit(cdu_t *cdu, cairo_t *cr);

void wyptchg_lit(cdu_t *cdu, cairo_t *cr);

void clear_lit(cdu_t *cdu, cairo_t *cr);

void wpt_sel(cdu_t *cdu, cairo_t *cr);

void wpt_sel_bkg(cairo_t *cr);

void data_selector(cdu_t *cdu, cairo_t *cr);

void text_dimmer(cdu_t *cdu, cairo_t *cr);

void brightness_knob(cdu_t *cdu, cairo_t *cr);

void auto_man(cdu_t *cdu, cairo_t *cr);

void screen_format(cdu_t *cdu);



void test_mode_set(cdu_t *cdu, cairo_t *cr);

void display_logic(cdu_t *cdu, cairo_t *cr);

void make_window_map();

//DELTE ME AND MY ASSOCIATED CODE 
void load_cdu_data(int window_index);


// Callbacks we will register when we create our window
void				draw(XPLMWindowID in_window_id, void * in_refcon);
int					handle_mouse(XPLMWindowID in_window_id, int x, int y, int is_down, void * in_refcon);
void				receive_main_monitor_bounds(int inMonitorIndex, int inLeftBx, int inTopBx, int inRightBx, int inBottomBx, void * refcon);
int					dummy_mouse_handler(XPLMWindowID in_window_id, int x, int y, int is_down, void * in_refcon);
XPLMCursorStatus	dummy_cursor_status_handler(XPLMWindowID in_window_id, int x, int y, void * in_refcon);
int					dummy_wheel_handler(XPLMWindowID in_window_id, int x, int y, int wheel, int clicks, void * in_refcon);
void				dummy_key_handler(XPLMWindowID in_window_id, char key, XPLMKeyFlags flags, char virtual_key, void * in_refcon, int losing_focus);
int					coord_in_rect(float x, float y, float * bounds_lbrt);
void 				window_params_setup(XPLMCreateWindow_t param);
void 				menu_handler(void *, void *);

int 				render_loop(XPLMDrawingPhase inPhase, int inIsBefore, void* refcon);
void 				render_cb(cairo_t *cr, unsigned int w, unsigned int h, void* userinfo);
void				fini_cb();

bool				seconds_update();
void 				get_plugin_path();




static void
log_dbg_string(const char *str)
{
        XPLMDebugString(str);
}

//cairo drawing functions
int	coord_in_rect(float x, float y, float * bounds_lbrt)  
{ return ((x >= bounds_lbrt[0]) && (x < bounds_lbrt[2]) && (y < bounds_lbrt[3]) && (y >= bounds_lbrt[1])); }

PLUGIN_API int XPluginStart(
							char *		outName,
							char *		outSig,
							char *		outDesc)
{
	strcpy(outName, "CIVA WINDOW");
	strcpy(outSig, "OMI.CIVA.GUI");
	strcpy(outDesc, "CIVA GUI WINDOW FOR LIBCIVA");
	XPLMEnableFeature("XPLM_USE_NATIVE_PATHS", 1);
	log_init(log_dbg_string, "CIVA-GUI");
	logMsg("CIV-A GUI using libacfutils-%s", libacfutils_version);
	get_plugin_path();

	image_path_setup();
	init_font();
	glewInit();
	g_menu_container_idx = XPLMAppendMenuItem(XPLMFindPluginsMenu(), "CIV-A Menu", 0, 0);
	g_menu_id = XPLMCreateMenu("CIV-A Menu", XPLMFindPluginsMenu(), g_menu_container_idx, menu_handler, NULL);
	XPLMAppendMenuItem(g_menu_id, "Show/Hide CDU-1", (void *)"Menu Item 1", 1);
	XPLMAppendMenuItem(g_menu_id, "Show/Hide CDU-2", (void *)"Menu Item 2", 1);
	XPLMAppendMenuItem(g_menu_id, "Show/Hide CDU-3", (void *)"Menu Item 3", 1);
	// XPLMAppendMenuSeparator(g_menu_id);
	// XPLMAppendMenuItem(g_menu_id, "Show Window", (void *)"Menu Item 2", 1);
	// XPLMAppendMenuItemWithCommand(g_menu_id, "Toggle Flight Configuration (Command-Based)", XPLMFindCommand("sim/operation/toggle_flight_config"));

	XPLMMenuID aircraft_menu = XPLMFindAircraftMenu();
	if(aircraft_menu) // This will be NULL unless this plugin was loaded with an aircraft (i.e., it was located in the current aircraft's "plugins" subdirectory)
	{
		XPLMAppendMenuItemWithCommand(aircraft_menu, "Toggle Settings (Command-Based)", XPLMFindCommand("sim/operation/toggle_settings_window"));
	}
	window_params_setup(params);
	logMsg("Menu Created successfuly");

	
	return 1;
}


PLUGIN_API int  XPluginEnable(void)  
{ 
	logMsg("entered XPluginEnable");

	//register the render loop so we draw after (0) the window (xplm_Phase_Window) is drawn

	//XPLMRegisterDrawCallback(render_loop, xplm_Phase_Window, 1, current_window);
	make_window_map();



	return 1; 
}

PLUGIN_API void	XPluginStop(void)
{
	// Since we created the window, we'll be good citizens and clean it up
	for(int i = 0; i < 3; ++i)
	{
		XPLMDestroyWindow(g_window[i]);
		g_window[i] = NULL;
	}
	XPLMDestroyMenu(g_menu_id);
	//XPLMUnregisterDrawCallback(render_loop, xplm_Phase_Window, 0, current_window);
	for(int i = 0; i < 3; ++i)
	{
		mt_cairo_render_fini(mtcr[i]);
	}
	fini_cb();
	log_fini();

}

PLUGIN_API void XPluginDisable(void) 
{

}
PLUGIN_API void XPluginReceiveMessage(XPLMPluginID inFrom, int inMsg, void * inParam) { }

void make_window_map()
{
	window_map = {{g_window[0], 0}, {g_window[1], 1}, {g_window[2], 2}};
}
void load_cdu_data(int window_index)
{
	cdu_data[window_index].data_sel = POS;
    cdu_data[window_index].displays.left_display_on = true;
    cdu_data[window_index].displays.right_display_on = true;
    cdu_data[window_index].test_pressed = false;
    cdu_data[window_index].wpt_num = 1;
    cdu_data[window_index].brightness_knob_ang = 0;
    cdu_data[window_index].displays.nav_from = 1;
    cdu_data[window_index].displays.nav_to = 2;
    cdu_data[window_index].hold_pressed = false;
    cdu_data[window_index].hold_on = false;	
	cdu_data[window_index].bat_on = cdu_batt_light[window_index];
	cdu_data[window_index].warn_on = cdu_warn_light[window_index];
}

//digit display refresh controller
bool seconds_update()
{
	current_time += sim_frame_time;
	if(current_time >= 1.0 && !sim_paused)
	{
		second_update = true;
		current_time = 0;
		return second_update;
	}

	else
	{
		second_update = false;
		return second_update;
	}

}

void	draw(XPLMWindowID in_window_id, void * in_refcon)
{
	//read in the current window index
	window_index = window_map[in_window_id];


	//if the mtcr instances have not been initialized for the called window then do so
	if(!init_renderer_complete[window_index])
	{	
		//on the first pass load in datarefs
		if(!loaded_datarefs)
		{
			LoadDataRefs();
			loaded_datarefs = true;
			logMsg("datarefs loaded");
		}
		//set the "old" window sizes to the load window sizes
		w_curr[window_index] = MIN_WIDTH; //current width
		h_curr[window_index] = MIN_HEIGHT; //current height
		w_old[window_index] = w_curr[window_index];
		h_old[window_index] = h_curr[window_index];
		//initialize mt_cairo
		GetDataRefs();
		load_cdu_data(window_index);
		mtcr[window_index] = mt_cairo_render_init(w_curr[window_index], h_curr[window_index], 0, 
							NULL, render_cb, NULL, &cdu_data[window_index]);
		//set the flag to true to prevent scaling in the render_cb
		init_renderer_complete[window_index] = true;
		//enable single threaded mode
		mt_cairo_render_enable_fg_mode(mtcr[window_index]);

		logMsg("CDU window %i init: %s", window_index, 
				mtcr[window_index] != NULL ? "done" : "failed");

		ASSERT(mtcr[window_index] != NULL);
	}

	//get the window geometry 
	XPLMGetWindowGeometry
	(in_window_id, &window_l, &window_t, &window_r, &window_b);
	w_curr[window_index] = window_r - window_l; //current width
	h_curr[window_index] = window_t - window_b; //current height
	pos[window_index] = {(double) window_l, (double) window_b}; //current position
	size[window_index] = {(double) w_curr[window_index], (double) h_curr[window_index]}; //current size


	//if the size has changed set the new size to be the current size
	//and reinitialize the renderer
	if(mtcr[window_index] != NULL && (w_curr[window_index] != w_old[window_index] || 
									  h_curr[window_index] != h_old[window_index]))
	{
		mt_cairo_render_fini(mtcr[window_index]);
		mtcr[window_index] = NULL;
		//reinitialize to the current size of the current window value
		GetDataRefs();
		load_cdu_data(window_index);
		mtcr[window_index] = mt_cairo_render_init(w_curr[window_index], h_curr[window_index], 0, NULL, 
							render_cb, NULL, &cdu_data[window_index]);
		ASSERT(mtcr[window_index] != NULL);
	}
	//set the old size to the current size for next loop size checks
	w_old[window_index] = w_curr[window_index];
	h_old[window_index] = h_curr[window_index];

	//get the current datarefs from the sim and other plugins
	GetDataRefs();
	load_cdu_data(window_index);
	//set the update flag for cdu data update to true if 1 second has elapsed
	bool update_flag = seconds_update();
	//load the new data into the cdu screens if the update flag is triggered
	if(update_flag)
	{
		load_digits(&cdu_data[window_index], current_pos_char[window_index],
						cdu_data[window_index].displays.left_digits, 
						cdu_data[window_index].displays.right_digits);	
	}

	mt_cairo_render_once_wait(mtcr[window_index]);
	mt_cairo_render_draw(mtcr[window_index], pos[window_index], size[window_index]);
}



void render_cb(cairo_t *cr, unsigned w, unsigned h, void* userinfo)
{
	//not really sure what the best way to handle scaling is, this currently doesnt work
	if(!init_renderer_complete[window_index])
	{
		sx[window_index] = (double)MIN_WIDTH / (float)WIDTH; //temp for debug
		sy[window_index] = (double)MIN_HEIGHT / (float)HEIGHT;
		cairo_matrix_init(
			&cr_matrix[window_index], 
			mat_xx[window_index], mat_yx[window_index], mat_xy[window_index], 
			mat_xy[window_index], mat_x0[window_index], mat_y0[window_index]);
	}

	cairo_scale(cr, sx[window_index], sy[window_index]);

	cdu_t cdu_data = *((cdu_t*)userinfo);
	cairo_get_matrix(cr, &cr_matrix[window_index]); //what is the current matrix for debugging
	cairo_set_source_rgba(cr, color.x, color.y, color.z,1);
	cairo_paint(cr);
	

	wpt_sel_bkg(cr);
    wpt_sel(&cdu_data, cr);


    cairo_set_source_surface(cr, cdu_bg, 0, 0);
    cairo_paint(cr);
    //test_mode_set(&cdu_data);


    //knobs
    data_selector(&cdu_data, cr);
    screen_format(&cdu_data);
    brightness_knob(&cdu_data, cr);
    display_logic(&cdu_data, cr);
    auto_man(&cdu_data, cr);

    //buttons


    //displays
    left_display(&cdu_data, cr);
    right_display(&cdu_data, cr);
    from_display(&cdu_data, cr);
    to_display(&cdu_data, cr);


}

void get_plugin_path()
{
	XPLMGetPluginInfo(XPLMGetMyID(), NULL, plugindir, NULL, NULL);

	fix_pathsep(plugindir);

	/* cut off the trailing path component (our filename) */
	if ((p = strrchr(plugindir, DIRSEP)) != NULL)
	    *p = '\0';

	if ((p = strrchr(plugindir, DIRSEP)) != NULL) 
		{
	    	if (strcmp(p + 1, "64") == 0 || strcmp(p + 1, "32") == 0 ||
	    	    strcmp(p + 1, "win_x64") == 0 ||
	    	    strcmp(p + 1, "mac_x64") == 0 ||
	    	    strcmp(p + 1, "lin_x64") == 0) 
				{
	    	        *p = '\0';
				}	
		}
	logMsg("plugindir: %s", plugindir);
}

void menu_handler(void * in_menu_ref, void * in_item_ref)
{
	
	if(!strcmp((const char *)in_item_ref, "Menu Item 1"))
	{
			if(XPLMGetWindowIsVisible(g_window[0]))
			{
				XPLMSetWindowIsVisible(g_window[0], 0);
			}
			else if (!XPLMGetWindowIsVisible(g_window[0]))
			{
				XPLMSetWindowIsVisible(g_window[0], 1);
			}
	}
		if(!strcmp((const char *)in_item_ref, "Menu Item 2"))
	{
			if(XPLMGetWindowIsVisible(g_window[1]))
			{
				XPLMSetWindowIsVisible(g_window[1], 0);
			}
			else if (!XPLMGetWindowIsVisible(g_window[1]))
			{
				XPLMSetWindowIsVisible(g_window[1], 1);
			}
	}
		if(!strcmp((const char *)in_item_ref, "Menu Item 3"))
	{
			if(XPLMGetWindowIsVisible(g_window[2]))
			{
				XPLMSetWindowIsVisible(g_window[2], 0);
			}
			else if (!XPLMGetWindowIsVisible(g_window[2]))
			{
				XPLMSetWindowIsVisible(g_window[2], 1);
			}
	}
}
int	handle_mouse(XPLMWindowID in_window_id, int x, int y, XPLMMouseStatus is_down, void * in_refcon)
{
	if(is_down == xplm_MouseDown)
	{
		const int is_popped_out = XPLMWindowIsPoppedOut(in_window_id);
		if (!XPLMIsWindowInFront(in_window_id))
		{
			XPLMBringWindowToFront(in_window_id);
		}
		else if(coord_in_rect(x, y, g_pop_button_lbrt)) // user clicked the pop-in/pop-out button
		{
			XPLMSetWindowPositioningMode(in_window_id, is_popped_out ? xplm_WindowPositionFree : xplm_WindowPopOut, 0);
		}
		else if(coord_in_rect(x, y, g_position_button_lbrt)) // user clicked the "move to lower left" button
		{
			// If we're popped out, and the user hits the "move to lower left" button,
			// we need to move them to the lower left of their OS's desktop space (units are pixels).
			// On the other hand, if we're a floating window inside of X-Plane, we need
			// to move to the lower left of the X-Plane global desktop (units are boxels).
			void (* get_geometry_fn)(XPLMWindowID, int *, int *, int *, int *) = is_popped_out ? &XPLMGetWindowGeometryOS : &XPLMGetWindowGeometry;
			int lbrt_current[4];
			get_geometry_fn(in_window_id, &lbrt_current[0], &lbrt_current[3], &lbrt_current[2], &lbrt_current[1]);

			int h = lbrt_current[3] - lbrt_current[1];
			int w = lbrt_current[2] - lbrt_current[0];
			void (* set_geometry_fn)(XPLMWindowID, int, int, int, int) = is_popped_out ? &XPLMSetWindowGeometryOS : &XPLMSetWindowGeometry;

			// Remember, the main monitor's origin is *not* guaranteed to be (0, 0), so we need to query for it in order to move the window to its lower left
			int bounds[4] = {0}; // left, bottom, right, top
			if(is_popped_out)
			{
				XPLMGetScreenBoundsGlobal(&bounds[0], &bounds[3], &bounds[2], &bounds[1]);
			}
			else
			{
				XPLMGetAllMonitorBoundsOS(receive_main_monitor_bounds, bounds);
			}

			set_geometry_fn(in_window_id, bounds[0], bounds[1] + h, bounds[0] + w, bounds[1]);
		}
	}
	return 1;
}

void receive_main_monitor_bounds(int inMonitorIndex, int inLeftBx, int inTopBx, int inRightBx, int inBottomBx, void * refcon)
{
	int * main_monitor_bounds = (int *)refcon;
	if(inMonitorIndex == 0) // the main monitor
	{
		main_monitor_bounds[0] = inLeftBx;
		main_monitor_bounds[1] = inBottomBx;
		main_monitor_bounds[2] = inRightBx;
		main_monitor_bounds[3] = inTopBx;
	}
}
void dummy_key_handler(XPLMWindowID in_window_id, char key, XPLMKeyFlags flags, char virtual_key, void * in_refcon, int losing_focus)
{}

int dummy_mouse_handler(XPLMWindowID in_window_id, int x, int y, int is_down, void * in_refcon) 
{
	return 0; 
}
XPLMCursorStatus dummy_cursor_status_handler(XPLMWindowID in_window_id, int x, int y, void * in_refcon) 
{
	return xplm_CursorDefault;
}
int dummy_wheel_handler(XPLMWindowID in_window_id, int x, int y, int wheel, int clicks, void * in_refcon) 
{
	return 0;
}
//these are the window setup parameters
void window_params_setup(XPLMCreateWindow_t param)
{
	param.structSize = sizeof(param);
	param.visible = 0;
	param.drawWindowFunc = draw;
	// Note on "dummy" handlers:
	// Even if we don't want to handle these events, we have to register a "do-nothing" callback for them
	param.handleMouseClickFunc = dummy_mouse_handler;
	param.handleRightClickFunc = dummy_mouse_handler;
	param.handleMouseWheelFunc = dummy_wheel_handler;
	param.handleKeyFunc = dummy_key_handler;
	param.handleCursorFunc = dummy_cursor_status_handler;
	param.refcon = NULL;
	param.layer = xplm_WindowLayerFloatingWindows;
	// Opt-in to styling our window like an X-Plane 11 native window
	// If you're on XPLM300, not XPLM301, swap this enum for the literal value 1.
	param.decorateAsFloatingWindow = xplm_WindowDecorationRoundRectangle;
	
	// Set the window's initial bounds
	// Note that we're not guaranteed that the main monitor's lower left is at (0, 0)...
	// We'll need to query for the global desktop bounds!
	int left, bottom, right, top;
	XPLMGetScreenBoundsGlobal(&left, &top, &right, &bottom);
	param.left = left + 50;
	param.bottom = bottom + 150;
	param.right = param.left + MIN_WIDTH;
	param.top = param.bottom + MIN_HEIGHT;
	//REPLACE 3 WITH NUM_IRU
	for(int i = 0; i < 3; ++i)
	{
		g_window[i] = XPLMCreateWindowEx(&param);
		XPLMSetWindowPositioningMode(g_window[i], xplm_WindowPositionFree, -1);
		XPLMSetWindowGravity(g_window[i], 0, 1, 0, 1); // As the X-Plane window resizes, keep our size constant, and our left and top edges in the same place relative to the window's left/top
		XPLMSetWindowResizingLimits(g_window[i], MIN_WIDTH, MIN_HEIGHT, WIDTH, HEIGHT); // Limit resizing our window: maintain a minimum width/height of 200 boxels and a max width/height of 500
		//XPLMSetWindowResizingLimits(g_window, WIDTH/4, HEIGHT/4, WIDTH*3/4, HEIGHT*3/4); // Limit resizing our window: maintain a minimum width/height of 200 boxels and a max width/height of 500
		char window_name[6];
		snprintf(window_name, sizeof(window_name), "CDU-%i", i+1);
		XPLMSetWindowTitle(g_window[i], window_name);
	}

}

void fini_cb()
{
	cairo_surface_destroy(cdu_bg);
    cairo_surface_destroy(warn_light_on);
    cairo_surface_destroy(warn_light_off);
    cairo_surface_destroy(key_pad_on);
    cairo_surface_destroy(key_pad_off);
    cairo_surface_destroy(bat_on );
    cairo_surface_destroy(bat_off);
    cairo_surface_destroy(alert_on );
    cairo_surface_destroy(alert_off);
    cairo_surface_destroy(insert_on);
    cairo_surface_destroy(insert_off );
    cairo_surface_destroy(remote_on);
    cairo_surface_destroy(remote_off );
    cairo_surface_destroy(hold_on);
    cairo_surface_destroy(hold_off);
    cairo_surface_destroy(wyptchg_on );
    cairo_surface_destroy(wyptchg_off);
    cairo_surface_destroy(wpt_num_digits);
    cairo_surface_destroy(wpt_bkg);
    cairo_surface_destroy(data_selector_knob);
	
}

cairo_surface_t* load_image(const char* img_path)
{
	char* path = mkpathname(plugindir, img_path, NULL);
	logMsg("Path loaded: %s", path);
	cairo_surface_t *surface = cairo_image_surface_create_from_png(path);
	lacf_free(path);
	return surface;
}

void image_path_setup()
{
	cdu_bg = load_image(cdu_path);
	warn_light_on = load_image(warn_light_on_path);
	warn_light_off = load_image(warn_light_off_path);

	key_pad_on = load_image(key_pad_on_path);
	key_pad_off = load_image(key_pad_off_path);

	bat_on = load_image(bat_on_path);
	bat_off = load_image(bat_off_path);

	alert_on = load_image(alert_on_path);
	alert_off = load_image(alert_off_path);

	insert_on = load_image(insert_on_path);
	insert_off = load_image(insert_off_path);

	remote_on = load_image(remote_on_path);
	remote_off = load_image(remote_off_path);

	hold_on = load_image(hold_on_path);
	hold_off = load_image(hold_off_path);

	wyptchg_on = load_image(wyptchg_on_path);
	wyptchg_off = load_image(wyptchg_off_path);

	wpt_num_digits = load_image(wpt_num_digits_path);
	wpt_bkg = load_image(wpt_bkg_path);
	data_selector_knob = load_image(data_selector_knob_path);
	auto_man_knob = load_image(auto_man_knob_path);
	clear_off = load_image(clear_off_path);
	clear_on = load_image(clear_on_path);
	dimmer = load_image(dimmer_path);
	logMsg("Images Loaded");
}
char* load_font(const char* font_path)
{
	char* path = mkpathname(plugindir, font_path, NULL);
	char* return_path = path;
	lacf_free(path);
	return return_path;
}

void init_font()
{
    seven_seg_full_path = mkpathname(plugindir, seven_seg_fnt_path, NULL);
	logMsg("SEVEN_SEG_PATH: %s", seven_seg_full_path);
	sans_full_path = mkpathname(plugindir, sans_fnt_path, NULL);
	logMsg("SANS_PATH: %s", sans_full_path);

	status = FT_Init_FreeType(&value);
    if (status != 0) 
    {
        logMsg("Error %d opening library.\n", status);
        ASSERT(status != 0);
    }
    status = FT_New_Face (value, seven_seg_full_path, 0, &ft_seven_seg);

    if (status != 0) 
    {
        logMsg("Error %d opening %s.\n", status, seven_seg_full_path);
        ASSERT(status != 0);
    }

    status = FT_Init_FreeType(&value);
    if (status != 0) 
    {
        logMsg("Error %d opening library.\n", status);
        ASSERT(status != 0);
    }
    status = FT_New_Face (value, sans_full_path, 0, &ft_sans);

    if (status != 0) 
    {
        logMsg("Error %d opening %s.\n", status, sans_full_path);
        ASSERT(status != 0);
    }

    seven_seg = cairo_ft_font_face_create_for_ft_face(ft_seven_seg, 0);
    sans = cairo_ft_font_face_create_for_ft_face(ft_sans, 0);
}

void n_s_light(bool on, bool north, bool test, cairo_t *cr)
{
    cairo_set_font_face(cr, sans);
    cairo_set_font_size(cr, 60.0);
    
    cairo_set_source_rgb(cr, text_color.red, text_color.green, text_color.blue);

    if(on && north && !test)
    {
        cairo_move_to(cr, 526, 103);
        cairo_show_text(cr, "N");
        
        cairo_set_source_rgb(cr, off_color.red, off_color.green, off_color.blue);
        cairo_move_to(cr, 529, 157);
        cairo_show_text(cr, "S");
    }
    else if (on && !north && !test) 
    {
        cairo_move_to(cr, 529, 157);
        cairo_show_text(cr, "S");

        cairo_set_source_rgb(cr, off_color.red, off_color.green, off_color.blue);
        cairo_move_to(cr, 526, 103);
        cairo_show_text(cr, "N");
    }

    else if (on && test) 
    {
        cairo_move_to(cr, 526, 103);
        cairo_show_text(cr, "N");
        cairo_move_to(cr, 529, 157);
        cairo_show_text(cr, "S");
    }

    else if(!on)
    {
        cairo_set_source_rgb(cr, off_color.red, off_color.green, off_color.blue);
        cairo_move_to(cr, 526, 103);
        cairo_show_text(cr, "N"); 
        cairo_move_to(cr, 529, 157);
        cairo_show_text(cr, "S");                 
    }
 
}

void e_w_light(bool on, bool east, bool test, cairo_t *cr)
{
    cairo_set_font_face(cr, sans);
    cairo_set_font_size(cr, 60.0);    
    cairo_set_source_rgb(cr, text_color.red, text_color.green, text_color.blue);



    if(on && east && !test)
    {
        cairo_move_to(cr, 1118, 103);
        cairo_show_text(cr, "E");
        cairo_set_source_rgb(cr, off_color.red, off_color.green, off_color.blue);
        cairo_move_to(cr, 1111, 157);
        cairo_show_text(cr, "W");        
    }
    if(on && !east && !test)
    {
        cairo_move_to(cr, 1111, 157);
        cairo_show_text(cr, "W");
        cairo_set_source_rgb(cr, off_color.red, off_color.green, off_color.blue);
        cairo_move_to(cr, 1118, 103);
        cairo_show_text(cr, "E");        
    }
    if(on && test)
    {
        cairo_set_source_rgb(cr, text_color.red, text_color.green, text_color.blue);
        cairo_move_to(cr, 1118, 103);
        cairo_show_text(cr, "E");
        cairo_move_to(cr, 1111, 157);
        cairo_show_text(cr, "W");
    }
    else if(!on)
    {
        cairo_set_source_rgb(cr, off_color.red, off_color.green, off_color.blue);
        cairo_move_to(cr, 1118, 103);
        cairo_show_text(cr, "E");
        cairo_move_to(cr, 1111, 157);
        cairo_show_text(cr, "W");            
    }  
}

void left_display(cdu_t *cdu, cairo_t *cr)
{   

    //display mask for blank segments
    cairo_set_font_face(cr, seven_seg);
    cairo_set_font_size(cr, 97.0);
    
    if(!cdu->displays.left_decimal)
    {
        cairo_set_source_rgb(cr, off_color.red, off_color.green, off_color.blue); //set to "off" color
        cairo_move_to(cr, 441, 158);
        cairo_show_text(cr, "."); 
    }
    
    if(!cdu->displays.left_degrees)
    {
        cairo_set_source_rgb(cr, off_color.red, off_color.green, off_color.blue); //set to off color
        cairo_move_to(cr, 290, 71);
        cairo_show_text(cr, ".");
    }
    if(!cdu->displays.left_deg2)
    {
        cairo_set_source_rgb(cr, off_color.red, off_color.green, off_color.blue); //set to off color
        cairo_move_to(cr, 526, 71);
        cairo_show_text(cr, ".");
    } 
    if(!cdu->displays.left_2dec_deg)
    {
        cairo_set_source_rgb(cr, off_color.red, off_color.green, off_color.blue); //set to "off" color
        cairo_move_to(cr, 362, 158);
        cairo_show_text(cr, ".");
    }
   

    if(cdu->displays.left_display_on)
    {        


        if(cdu->displays.left_degrees)
        {
            cairo_set_source_rgb(cr, on_color.red, on_color.green, on_color.blue); //set to off color
            cairo_move_to(cr, 290, 71);
            cairo_show_text(cr, ".");
        }

        if(cdu->displays.left_deg2)
        {
            cairo_set_source_rgb(cr, text_color.red, text_color.green, text_color.blue); //set to "off" color
            cairo_move_to(cr, 526, 71);
            cairo_show_text(cr, "."); 
        }

        if(cdu->displays.left_2dec_deg)
        {
            cairo_set_source_rgb(cr, text_color.red, text_color.green, text_color.blue); //set to "on" color
            cairo_move_to(cr, 362, 158);
            cairo_show_text(cr, ".");             
        }
        if(cdu->displays.left_decimal)
        {
            cairo_set_source_rgb(cr, text_color.red, text_color.green, text_color.blue); //set to "off" color
            cairo_move_to(cr, 441, 158);
            cairo_show_text(cr, ".");
        }        


        cairo_set_source_rgb(cr, text_color.red, text_color.green, text_color.blue); //set to "on" color
        cairo_move_to(cr, 124, 157); //move to start of display line 

        char buffer[1];

        for (int i = 0; i < 5; ++i)
        {
            sprintf(buffer, "%c", cdu->displays.left_digits[i]);
            cairo_show_text(cr, buffer);
        }  
    }

    n_s_light(cdu->displays.ns_on, cdu->displays.north_on, cdu->test_pressed, cr);
     
}

void right_display(cdu_t *cdu, cairo_t *cr)
{
    cairo_set_font_face(cr, seven_seg);
    cairo_set_font_size(cr, 97.0);
    
    if(!cdu->displays.right_decimal)
    {
        cairo_set_source_rgb(cr, off_color.red, off_color.green, off_color.blue); //set to "off" color
        cairo_move_to(cr, 1035, 157);
        cairo_show_text(cr, ".");           
    }
    if(!cdu->displays.right_degrees)
    {
        cairo_set_source_rgb(cr, off_color.red, off_color.green, off_color.blue); //set to off color
        cairo_move_to(cr, 883, 71);
        cairo_show_text(cr, ".");
    }

    if(!cdu->displays.right_dec_deg)
    {
        cairo_set_source_rgb(cr, off_color.red, off_color.green, off_color.blue); //set to off color
        cairo_move_to(cr, 1118, 71);
        cairo_show_text(cr, ".");
    }     

    if(cdu->displays.right_display_on)
    {    
        if(cdu->displays.right_degrees)
        {
            cairo_set_source_rgb(cr, text_color.red, text_color.green, text_color.blue); //set to "on" color
            cairo_move_to(cr, 883, 71);
            cairo_show_text(cr, ".");
        }


        if(cdu->displays.right_decimal)
        {
            cairo_set_source_rgb(cr, text_color.red, text_color.green, text_color.blue); //set to "on" color
            cairo_move_to(cr, 1035, 157);
            cairo_show_text(cr, ".");        
        }    
        
        if(cdu->displays.right_dec_deg)
        {
            cairo_set_source_rgb(cr, text_color.red, text_color.green, text_color.blue); //set to "off" color
            cairo_move_to(cr, 1118, 71);
            cairo_show_text(cr, "."); 
        }

        cairo_move_to(cr, 638, 157);
        cairo_set_source_rgb(cr, text_color.red, text_color.green, text_color.blue); //set to "on" color    
        char buffer[1];

        for (int i = 0; i < 6; ++i)
        {
            sprintf(buffer, "%c", cdu->displays.right_digits[i]);
            cairo_show_text(cr, buffer);
        }  
    }
    
    e_w_light(cdu->displays.ew_on, cdu->displays.east_on, cdu->test_pressed, cr);
}

void from_display(cdu_t *cdu, cairo_t *cr) //add flashing function
{
    cairo_set_font_face(cr, seven_seg);
    cairo_set_font_size(cr, 92.0);

    if(cdu->displays.nav_from_on)
    {
        cairo_move_to(cr, 491, 487);
        cairo_set_source_rgb(cr, text_color.red, text_color.green, text_color.blue);
        std::string tmp = std::to_string(cdu->displays.nav_from);
        const char * from = tmp.c_str();
        cairo_show_text(cr, from);
    }
}

void to_display(cdu_t *cdu, cairo_t *cr) //add flashing function
{
    cairo_set_font_face(cr, seven_seg);
    cairo_set_font_size(cr, 92.0);

    if(cdu->displays.nav_to_on)
    {
        std::string tmp = std::to_string(cdu->displays.nav_to);
        const char * to = tmp.c_str();
    
        cairo_set_source_rgb(cr, text_color.red, text_color.green, text_color.blue);
        cairo_move_to(cr, 566, 487);
        cairo_show_text(cr, to);
    }

}

void warn_lit(cdu_t *cdu, cairo_t *cr)
{
    x = 1055;
    y = 229;    
    if(cdu->warn_on)
    {
        cairo_save(cr);
        cairo_translate(cr, x, y);
        cairo_set_source_surface(cr, warn_light_on, 0, 0);
        cairo_paint(cr);
        cairo_restore(cr);
    }
    if(!cdu->warn_on)
    {
        cairo_save(cr);
        cairo_translate(cr, x, y);
        cairo_set_source_surface(cr, warn_light_off, 0, 0);
        cairo_paint(cr);
        cairo_restore(cr);
    }    
}

void numpad_lit(cdu_t *cdu, cairo_t *cr)
{
    x = 737;
    y = 400;    
    if(cdu->keypad_on)
    {
        cairo_save(cr);
        cairo_translate(cr, x, y);
        cairo_set_source_surface(cr, key_pad_on, 0, 0);
        cairo_paint(cr);
        cairo_restore(cr);
    }
    if(!cdu->keypad_on)
    {
        cairo_save(cr);
        cairo_translate(cr, x, y);
        cairo_set_source_surface(cr, key_pad_off, 0, 0);
        cairo_paint(cr);
        cairo_restore(cr);
    }    
}

void bat_lit(cdu_t *cdu, cairo_t *cr)
{
    x = 898;
    y = 229;
    if(cdu->bat_on)
    {
        cairo_save(cr);
        cairo_translate(cr, x, y);
        cairo_set_source_surface(cr, bat_on, 0, 0);
        cairo_paint(cr);
        cairo_restore(cr);
    }
    if(!cdu->bat_on)
    {
        cairo_save(cr);
        cairo_translate(cr, x, y);
        cairo_set_source_surface(cr, bat_off, 0, 0);
        cairo_paint(cr);
        cairo_restore(cr);
    }    
}

void alert_lit(cdu_t *cdu, cairo_t *cr)
{
    x = 741;
    y = 229;
    if(cdu->alert_on)
    {

        cairo_save(cr);
        cairo_translate(cr, x, y);
        cairo_set_source_surface(cr, alert_on, 0, 0);
        cairo_paint(cr);
        cairo_restore(cr);
    }
    if(!cdu->alert_on)
    {
        cairo_save(cr);
        cairo_translate(cr, x, y);
        cairo_set_source_surface(cr, alert_off, 0, 0);
        cairo_paint(cr);
        cairo_restore(cr);
    }    
}

void insert_lit(cdu_t *cdu, cairo_t *cr)
{
    x = 485;
    y = 229;
    if(cdu->insert_on)
    {

        cairo_save(cr);
        cairo_translate(cr, x, y);
        cairo_set_source_surface(cr, insert_on, 0, 0);
        cairo_paint(cr);
        cairo_restore(cr);
    }
    if(!cdu->insert_on)
    {
        cairo_save(cr);
        cairo_translate(cr, x, y);
        cairo_set_source_surface(cr, insert_off, 0, 0);
        cairo_paint(cr);
        cairo_restore(cr);
    }    
}

void remote_lit(cdu_t *cdu, cairo_t *cr)
{
    x = 281;
    y = 229;
    if(cdu->remote_on)
    {

        cairo_save(cr);
        cairo_translate(cr, x, y);
        cairo_set_source_surface(cr, remote_on, 0, 0);
        cairo_paint(cr);
        cairo_restore(cr);
    }
    if(!cdu->remote_on)
    {
        cairo_save(cr);
        cairo_translate(cr, x, y);
        cairo_set_source_surface(cr, remote_off, 0, 0);
        cairo_paint(cr);
        cairo_restore(cr);
    }    
}

void hold_lit(cdu_t *cdu, cairo_t *cr)
{
    x = 124;
    y = 229;

    if(cdu->hold_on)
    {

        cairo_save(cr);
        cairo_translate(cr, x, y);
        cairo_set_source_surface(cr, hold_on, 0, 0);
        cairo_paint(cr);
        cairo_restore(cr);
    }
    if(!cdu->hold_on)
    {
        cairo_save(cr);
        cairo_translate(cr, x, y);
        cairo_set_source_surface(cr, hold_off, 0, 0);
        cairo_paint(cr);
        cairo_restore(cr);
    }    
}

void wyptchg_lit(cdu_t *cdu, cairo_t *cr)
{
    x = 560;
    y = 552;
    if(cdu->wypt_chg_on)
    {
        cairo_save(cr);
        cairo_translate(cr, x, y);
        cairo_set_source_surface(cr, wyptchg_on, 0, 0);
        cairo_paint(cr);
        cairo_restore(cr);
    }
    if(!cdu->wypt_chg_on)
    {
        cairo_save(cr);
        cairo_translate(cr, x, y);
        cairo_set_source_surface(cr, wyptchg_off, 0, 0);
        cairo_paint(cr);
        cairo_restore(cr);
    }    
}

void clear_lit(cdu_t *cdu, cairo_t *cr)
{
    int x = 930;
    int y = 845;

    if(cdu->clear_on)
    {
        cairo_save(cr);
        cairo_translate(cr, x, y);
        cairo_set_source_surface(cr, clear_on, 0, 0);
        cairo_paint(cr);
        cairo_restore(cr);
    }
    if(!cdu->clear_on)
    {
        cairo_save(cr);
        cairo_translate(cr, x, y);
        cairo_set_source_surface(cr, clear_off, 0, 0);
        cairo_paint(cr);
        cairo_restore(cr);
    }    
}

void wpt_sel(cdu_t *cdu, cairo_t *cr)
{
    int num = cdu->wpt_num;
    y = 360;
    int offset[10] = {0, 60, 113, 168, 223, 279, 334, 387, 443, 498};
    x = 253 - offset[num];
    cairo_save(cr);
    cairo_translate(cr, x, y);
    cairo_set_source_surface(cr, wpt_num_digits, 0, 0);
    cairo_paint(cr);
    cairo_restore(cr);    
}

void wpt_sel_bkg(cairo_t *cr)
{
    x = 227;
    y = 369;
    cairo_save(cr);
    cairo_translate(cr, x, y);
    cairo_set_source_surface(cr, wpt_bkg, 0, 0);
    cairo_paint(cr);
    cairo_restore(cr);
}

void data_selector(cdu_t *cdu, cairo_t *cr)
{
    float width_2 = 116;
    float height_2 = 117.5;
    int x = 247;
    int y = 730;
    float radians = DEG2RAD(cdu->data_sel);
    cairo_save(cr);

    cairo_translate(cr, x, y);
    cairo_rotate(cr, radians);
    cairo_translate(cr, -width_2, -height_2);

    cairo_set_source_surface(cr, data_selector_knob, 0,0);
    cairo_paint(cr);
    cairo_restore(cr);  

}

void text_dimmer(cdu_t *cdu, cairo_t *cr)
{
    
    float max = 1;
    float min = 0.5;

    float dim_val = (((90 - cdu->brightness_knob_ang) / 90) * (max - min)) + min;
    
    text_color.red = on_color.red * dim_val;
    text_color.green = on_color.green * dim_val;
    text_color.blue = on_color.blue * dim_val;

}

void brightness_knob(cdu_t *cdu, cairo_t *cr)
{
    float width_2 = 54;
    float height_2 = 54;
    int x = 247;
    int y = 730;
    float radians = -DEG2RAD(cdu->brightness_knob_ang);
    cairo_save(cr);

    cairo_translate(cr, x, y);
    cairo_rotate(cr, radians);
    cairo_translate(cr, -width_2, -height_2);

    cairo_set_source_surface(cr, dimmer, 0,0);
    cairo_paint(cr);
    cairo_restore(cr);
    text_dimmer(cdu, cr);
}

void auto_man(cdu_t *cdu, cairo_t *cr)
{
    float width_2 = 56;
    float height_2 = 58;
    int x = 477;
    int y = 913;
    float radians = DEG2RAD(cdu->sequence);
    cairo_save(cr);

    cairo_translate(cr, x, y);
    cairo_rotate(cr, radians);
    cairo_translate(cr, -width_2, -height_2);
    cairo_set_source_surface(cr, auto_man_knob, 0,0);
    cairo_paint(cr);
    cairo_restore(cr);

}

void screen_format(cdu_t *cdu)
{
    switch (cdu->data_sel) 
    {
        case TKGS:
            cdu->displays.left_degrees = false;
            cdu->displays.right_degrees = false;
            cdu->displays.left_decimal = true;
            cdu->displays.right_decimal = false;
            cdu->displays.left_deg2 = true;
            cdu->displays.right_dec_deg = false;
            cdu->displays.nav_from_on = true;
            cdu->displays.nav_to_on = true;
            break;

        case HDG:
            cdu->displays.left_degrees = false;
            cdu->displays.right_degrees = false;
            cdu->displays.left_decimal = true;
            cdu->displays.right_decimal = false;
            cdu->displays.left_deg2 = true;
            cdu->displays.right_dec_deg = false;
            cdu->displays.nav_from_on = true;
            cdu->displays.nav_to_on = true;
            break;
        case XTK:
            cdu->displays.left_2dec_deg = true;
            cdu->displays.left_deg2 = false;
            cdu->displays.left_decimal = false;
            cdu->displays.left_degrees = false;
            cdu->displays.ns_on = false;

            cdu->displays.right_degrees = false;
            cdu->displays.right_dec_deg = true;
            cdu->displays.right_decimal = false;
            cdu->displays.ew_on = false;

            cdu->displays.nav_from_on = true;
            cdu->displays.nav_to_on = true;            
            break;

        case POS:
            cdu->displays.left_degrees = true;
            cdu->displays.left_decimal = true;
            cdu->displays.ns_on = true;
            cdu->displays.left_2dec_deg = false;
            cdu->displays.left_deg2 = false;

            cdu->displays.right_degrees = true;
            cdu->displays.right_decimal = true;
            cdu->displays.ew_on = true;
            cdu->displays.right_dec_deg = false;

            cdu->displays.nav_from_on = true;
            cdu->displays.nav_to_on = true;            
            break;

        case WAYPT:
            cdu->displays.left_degrees = true;
            cdu->displays.left_decimal = true;
            cdu->displays.ns_on = true;
            cdu->displays.left_2dec_deg = false;
            cdu->displays.left_deg2 = false;

            cdu->displays.right_degrees = true;
            cdu->displays.right_decimal = true;
            cdu->displays.ew_on = true;
            cdu->displays.right_dec_deg = false;

            cdu->displays.nav_from_on = true;
            cdu->displays.nav_to_on = true;            
            break;

        case DIS:
            cdu->displays.left_deg2 = false;
            cdu->displays.left_2dec_deg = false;
            cdu->displays.left_decimal = false;
            cdu->displays.left_degrees = false;
            cdu->displays.ns_on = false;

            cdu->displays.right_decimal = true;
            cdu->displays.right_dec_deg = false;
            cdu->displays.right_degrees = false;
            cdu->displays.ew_on = false;

            cdu->displays.nav_from_on = true;
            cdu->displays.nav_to_on = true;            
            break;

        case WIND:
            cdu->displays.left_decimal = false;
            cdu->displays.left_2dec_deg = false;
            cdu->displays.left_degrees = false;

            if(cdu->hold_pressed)
            {
                cdu->displays.left_deg2 = false;
                cdu->displays.ns_on = true;
                cdu->displays.ew_on = true;
            }
            else if(!cdu->hold_pressed)
            {
                cdu->displays.left_deg2 = true;                
                cdu->displays.ns_on = false;
                cdu->displays.ew_on = false;
            }

            cdu->displays.right_dec_deg = false;
            cdu->displays.right_decimal = false;
            cdu->displays.right_degrees = false;

            cdu->displays.nav_from_on = true;
            cdu->displays.nav_to_on = true;            
            break;
        
        case STS:
            cdu->displays.left_deg2 = true;
            cdu->displays.left_decimal = false;
            cdu->displays.left_2dec_deg = false;
            cdu->displays.left_degrees = false;
            cdu->displays.ns_on = false;

            cdu->displays.right_decimal = false;
            cdu->displays.right_dec_deg = false;
            cdu->displays.right_degrees = false;
            cdu->displays.ew_on = false;

            cdu->displays.nav_from_on = true;
            cdu->displays.nav_to_on = true;            
            break;
    }
}


void test_mode_set(cdu_t *cdu, cairo_t *cr)
{

    char test_vals[14];

    for(int i = 0; i < 14; ++i)
    {
        if(i != 13)
        {
            test_vals[i] = '8';
        }
        if (i == 14)
        {
            test_vals[i] = '\0';
        }
    }
	load_digits(cdu, test_vals, cdu->displays.left_digits, cdu->displays.right_digits);

    cdu->displays.left_degrees = true;
    cdu->displays.right_degrees = true;
    cdu->displays.left_decimal = true;
    cdu->displays.right_decimal = true;
    cdu->displays.left_deg2 = true;
    cdu->displays.right_dec_deg = true;
    cdu->displays.left_2dec_deg = true;

    cdu->displays.nav_from_on = true;
    cdu->displays.nav_to_on = true;
    cdu->displays.nav_from = 8;
    cdu->displays.nav_to = 8;

    cdu->hold_on = true;
    cdu->remote_on = true;
    cdu->insert_on = true;
    cdu->alert_on = true;
    cdu->bat_on = true;
    cdu->warn_on = true;
    cdu->wypt_chg_on = true;
    cdu->clear_on = true;
    cdu->keypad_on = true;


    left_display(cdu, cr);
    right_display(cdu, cr);
    from_display(cdu, cr);
    to_display(cdu, cr);
    hold_lit(cdu, cr);
    remote_lit(cdu, cr);
    insert_lit(cdu, cr);
    alert_lit(cdu, cr);
    bat_lit(cdu, cr);
    warn_lit(cdu, cr);
    wyptchg_lit(cdu, cr);
    clear_lit(cdu, cr);
    numpad_lit(cdu, cr);   

}

void display_logic(cdu_t *cdu, cairo_t *cr)
{
    if(cdu->test_pressed)
    {
        test_mode_set(cdu, cr);
    }
    else if (!cdu->test_pressed)
    {
        screen_format(cdu);
        hold_lit(cdu, cr);
        remote_lit(cdu, cr);
        insert_lit(cdu, cr);
        alert_lit(cdu, cr);
        bat_lit(cdu, cr);
        warn_lit(cdu, cr);
        wyptchg_lit(cdu, cr);
        clear_lit(cdu, cr);
        numpad_lit(cdu, cr);        
    }
}
