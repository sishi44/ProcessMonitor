#ifndef FORMAT_H
#define FORMAT_H

#include <string>

namespace Format {
std::string ElapsedTime(long times);  // See src/format.cpp
std::string TwoDigital(int num);
};                                    // namespace Format

#endif