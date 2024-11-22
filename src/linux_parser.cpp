#include <dirent.h>
#include <unistd.h>
#include <string>
#include <vector>
#include <unordered_map>


#include "linux_parser.h"

using std::stof;
using std::to_string;
using std::vector;


/**
 * @brief Retrieves the operating system name from the OS release file.
 *
 * This function reads the operating system information from the file specified
 * by the kOSPath constant. It looks for the "PRETTY_NAME" key in the file,
 * which typically contains the human-readable name of the operating system.
 *
 * @return std::string: A string containing the operating system name. If the file cannot be
 *         opened or the "PRETTY_NAME" key is not found, an empty string is returned.
 */
std::string LinuxParser::OperatingSystem() {
  std::ifstream filestream(kOSPath);
  if (filestream) {
    std::string line;
    while (std::getline(filestream, line)) {
      std::replace(line.begin(), line.end(), ' ', '_');
      std::replace(line.begin(), line.end(), '=', ' ');
      std::replace(line.begin(), line.end(), '"', ' ');
      std::istringstream linestream(line);
      std::string key;
      std::string value;
      while (linestream >> key >> value) {
        if (key == "PRETTY_NAME") {
          std::replace(value.begin(), value.end(), '_', ' ');
          return value;
        }
      }
    }
  } 
  return "";
}


/**
 * @brief Retrieves the kernel version of the operating system.
 *
 * This function reads the kernel version from the file located at
 * /proc/version and returns it as a string. It extracts the kernel
 * version by parsing the first line of the file.
 *
 * @return std::string: A string representing the kernel version.
 */
std::string LinuxParser::Kernel() {
  std::ifstream stream(kProcDirectory + kVersionFilename);
  if (stream.is_open()) {
    std::string line;
    std::getline(stream, line);
    std::istringstream linestream(line);
    std::string os, version, kernel;
    linestream >> os >> version >> kernel;
    return kernel;
  }
  return "";
}

// BONUS: Update this to use std::filesystem
vector<int> LinuxParser::Pids() {
  vector<int> pids;
  DIR* directory = opendir(kProcDirectory.c_str());
  struct dirent* file;
  while ((file = readdir(directory)) != nullptr) {
    // Is this a directory?
    if (file->d_type == DT_DIR) {
      // Is every character of the name a digit?
      std::string filename(file->d_name);
      if (std::all_of(filename.begin(), filename.end(), isdigit)) {
        int pid = stoi(filename);
        pids.push_back(pid);
      }
    }
  }
  closedir(directory);
  return pids;
}


/**
 * @brief Calculate the memory utilization of the system.
 * 
 * This function reads memory information from the /proc/meminfo file and calculates the memory utilization.
 * The memory utilization is calculated as the total used memory divided by the total memory.
 * The calculation formula follows the free command description of Ubuntu 20.04 LTS (https://manpages.ubuntu.com/manpages/focal/man1/free.1.html)
 * because the codespace of the project is Ubuntu 20.04 LTS.
 * 
 * For Ubuntu 23.10 and later, the formula is simplified as total - available.
 * 
 * @return float The memory utilization as a fraction (0.0 to 1.0).
 */
float LinuxParser::MemoryUtilization() {
  std::ifstream file_stream(kProcDirectory + kMeminfoFilename);
  if(file_stream){
    std::string line;
    std::unordered_map<std::string, long> meminfo;
    std::string key; 
    long value;
    while(getline(file_stream, line)){
      std::istringstream linestream(line);
      linestream >> key >> value;
        key.pop_back(); // remove the colon at the end of the key
        meminfo[key] = value;
      }
    
      long total = meminfo["MemTotal"] + meminfo["SwapTotal"];
      long free = meminfo["MemFree"] + meminfo["SwapFree"];
      long buffers = meminfo["Buffers"];
      long cache = meminfo["Cached"] + meminfo["Slab"];
      long total_used_memory = total - free - buffers - cache;
      return static_cast<float>(total_used_memory) / total;
  }
  return 0.0;
}


/**
 * @brief Retrieves the system uptime.
 *
 * This function reads the system uptime from the /proc/uptime file and returns it.
 * The uptime is the amount of time (in seconds) that the system has been running since it was started.
 *
 * @return long: The system uptime in seconds. If the file cannot be read, returns 0.
 */
long LinuxParser::UpTime() { 
  std::ifstream file_stream(kProcDirectory + kUptimeFilename);
  if (file_stream){
    std::string line;
    long uptime;
    std::getline(file_stream, line);
    std::istringstream linestream(line);
    linestream >> uptime;
    return uptime;
  } 
  return 0;
}


/**
 * @brief Calculates the total number of jiffies (time units) for the system.
 *
 * This function retrieves CPU utilization statistics and calculates the total
 * number of jiffies by summing up the active and idle jiffies.
 *
 * @return long: The total number of jiffies (active + idle) for the system.
 */
long LinuxParser::Jiffies() { 
   vector<std::string> cpu_stats = CpuUtilization();

  long user = std::stol(cpu_stats[LinuxParser::CPUStates::kUser_]);
  long nice = std::stol(cpu_stats[LinuxParser::CPUStates::kNice_]);
  long system = std::stol(cpu_stats[LinuxParser::CPUStates::kSystem_]);
  long idle = std::stol(cpu_stats[LinuxParser::CPUStates::kIdle_]);
  long iowait = std::stol(cpu_stats[LinuxParser::CPUStates::kIOwait_]);
  long irq = std::stol(cpu_stats[LinuxParser::CPUStates::kIRQ_]);
  long softirq = std::stol(cpu_stats[LinuxParser::CPUStates::kSoftIRQ_]);
  long steal = std::stol(cpu_stats[LinuxParser::CPUStates::kSteal_]);
  // Guest time is already accounted in usertime, so we don't need to add it
  long active_jiffies = user + nice + system + irq + softirq + steal;
  long idle_jiffies = idle + iowait;

  return active_jiffies + idle_jiffies;
}

// TODO: Read and return the number of active jiffies for a PID
// REMOVE: [[maybe_unused]] once you define the function
long LinuxParser::ActiveJiffies(int pid[[maybe_unused]]) { return 0; }


/**
 * @brief Calculates the total number of active jiffies for the system.
 *
 * This function retrieves CPU utilization statistics and sums up the jiffies
 * spent in user mode, nice mode, system mode, IRQ, soft IRQ, and steal time.
 * Note that guest time is already included in user time and is not added separately.
 *
 * @return long: The total number of active jiffies.
 */
long LinuxParser::ActiveJiffies() {
  vector<std::string> cpu_stats = LinuxParser::CpuUtilization();

  long user = std::stol(cpu_stats[LinuxParser::CPUStates::kUser_]);
  long nice = std::stol(cpu_stats[LinuxParser::CPUStates::kNice_]);
  long system = std::stol(cpu_stats[LinuxParser::CPUStates::kSystem_]);
  long irq = std::stol(cpu_stats[LinuxParser::CPUStates::kIRQ_]);
  long softirq = std::stol(cpu_stats[LinuxParser::CPUStates::kSoftIRQ_]);
  long steal = std::stol(cpu_stats[LinuxParser::CPUStates::kSteal_]);
  // Guest time is already accounted in usertime, so we don't need to add it
  return user + nice + system + irq + softirq + steal;
}


/**
 * @brief Calculates the total number of idle jiffies for the system.
 *
 * This function retrieves the CPU utilization statistics and extracts the
 * idle and iowait jiffies, then returns their sum. Idle jiffies represent
 * the time the CPU has spent doing nothing, while iowait jiffies represent
 * the time the CPU has been waiting for I/O operations to complete.
 *
 * @return long: The total number of idle jiffies (idle + iowait).
 */
long LinuxParser::IdleJiffies() {
  vector<std::string> cpu_stats = LinuxParser::CpuUtilization();
  long idle = std::stol(cpu_stats[LinuxParser::CPUStates::kIdle_]);
  long iowait = std::stol(cpu_stats[LinuxParser::CPUStates::kIOwait_]);
  return idle + iowait;
}


/**
 * @brief Retrieves the CPU utilization statistics from the /proc/stat file.
 *
 * This function reads the first line of the /proc/stat file, which contains
 * the aggregate CPU statistics. It extracts the values following the "cpu" prefix
 * and returns them as a vector of strings. Each value represents a different
 * aspect of CPU time (e.g., user time, system time, idle time, etc.).
 *
 * @return std::vector<std::string>: A vector containing the CPU utilization statistics.
 */
vector<std::string> LinuxParser::CpuUtilization() {
  std::vector<std::string> cpu_stats;
  std::ifstream file_stream(kProcDirectory + kStatFilename);
  if(file_stream){
    std::string line;
    std::getline(file_stream, line);
    std::istringstream linestream(line);
    std::string cpu_prefix;
    linestream >> cpu_prefix; // read the first token "cpu" and discard it later
    std::string value;
    while(linestream >> value){
      cpu_stats.push_back(value);
    }
  return cpu_stats;
  }
}

/**
 * @brief Reads and returns the total number of processes from the /proc/stat file.
 *
 * This function opens the /proc/stat file, reads its contents line by line,
 * and searches for the line that starts with the keyword "processes".
 * When it finds this line, it extracts and returns the integer value that follows the keyword.
 * If the file cannot be opened or the keyword is not found, the function returns 0.
 *
 * @return int: The total number of processes, or 0 if the information cannot be retrieved.
 */
int LinuxParser::TotalProcesses() {
  std::ifstream file_stream(kProcDirectory + kStatFilename);
  if(file_stream){
    std::string line;
    while(getline(file_stream, line)){
      std::istringstream linestream(line);
      std::string key;
      int value;
      linestream >> key >> value;
      if (key == "processes"){
        return value;
      }
    }
  }
  return 0;
}

/**
 * @brief Retrieves the number of running processes from the /proc/stat file.
 *
 * This function reads the /proc/stat file to find the line that starts with "procs_running"
 * and extracts the corresponding value, which represents the number of currently running processes.
 *
 * @return int: The number of running processes. Returns 0 if the file cannot be read or the key is not found.
 */
int LinuxParser::RunningProcesses() {
  std::ifstream file_stream(kProcDirectory + kStatFilename);
  if (file_stream){
    std::string line;
    while(getline(file_stream,line)){
      std::istringstream linestream(line);
      std::string key;
      int value;
      linestream >> key >> value;
      if (key == "procs_running"){
        return value;
      }
    }
  }
  return 0;
}

// TODO: Read and return the command associated with a process
// REMOVE: [[maybe_unused]] once you define the function
std::string LinuxParser::Command(int pid[[maybe_unused]]) { return std::string(); }

// TODO: Read and return the memory used by a process
// REMOVE: [[maybe_unused]] once you define the function
std::string LinuxParser::Ram(int pid[[maybe_unused]]) { return std::string(); }

// TODO: Read and return the user ID associated with a process
// REMOVE: [[maybe_unused]] once you define the function
std::string LinuxParser::Uid(int pid[[maybe_unused]]) { return std::string(); }

// TODO: Read and return the user associated with a process
// REMOVE: [[maybe_unused]] once you define the function
std::string LinuxParser::User(int pid[[maybe_unused]]) { return std::string(); }

// TODO: Read and return the uptime of a process
// REMOVE: [[maybe_unused]] once you define the function
long LinuxParser::UpTime(int pid[[maybe_unused]]) { return 0; }
