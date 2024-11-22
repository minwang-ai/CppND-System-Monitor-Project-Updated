#include <thread>  // For std::this_thread::sleep_for
#include <chrono>  // For std::chrono::milliseconds

#include "processor.h"
#include "linux_parser.h"

/**
 * @brief Calculates the CPU utilization.
 *
 * This function computes the CPU utilization by comparing the current and previous
 * values of active and total jiffies. It retrieves the current values of active and
 * total jiffies, calculates the differences (deltas) between the current and previous
 * values, and then computes the CPU utilization as the ratio of the delta of active
 * jiffies to the delta of total jiffies. If the delta of total jiffies is zero, the
 * function waits for a short period and re-evaluates the jiffies to avoid division by zero.
 *
 * @return float: The CPU utilization as a float value between 0.0 and 1.0.
 */
float Processor::Utilization() {
    // Get the current values of active and total jiffies
    long active_jiffies = LinuxParser::ActiveJiffies();
    long total_jiffies = LinuxParser::Jiffies();

    // Calculate the difference between the current and previous values
    long delta_active_jiffies = active_jiffies - prev_active_jiffies_;
    long delta_total_jiffies = total_jiffies - prev_total_jiffies_;

    // If delta_total_jiffies is zero, wait for a short period and re-evaluate
    if (delta_total_jiffies == 0) {
    std::this_thread::sleep_for(std::chrono::milliseconds(100));  // Wait for 100 milliseconds
    active_jiffies = LinuxParser::ActiveJiffies();
    total_jiffies = LinuxParser::Jiffies();
    delta_active_jiffies = active_jiffies - prev_active_jiffies_;
    delta_total_jiffies = total_jiffies - prev_total_jiffies_;
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