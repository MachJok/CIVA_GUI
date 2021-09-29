#ifndef GUI_STRUCTS_H
#define GUI_STRUCTS_H

struct color_t
{
    float red;
    float green;
    float blue;
};

enum data_selector_enum
{
    TKGS = 0,
    HDG = 38,
    XTK = 75,
    POS = 111,
    WAYPT = 145,
    DIS = 183,
    WIND = 220,
    STS = 255
};
enum auto_man_knob_enum
{
    automatic = 0,
    manual = 90
};

struct display_data_t
{
    bool left_display_on;
    bool right_display_on;
    char left_digits[5];
    bool left_degrees;
    bool left_decimal;
    bool left_deg2;
    bool left_2dec_deg;
    bool ns_on;
    bool north_on;
    char right_digits[6];
    bool right_degrees;
    bool right_decimal;
    bool right_dec_deg;
    bool ew_on;
    bool east_on;
    double left_size;
    double right_size;

    bool nav_from_on;
    int nav_from;

    bool nav_to_on;
    int nav_to;

};

struct cdu_t
{
    display_data_t displays;

    enum data_selector_enum data_sel;
    enum auto_man_knob_enum sequence;

    int wpt_num;
	bool auto_man;
    bool hold_pressed;
    bool remote_pressed;
    bool insert_pressed;
    bool wypt_chg_pressed;
    bool one_pressed;
    bool two_pressed;
    bool three_pressed;
    bool four_pressed;
    bool five_pressed;
    bool six_pressed;
    bool seven_pressed;
    bool eight_pressed;
    bool nine_pressed;
    bool zero_pressed;
    bool test_pressed;
    bool hold_on;
    bool remote_on;
    bool insert_on;
    bool alert_on;
    bool bat_on;
    bool warn_on;
    bool wypt_chg_on;
    bool clear_on;
    bool keypad_on;
    float brightness_knob_ang;
};


const color_t on_color = {1, 0.737, 0};
const color_t off_color = {0.141, 0.141, 0.141};





#endif