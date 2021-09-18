#include "gui_functions.h"

void load_digits(cdu_t* cdu, char display_str[14], char left_digits[5], char right_digits[6])
{
    for (int i = 0; i < 14; ++i)
    {
        if (i < 5)
        {
            left_digits[i] = display_str[i];
        }

        if (i > 4 && i < 11)
        {
            right_digits[i-5] = display_str[i];
        }

        if (i == 11 && display_str[i] == 'N')
        {
            cdu->displays.north_on = true;
        }
        if (i == 11 && display_str[i] == 'S')
        {
            cdu->displays.north_on = false;
        }
        if (i == 12 && display_str[i] == 'E')
        {
            cdu->displays.east_on = true;
        }
        if (i == 12 && display_str[i] == 'W')
        {
            cdu->displays.east_on = false;
        }        
    }
}
