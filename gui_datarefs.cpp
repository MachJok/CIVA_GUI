#include "gui_datarefs.h"
#include "acfutils/log.h"
#include "gui_functions.h"
#include "gui_structs.h"
#include "acfutils/dr.h"
#include "acfutils/dr_cmd_reg.h"
#include "gui_variables.h"

static dr_t Gui_Sim_Frame_Time;
static dr_t Gui_Sim_Paused;
static dr_t waypoint_pos[3];
static dr_t current_pos[3];
static dr_t heading_true[3];
static dr_t drift_ang[3];
static dr_t velocity_vect[3]; //track & gs
static dr_t nav_pos[3]; //current nav pos
static dr_t polar_wind[3]; //wind dir/mag
static dr_t track_ang_error[3];
static dr_t current_leg_dist[3];
static dr_t cross_track_err[3];


static dr_t warn_light[3];
static dr_t batt_light[3];
static dr_t auto_man_switch_light[3];
static dr_t remote_light[3];

//finish implementing datarefs
void LoadDataRefs()
{
    fdr_find(&Gui_Sim_Frame_Time, "sim/time/framerate_period");
	logMsg("Sim_Frame_Time passed");
    fdr_find(&Gui_Sim_Paused, "sim/time/paused");
	logMsg("Sim_Paused passed");
	for (int i = 0; i < 3; ++i)
	{
		fdr_find(&waypoint_pos[i], "omi/iru/%d/nav/waypoint_dm", i);
		fdr_find(&current_pos[i], "omi/iru/%d/nav_pos_dm",i);
		
	}
}

void GetDataRefs()
{
	sim_frame_time = dr_getf_prot(&Gui_Sim_Frame_Time);
	sim_paused = dr_geti(&Gui_Sim_Paused);
	for (int i = 0; i < 3; ++i)
	{
		dr_getbytes(&current_pos[i], current_pos_char[i], 0, sizeof(current_pos_char[i]));
		dr_getbytes(&waypoint_pos[i], waypoint_pos_char[i], 0, sizeof(waypoint_pos_char[i]));
		
	}
}

void SetDataRefs()
{

}