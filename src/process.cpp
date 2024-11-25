#include <unistd.h>
#include <cctype>
#include <sstream>
#include <string>
#include <vector>
#include <thread>  // For std::this_thread::sleep_for
#include <chrono>  // For std::chrono::milliseconds

#include "process.h"
#include "linux_parser.h"


using std::to_string;
using std::vector;
// constructor
Process::Process(int pid):pid_{pid} {}

// Return this process's ID
int Process::Pid() { return pid_; }

float Process::CpuUtilization() {
   // Get the current values of active and total jiffies
    long active_jiffies = LinuxParser::ActiveJiffies(pid_);
    long total_jiffies = LinuxParser::Jiffies();

    // Calculate the difference between the current and previous values
    long delta_active_jiffies = active_jiffies - prev_active_jiffies_;
    long delta_total_jiffies = total_jiffies - prev_total_jiffies_;

    // Avoid division by zero
    // Define a maximun number of retries and a sleep duration
    const int max_retries{5};
    const std::chrono::milliseconds sleep_duration{100};
    //retry until delta_total_jiffies is not zero or max_retries is reached
    int retries{0};
    if (delta_total_jiffies == 0 && retries < max_retries) {
        // Wait for a short period and re-evaluate the jiffies
        std::this_thread::sleep_for(sleep_duration);
        active_jiffies = LinuxParser::ActiveJiffies();
        total_jiffies = LinuxParser::Jiffies();
        delta_active_jiffies = active_jiffies - prev_active_jiffies_;
        delta_total_jiffies = total_jiffies - prev_total_jiffies_;
        retries++;
    }

    // Update the previous values with the current values
    prev_active_jiffies_ = active_jiffies;
    prev_total_jiffies_ = total_jiffies;
    
    // Avoid division by zero
    if (delta_total_jiffies == 0) {
        return 0.0;
    }

    // Calculate the CPU utilization
    return static_cast<float>(delta_active_jiffies) / delta_total_jiffies;
}

/*
discard this method because it does not work properly, cpu usage is always 0 or make the reresh rate very slow

// Return this process's CPU utilization
float Process::CpuUtilization() const {
   // Get the current values of active and total jiffies
    long initial_active_jiffies_pid = LinuxParser::ActiveJiffies(pid_);
    long initial_total_jiffies_system = LinuxParser::Jiffies();

    // Wait for a short interval (e.g., 10 milliseconds)
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    // Measure final CPU time and system jiffies
    long final_active_jiffies_pid = LinuxParser::ActiveJiffies(pid_);
    long final_total_jiffies_system = LinuxParser::Jiffies();

    // Calculate the differences
    long delta_active_jiffies_pid = final_active_jiffies_pid - initial_active_jiffies_pid;
    long delta_total_jiffies_system = final_total_jiffies_system - initial_total_jiffies_system;

    // Calculate CPU usage
    if (delta_total_jiffies_system > 0) {
        float cpu_utilization_ = static_cast<float>(delta_active_jiffies_pid) / delta_total_jiffies_system;
        return cpu_utilization_;
    }
    return 0.0;
}
*/


// Return the command that generated this process
std::string Process::Command() { 
    if (cmdline_.empty()) {
        cmdline_ = LinuxParser::Command(pid_);
    }
    return cmdline_; 
}

// Return this process's memory utilization
std::string Process::Ram() const{ 
    return LinuxParser::Ram(pid_); 
}

// Return the user (name) that generated this process
std::string Process::User() {
    std::string user_ = LinuxParser::User(pid_);
    return user_;
}

// Return the age of this process (in seconds)
long int Process::UpTime() {
     return LinuxParser::UpTime(pid_); 
}

// Overload the "less than" comparison operator for Process objects (not used)
bool Process::operator<(Process const& a) const {
    return std::stol(this->Ram()) < std::stol(a.Ram());
}
// Overload the "greater than" comparison operator for Process objects to achieve descending order
bool Process::operator>(Process const& a) const {
    return std::stol(this->Ram()) > std::stol(a.Ram());
}