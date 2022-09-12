#pragma once
#include <string>
#include <fstream>
#include <iostream>


class Log {
public:
    static void setlogmode(bool filelog, int level, const char* path);
    static void log(const char* logmsg, int level);
    static void log(std::string logmsg, int level);

    static std::fstream logfile;
    static int loglevel;
    static bool dofilelogging;
    static std::string gamepath;

private:

};