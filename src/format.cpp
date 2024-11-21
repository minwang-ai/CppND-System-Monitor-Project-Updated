#include <string>
#include <iomanip>
#include <sstream>

#include "format.h"

using std::string;

/**
 * @brief Converts a time duration in seconds to a formatted string in HH:MM:SS format.
 *
 * This function takes a total number of seconds and converts it into a formatted string
 * representing hours, minutes, and seconds, with each component zero-padded to two digits.
 *
 * @param seconds long: The total number of seconds to be converted.
 * @return std::string: A string representing the time duration in HH:MM:SS format, with each component
 *         zero-padded to two digits.
 */
string Format::ElapsedTime(long seconds) { 
    long hours = seconds / 3600;
    seconds %= 3600;
    long minutes = seconds / 60;
    seconds %= 60;
    std::ostringstream stream;
    stream << std::setw(2) << std::setfill('0') << hours << ":"
           << std::setw(2) << std::setfill('0') << minutes << ":"
           << std::setw(2) << std::setfill('0') << seconds;
    return stream.str();
}