#include <unistd.h>
#include <cstddef>
#include <set>
#include <string>
#include <vector>
#include <iostream>

#include "process.h"
#include "processor.h"
#include "system.h"
#include "linux_parser.h"

using std::set;
using std::size_t;
using std::string;
using std::vector;

// Return the system's CPU
Processor& System::Cpu() { return cpu_; }

// Return a vector container composed of the system's processses
vector<Process>& System::Processes() { return processes_; }

// Return the system's kernel identifier (string)
std::string System::Kernel() { 
    if (!kernel_cached_) {
        kernel_ = LinuxParser::Kernel();
        kernel_cached_ = true;
    }
    return kernel_; 
}

// TODO: Return the system's memory utilization
float System::MemoryUtilization() { return 0.0; }

// TODO: Return the operating system name
std::string System::OperatingSystem() { 
    if (!os_cached_) {
        os_ = LinuxParser::OperatingSystem();
        os_cached_ = true;
    }
    return os_; 
}

// TODO: Return the number of processes actively running on the system
int System::RunningProcesses() {
    return LinuxParser::RunningProcesses();
}

// TODO: Return the total number of processes on the system
int System::TotalProcesses() { return 0; }

// TODO: Return the number of seconds since the system started running
long int System::UpTime() { 
    return LinuxParser::UpTime(); 
    //just for debugging, delete later
    // uptime_ = LinuxParser::UpTime(); 
    //std::cout << "System uptime: " << uptime_ << std::endl;
    //return uptime_;
}