#ifndef LOGGING_H
#define LOGGING_H

#pragma once

#include <iostream>
#include <chrono>
#include <ctime>
#include <cstring>
#include "filesystem.h"

inline std::string current_time()
{
    time_t now = time(NULL);
    std::string buf;
    buf.resize(40);
    struct tm tstruct = *localtime(&now);
    strftime((char*)buf.data(), sizeof(buf), "%X", &tstruct);
    return buf;
}

#define DEBUG(x) std::cout << "[" << __FILENAME__ << " - " << current_time() << "] " << x << std::endl;

#endif