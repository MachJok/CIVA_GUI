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
static dr_t polar_wind[3]; //wind dir/mag
static dr_t track_ang_error[3];
static dr_t current_leg_dist[3];
static dr_t current_leg_crs[3];
static dr_t cross_track_err[3];
static dr_t from_to[3];
static dr_t trk_gs[3];
static dr_t wind_vect[3];
static dr_t waypoint_sel[3];

static dr_t auto_man_switch[3];

static dr_t ins_warn_light[3];
static dr_t ins_batt_light[3];
static dr_t ins_alert_light[3];
static dr_t remote_light[3];

//finish implementing datarefs
void LoadDataRefs()
{
    fdr_find(&Gui_Sim_Frame_Time, "sim/time/framerate_period");
    fdr_find(&Gui_Sim_Paused, "sim/time/paused");
	for (int i = 0; i < 3; ++i) //REPLACE WITH NUM_IRU
	{
		fdr_find(&waypoint_pos[i], "omi/iru/%d/nav/waypoint_dm", i);
		fdr_find(&current_pos[i], "omi/iru/%d/nav_pos_dm",i); //replace with current pos
		fdr_find(&heading_true[i], "omi/iru/%d/heading", i);
		fdr_find(&drift_ang[i], "omi/iru/%d/drift_angle", i);
		fdr_find(&velocity_vect[i], "omi/iru/%d/ground_vect_ecef", i);
		fdr_find(&track_ang_error[i], "omi/iru/%d/nav/tke", i);
		fdr_find(&current_leg_dist[i], "omi/iru/%d/nav/wpt_dist", i);
		fdr_find(&current_leg_crs[i], "omi/iru/%d/nav/dsrtk", i);
		fdr_find(&cross_track_err[i], "omi/iru/%d/nav/tke", i);
		fdr_find(&from_to[i],"omi/iru/%d/nav/leg_frm_to", i);
		fdr_find(&trk_gs[i], "omi/iru/%d/ground_vel_vect", i);
		fdr_find(&wind_vect[i],"omi/iru/%d/wind_vect",i);
		
		fdr_find(&waypoint_sel[i],"omi/iru/%d/nav/waypoint_selector", i);
		fdr_find(&auto_man_switch[i], "omi/iru/%d/nav/auto_man_switch",i);


		fdr_find(&ins_alert_light[i], "omi/iru/%d/alert_light",i);
		fdr_find(&ins_warn_light[i], "omi/iru/%d/warn_light", i);
		fdr_find(&ins_batt_light[i], "omi/iru/%d/battery_light", i);
	}
	logMsg("datarefs loaded");
}

void GetDataRefs()
{
	sim_frame_time = dr_getf_prot(&Gui_Sim_Frame_Time);
	sim_paused = dr_geti(&Gui_Sim_Paused);
	for (int i = 0; i < 3; ++i) //REPLACE with NUM_IRU
	{
		dr_getbytes(&current_pos[i], current_pos_char[i], 0, sizeof(current_pos_char[i]));
		dr_getbytes(&waypoint_pos[i], waypoint_pos_char[i], 0, sizeof(waypoint_pos_char[i]));
		cdu_warn_light[i] = dr_geti(&ins_warn_light[i]);
		cdu_batt_light[i] = dr_geti(&ins_batt_light[i]);
		cdu_alert_light[i] = dr_geti(&ins_alert_light[i]);

		dr_getvi(&from_to[i], nav_from_to[i], 0, sizeof(from_to[i]));
		waypoint_sel_num[i] = dr_geti(&waypoint_sel[i]);
		auto_man_knob_dr[i] = dr_geti(&auto_man_switch[i]);
		
	}
}

void SetDataRefs()
{

}