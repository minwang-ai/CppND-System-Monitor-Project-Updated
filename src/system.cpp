#include <unistd.h>
#include <cstddef>
#include <set>
#include <string>
#include <vector>
#include <iostream>
#include <algorithm>  // For std::sort

#include "process.h"
#include "processor.h"
#include "system.h"
#include "linux_parser.h"

using std::set;
using std::size_t;


// Return the system's CPU
Processor& System::Cpu() { return cpu_; }

// Return a vector container composed of the system's processses
std::vector<Process>& System::Processes() {
    std::vector<int> pids = LinuxParser::Pids();
    processes_.clear(); // Clear the existing processes before updating to ensure it is up-to-date
    for (int pid : pids){
        processes_.emplace_back(pid); // Construct a Process object in place for each PID
    } 
    // Sort the processes in descending order of CPU utilization)
    // By default, std::sort uses the less-than operator (<) to compare elements. 
    // based on the overloaded < operator in the Process class
    std::sort(processes_.begin(), processes_.end(), std::greater<Process>());
    return processes_; 
}

// Return the system's kernel identifier (string)
std::string System::Kernel() { 
    if (kernel_.empty()) {
        kernel_ = LinuxParser::Kernel();
    }
    return kernel_; 
}

// Return the system's memory utilization
float System::MemoryUtilization() {
    return LinuxParser::MemoryUtilization();
}

// Return the system's operating system name (string)
std::string System::OperatingSystem() { 
    if (os_.empty()) {
        os_ = LinuxParser::OperatingSystem();
    }
    return os_; 
}

// Return the number of processes actively running on the system
int System::RunningProcesses() {
    return LinuxParser::RunningProcesses();
}

// Return the total number of processes on the system
int System::TotalProcesses() {
    return LinuxParser::TotalProcesses();
}

// Return the number of seconds since the system started running
long int System::UpTime() { 
    return LinuxParser::UpTime(); 
}