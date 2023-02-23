#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#pragma once

#include <filesystem>
#include <fstream>
#include <string>

#define __FILENAME__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)

namespace fs = std::filesystem;

inline bool file_exists(const std::string& fname) 
{
    if (FILE *file = fopen(fname.c_str(), "r")) 
    {
        fclose(file);
        return true;
    } 
    else 
    {
        return false;
    }   
}

inline std::string read_file_to_string(fs::path path)
{
    // Open the stream to 'lock' the file.
    std::ifstream f(path, std::ios::in | std::ios::binary);

    // Obtain the size of the file.
    const auto sz = fs::file_size(path);

    // Create a buffer.
    std::string result(sz, '\0');

    // Read the whole file into the buffer.
    f.read(result.data(), sz);

    return result;
}

#endif