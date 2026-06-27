/*
   Fork: AUTO TRIM flight mode (mode 27).

   Inherits ModeCourseHold so the plane flies a held-track straight line
   while the pilot trims by stick.  On entry the servos_auto_trim loop is
   armed via auto_trim.run; on exit it's disarmed.  Otherwise the mode
   behaves like Course Hold.

   Ported from ArduCustom 9e84d8a1ee (shellixyz, v11.2).  The fork's
   _exit() body had a typo (called ModeCourseHold::_enter() instead of
   _exit()); we call _exit() here.
*/

#include "mode.h"
#include "Plane.h"

bool ModeAutoTrim::_enter()
{
    if (!ModeCourseHold::_enter()) {
        return false;
    }
    plane.auto_trim.run = true;
    return true;
}

void ModeAutoTrim::_exit()
{
    plane.auto_trim.run = false;
}
