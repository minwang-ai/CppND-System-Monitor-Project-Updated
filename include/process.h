#ifndef PROCESS_H
#define PROCESS_H

#include <string>
/*
Basic class for Process representation
It contains relevant attributes as shown below
*/
class Process {
 public:
  // Constructor
  Process(int pid);  

  int Pid();                               
  std::string User();                      // TODO: See src/process.cpp
  std::string Command();                   // TODO: See src/process.cpp
  float CpuUtilization();                  // TODO: See src/process.cpp
  std::string Ram();                       // TODO: See src/process.cpp
  long int UpTime();                       // TODO: See src/process.cpp
  bool operator<(Process const& a) const;  // TODO: See src/process.cpp


 private:
    int pid_;
    std::string user_;
    // Store the previous values for active and total jiffies for calculating the difference
    long prev_active_jiffies_{0};
    long prev_total_jiffies_{0};
};

#endif