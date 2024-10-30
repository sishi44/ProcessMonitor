#include <unistd.h>
#include <cctype>
#include <sstream>
#include <string>

#include "linux_parser.h"
#include "process.h"

using std::string;

Process::Process(int pid) : pid_(pid) {
  user_ = LinuxParser::User(pid);
  cpu_utilization_ = Process::CpuUtilization();
  ram_ = LinuxParser::Ram(pid);
  command_ = LinuxParser::Command(pid);
  uptime_ = LinuxParser::UpTime(pid);
}

// Return this process's ID
int Process::Pid() { return pid_; }

// Return this process's CPU utilization
float Process::CpuUtilization() {    
    long total = LinuxParser::ActiveJiffies(pid_);
    long seconds = LinuxParser::UpTime(pid_);

    if (seconds > 0) {
      cpu_utilization_ = (total * 1.0 / sysconf(_SC_CLK_TCK) ) / seconds;
    } else {
      cpu_utilization_ = 0.0f;
    }

    return cpu_utilization_;
}

// Return the command that generated this process
string Process::Command() { return command_; }

// Return this process's memory utilization
string Process::Ram() { return ram_; }

// Return the user (name) that generated this process
string Process::User() { return user_; }

// Return the age of this process (in seconds)
long int Process::UpTime() { return uptime_; }

// Overload the "less than" comparison operator for Process objects
bool Process::operator<(Process const& a) const {
    return cpu_utilization_ < a.cpu_utilization_;
}