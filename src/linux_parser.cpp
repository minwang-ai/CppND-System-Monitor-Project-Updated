#include <dirent.h>
#include <unistd.h>
#include <string>
#include <vector>
#include <unordered_map>

#include <unistd.h> // for using sysconf

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
/**
 * @brief Retrieves a list of process IDs (PIDs) from the /proc directory.
 *
 * This function opens the /proc directory and iterates through its entries.
 * It checks each entry to determine if it is a directory and if the directory
 * name consists entirely of digits. If both conditions are met, the directory
 * name is converted to an integer and added to the list of PIDs.
 *
 * @return A vector of integers representing the process IDs.
 */
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


/**
 * @brief Calculates the active jiffies for a given process.
 *
 * This function reads the /proc/[pid]/stat file to extract the values of utime, stime, cutime, and cstime,
 * which represent the amount of time the process has spent in user mode, kernel mode, and the respective
 * times for its waited-for children. It sums these values to return the total active jiffies for the process.
 *
 * @param pid int: The process ID for which to calculate active jiffies.
 * @return long: The total active jiffies for the process, or 0 if the file cannot be read.
 */
long LinuxParser::ActiveJiffies(int pid) { 
  std::ifstream file_stream(kProcDirectory + std::to_string(pid) + kStatFilename);
  if(file_stream){
    std::string line;
    std::getline(file_stream, line);
    std::istringstream linestream(line);
    std::string value;
    long utime{0}, stime{0}, cutime{0}, cstime{0};
    for (int i = 1; i <= 22; i++){
      linestream >> value;
      if (i == 14) utime = std::stol(value);
      if (i == 15) stime = std::stol(value);
      if (i == 16) cutime = std::stol(value);
      if (i == 17) cstime = std::stol(value);
    }
    return utime + stime + cutime + cstime;
  }
  return 0; 
}


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
  }
  return cpu_stats;
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


/**
 * @brief Retrieves the command that was used to start a process.
 *
 * This function reads the command line information for a given process ID (pid)
 * from the /proc filesystem and returns it as a string. If the file cannot be
 * opened or read, an empty string is returned.
 *
 * @param pid int : The process ID for which to retrieve the command line information.
 * @return std::string: A tring containing the command line used to start the process, or
 *         an empty string if the information cannot be retrieved.
 */
std::string LinuxParser::Command(int pid) {
  std::ifstream file_stream(kProcDirectory + std::to_string(pid) + kCmdlineFilename);
  if (file_stream) {
    std::string line;
    std::getline(file_stream, line, '\0');  // Read until the first null character
    std::string command = line;
    while (std::getline(file_stream, line, '\0')) {
      command += " " + line; // replace the null character with a space
    }
    // Replace newline characters with spaces
    std::replace(command.begin(), command.end(), '\n', ' ');
    return command;
  }
  return ""; 
}

// TODO: Read and return the memory used by a process
// REMOVE: [[maybe_unused]] once you define the function
std::string LinuxParser::Ram(int pid) {
  std::ifstream file_stream(kProcDirectory + std::to_string(pid) + kStatusFilename);
  if(file_stream){
    std::string line;
    std::string key;
    long value;
    while(std::getline(file_stream, line)){
      std::istringstream linestream(line);
      linestream >> key >> value;
      if (key == "VmSize:"){
        long ram = value / 1024; // convert from KB to MB
        return std::to_string(ram);
      }
    }
  }
   return "0"; 
}


/**
 * @brief Retrieves the UID of a process given its PID.
 *
 * This function reads the /proc/[pid]/status file to find the UID of the process.
 * It opens the file, reads it line by line, and searches for the line that starts
 * with "Uid:". Once found, it extracts and returns the UID value (the first integer).
 *
 * @param pid int: The process ID for which the UID is to be retrieved.
 * @return std::string: The UID of the process as a string. If the UID cannot be found, an empty string is returned.
 */
std::string LinuxParser::Uid(int pid) { 
  std::ifstream file_stream(kProcDirectory +std::to_string(pid) + kStatusFilename);
  if(file_stream){
    std::string line;
    while(std::getline(file_stream, line)){
      std::istringstream linestream(line);
      std::string key;
      std::string value;
      linestream >> key >> value;
      if(key == "Uid:"){
        return value;
      }
    }
  }
  return ""; 
}


/**
 * @brief Retrieves the username associated with a given process ID (PID).
 *
 * This function reads the UID of the process from the /proc filesystem and then
 * looks up the corresponding username from the /etc/passwd file.
 *
 * @param pid int: The process ID for which to retrieve the username.
 * @return std::string: The username associated with the given PID, or an empty string if the
 *         username could not be found.
 */
std::string LinuxParser::User(int pid) { 
  std::string uid = LinuxParser::Uid(pid);
  std::ifstream file_stream(kPasswordPath);
  if(file_stream){
    std::string line;
    while(std::getline(file_stream, line)){
      std::istringstream linestream(line);
      std::string user_name, x, file_uid;
       // Read the username
      std::getline(linestream, user_name, ':');
      // Read the 'x' (password placeholder)
      std::getline(linestream, x, ':');
      // Read the UID
      std::getline(linestream, file_uid, ':');
      // Ignore the rest of the line
      if(file_uid == uid){
        return user_name;
      }
    }
  }  
  return ""; 
}



/**
 * @brief Get the uptime of a process in seconds.
 *
 * This function reads the /proc/[pid]/stat file to extract the process start time
 * and calculates the uptime of the process by subtracting the start time from the
 * system uptime.
 *
 * @param pid int: The process ID for which to get the uptime.
 * @return long: The uptime of the process in seconds. If the file cannot be opened, returns 0.
 */
long LinuxParser::UpTime(int pid) { 
  std::ifstream file_stream(kProcDirectory + std::to_string(pid) + kStatFilename);
  if (file_stream) {
    std::string line;
    std::getline(file_stream, line);
    std::istringstream linestream(line);
    std::string value;
    long starttime;
    for (int i = 1; i <= 22; ++i) {
      linestream >> value;
      if (i == 22) starttime = std::stol(value);
    }
    // no matter the start time is expressed either in jiffies (before linux 2.6) or clock ticks (after linux 2.6), we simply convert it to seconds
    starttime /= sysconf(_SC_CLK_TCK); // convert clock ticks to seconds by dividing by the frequency (Hertz).
    return LinuxParser::UpTime() - starttime; // subtract the process start time from the system uptime to get the process uptime
  }
  return 0; 
}
