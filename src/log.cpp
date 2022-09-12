#include "log.hpp"
#include <ctime>

std::fstream Log::logfile;
int Log::loglevel;
bool Log::dofilelogging;
std::string Log::gamepath;

void Log::setlogmode(bool filelog, int level, const char* path) {
    gamepath = std::string(path);
    if(!dofilelogging && filelog) {
        //clean log file
        logfile.open((gamepath + "log.txt"), std::ofstream::out);
        if(logfile.fail()) {
            std::cout << "log file doesnt exist\n";
            filelog = false;
        }
        //logfile.write("", 1);
        logfile.close();
        
    }
    dofilelogging = filelog;
    loglevel = level;
    
}
void Log::log(const char* logmsg, int level) {
    if(level <= loglevel) {
        std::cout << logmsg << "\n";
        
        if(dofilelogging) {
            char buffer[256];
            time_t curtime = time(NULL);
            strftime(buffer, sizeof(buffer), "%H:%M:%S", localtime(&curtime));
            std::string logstring = "[" + std::string(buffer) + "] " + std::string(logmsg) + "\n";

            //open and close to save it so it isn't empty after crash
            logfile.open((gamepath + "log.txt"), std::ofstream::app);
            logfile << logstring;
            logfile.close();
        }
    }
}
void Log::log(std::string logmsg, int level) {
    if(level <= loglevel) {
        std::cout << logmsg << "\n";
        if(dofilelogging) {
            char buffer[256];
            time_t curtime = time(NULL);
            strftime(buffer, sizeof(buffer), "%H:%M:%S", localtime(&curtime));
            logmsg = "[" + std::string(buffer) + "] " + std::string(logmsg) + "\n";

            //open and close to save it so it isn't empty after crash
            logfile.open((gamepath + "log.txt"), std::ofstream::app);
            logfile << logmsg;
            logfile.close();
        }
    }
}