#include <dirent.h>
#include <unistd.h>
#include <string>
#include <vector>
#include <unordered_map>

#include "linux_parser.h"

using std::stof;
using std::string;
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
 * @brief Calculates the memory utilization of the system.
 *
 * This function reads the memory information from the /proc/meminfo file,
 * extracts the total memory and available memory, and calculates the memory
 * utilization as the ratio of used memory to total memory.
 *
 * @return float: The memory utilization as a float value between 0 and 1.
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
      if (key == "MemTotal:" || key == "MemAvailable:") {
        key.pop_back(); // remove the colon at the end of the key
        meminfo[key] = value;
      }
    }
    long total_used_memory = meminfo["MemTotal"] - meminfo["MemAvailable"];
    return static_cast<float>(total_used_memory) / meminfo["MemTotal"];
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

// TODO: Read and return the number of jiffies for the system
long LinuxParser::Jiffies() { return 0; }

// TODO: Read and return the number of active jiffies for a PID
// REMOVE: [[maybe_unused]] once you define the function
long LinuxParser::ActiveJiffies(int pid[[maybe_unused]]) { return 0; }

// TODO: Read and return the number of active jiffies for the system
long LinuxParser::ActiveJiffies() { return 0; }

// TODO: Read and return the number of idle jiffies for the system
long LinuxParser::IdleJiffies() { return 0; }

// TODO: Read and return CPU utilization
vector<std::string> LinuxParser::CpuUtilization() { return {}; }

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
