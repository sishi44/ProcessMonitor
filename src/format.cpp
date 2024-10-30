#include <string>
#include <sstream>
#include <iomanip>

#include "format.h"

using std::string;
using std::to_string;

// Self defined helper function
// Force to display two digital for hours, minutes and seconds
string Format::TwoDigital(int num) {
    std::stringstream ss;
    ss << std::setw(2) << std::setfill('0') << num;

    return ss.str();
}

// Complete this helper function
// INPUT: Long int measuring seconds
// OUTPUT: HH:MM:SS
string Format::ElapsedTime(long seconds) {
    int hours, minutes;

    hours = seconds / 3600;
    seconds = seconds % 3600;
    minutes = seconds / 60;
    seconds = seconds % 60;

    return Format::TwoDigital(hours) + ":" + Format::TwoDigital(minutes) + ":" +
           Format::TwoDigital(seconds);
}