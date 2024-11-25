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
  std::string User();                      
  std::string Command();                   
  float CpuUtilization();                  
  std::string Ram() const;                       
  long int UpTime();                       
  bool operator<(Process const& a) const;  
  bool operator>(Process const& a) const; 


 private:
    int pid_;
    // Caching is appropriate because these values do not change during the runtime.
    std::string user_;
    std::string cmdline_;
    // caching cpu utilization because it is used in the comparison function multiple times
    // float cpu_utilization_;
    // Store the previous values for active and total jiffies for calculating the difference
    long prev_active_jiffies_{0};
    long prev_total_jiffies_{0};
};

#endif