#ifndef PROCESSOR_H
#define PROCESSOR_H

class Processor {
 public:
  float Utilization();  

 private:
    // Store the previous values for active and total jiffies for calculating the difference
    long prev_active_jiffies_{0};
    long prev_total_jiffies_{0};
};

#endif