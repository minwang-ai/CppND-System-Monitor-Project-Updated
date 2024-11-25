#include <unistd.h>
#include <cctype>
#include <sstream>
#include <string>
#include <vector>
#include <thread>  // For std::this_thread::sleep_for
#include <chrono>  // For std::chrono::milliseconds

#include "process.h"
#include "linux_parser.h"

using std::string;
using std::to_string;
using std::vector;
// constructor
Process::Process(int pid):pid_{pid} {}

// Return this process's ID
int Process::Pid() { return pid_; }

// TODO: Return this process's CPU utilization
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

// TODO: Return the command that generated this process
string Process::Command() { return string(); }

// TODO: Return this process's memory utilization
string Process::Ram() { return string(); }

// Return the user (name) that generated this process
string Process::User() {
    std::string user_ = LinuxParser::User(pid_);
    return user_;
}

// Return the age of this process (in seconds)
long int Process::UpTime() {
     return LinuxParser::UpTime(pid_); 
}

// TODO: Overload the "less than" comparison operator for Process objects
// REMOVE: [[maybe_unused]] once you define the function
bool Process::operator<(Process const& a[[maybe_unused]]) const { return true; }