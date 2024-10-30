#include <dirent.h>
#include <unistd.h>
#include <string>
#include <vector>

#include "linux_parser.h"

using std::stoi;
using std::stof;
using std::stol;
using std::string;
using std::to_string;
using std::vector;

// An example of how to read data from the filesystem
string LinuxParser::OperatingSystem() {
  string line;
  string key;
  string value;
  std::ifstream filestream(kOSPath);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::replace(line.begin(), line.end(), ' ', '_');
      std::replace(line.begin(), line.end(), '=', ' ');
      std::replace(line.begin(), line.end(), '"', ' ');
      std::istringstream linestream(line);
      while (linestream >> key >> value) {
        if (key == "PRETTY_NAME") {
          std::replace(value.begin(), value.end(), '_', ' ');
          return value;
        }
      }
    }
  }
  return value;
}

// An example of how to read data from the filesystem
string LinuxParser::Kernel() {
  string os, version, kernel;
  string line;
  std::ifstream stream(kProcDirectory + kVersionFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    linestream >> os >> version >> kernel;
  }
  return kernel;
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
      string filename(file->d_name);
      if (std::all_of(filename.begin(), filename.end(), isdigit)) {
        int pid = stoi(filename);
        pids.push_back(pid);
      }
    }
  }
  closedir(directory);
  return pids;
}

// Read and return the system memory utilization
float LinuxParser::MemoryUtilization() {
  string line, key, value;
  float mem_total, mem_free;
  float mem_utilization;

  std::ifstream stream(kProcDirectory + kMeminfoFilename);
  if (stream.is_open()) {
    while (std::getline(stream, line)) {
      std::replace(line.begin(), line.end(), ':', ' ');
      std::istringstream linestream(line);
      while (linestream >> key >> value) {
        if (key == "MemTotal") {
          try {
            mem_total = stof(value);
          } catch (...) {
            mem_total = 0.0f; 
          }
        } else if (key == "MemFree") {
          try {
            mem_free = stof(value); 
          } catch(...) {
            mem_free = 0.0f;
          }
          if (mem_total != 0.0f) {
            mem_utilization = (mem_total - mem_free) / mem_total;
            return mem_utilization;
          }
        }
      }
    }
  }

  return mem_utilization;
}

// Read and return the system uptime [s]
long LinuxParser::UpTime() {
  string line;  
  long up_time, idle_time;

  std::ifstream stream(kProcDirectory + kUptimeFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    linestream >> up_time >> idle_time;
  }

  return up_time;
}

// Read and return the number of jiffies for the system
long LinuxParser::Jiffies() { return LinuxParser::ActiveJiffies() + LinuxParser::IdleJiffies(); }

// Read and return the number of active jiffies for a PID
long LinuxParser::ActiveJiffies(int pid) {
  string line, value;
  vector<string> values;
  long active_jiffies;

  std::ifstream stream(kProcDirectory + to_string(pid) + kStatFilename);
  if (stream.is_open()) {
    while (std::getline(stream, line)) {
      std::istringstream linestream(line);
      while (linestream >> value) {
        values.push_back(value);
      }
    }

    try {
      active_jiffies = stol(values[13]) + stol(values[14]) + stol(values[15]) + stol(values[16]);
    } catch(...) {
      active_jiffies = 0;
    }
  }

  return active_jiffies;
}

// Read and return the number of active jiffies for the system
long LinuxParser::ActiveJiffies() {
  long active_jiffies;

  auto cpu_states = CpuUtilization();
  try {
    active_jiffies = stol(cpu_states[CPUStates::kUser_]) + stol(cpu_states[CPUStates::kNice_]) +
                     stol(cpu_states[CPUStates::kSystem_]) + stol(cpu_states[CPUStates::kIRQ_]) +
                     stol(cpu_states[CPUStates::kSoftIRQ_]) + stol(cpu_states[CPUStates::kSteal_]);
  } catch(...) {
    active_jiffies = 0;
  }

  return active_jiffies;
}

// Read and return the number of idle jiffies for the system
long LinuxParser::IdleJiffies() {
  long idle_jiffies;

  auto cpu_states = LinuxParser::CpuUtilization();
  try {
    idle_jiffies = stol(cpu_states[CPUStates::kIdle_]) + stol(cpu_states[CPUStates::kIOwait_]);
  } catch(...) {
    idle_jiffies = 0;
  }

  return idle_jiffies;
}

// Read and return CPU utilization
vector<string> LinuxParser::CpuUtilization() {
  string line, key, value;
  vector<string> cpu_states;

  std::ifstream stream(kProcDirectory + kStatFilename);
  if (stream.is_open()) {
    while (std::getline(stream, line)) {
      std::istringstream linestream(line);
      linestream >> key;
      if (key == "cpu") {
        while(linestream >> value) {
          cpu_states.push_back(value);
        }
        break;
      }
    }
  }

  return cpu_states;
}

// Read and return the total number of processes
int LinuxParser::TotalProcesses() {
  string line, key, value;
  int total_processes;
  std::ifstream stream(kProcDirectory + kStatFilename);
  if (stream.is_open()) {
    while (std::getline(stream, line)) {
      std::istringstream linestream(line);
      while (linestream >> key >> value) {
        if (key == "processes") {
          try {
            total_processes = stoi(value);
          } catch(...) {
            total_processes = 0;
          }
          break;
        }
      }
    }
  }

  return total_processes;
}

// Read and return the number of running processes
int LinuxParser::RunningProcesses() {
  string line, key, value;
  int running_processes;

  std::ifstream stream(kProcDirectory + kStatFilename);
  if (stream.is_open()) {
    while (std::getline(stream, line)) {
      std::istringstream linestream(line);
      while (linestream >> key >> value) {
        if (key == "procs_running") {
          try {
            running_processes = stoi(value);
          } catch(...) {
            running_processes = 0;
          }
          break;
        }
      }
    }
  }

  return running_processes;
}

// Read and return the command associated with a process
string LinuxParser::Command(int pid) {
  string command;

  std::ifstream stream(kProcDirectory + to_string(pid) + kCmdlineFilename);
  if (stream.is_open()) {
    std::getline(stream, command);
  }

  return command;
}

// Read and return the memory used by a process
string LinuxParser::Ram(int pid) {
  string line, key, value;
  string ram;

  std::ifstream stream(kProcDirectory + to_string(pid) + kStatusFilename);
  if (stream.is_open()) {
    while (std::getline(stream, line)) {
      std::replace(line.begin(), line.end(), ':', ' ');
      std::istringstream linestream(line);
      while (linestream >> key >> value) {
        if (key == "VmSize") {
          try {
            int temp_ram = stoi(value) / 1024;
            ram = to_string(temp_ram);
          } catch(...) {
            ram = " ";
          }
          break;
        }
      }
    }
  }

  return ram;
}

// Read and return the user ID associated with a process
string LinuxParser::Uid(int pid) {
  string line, key, value;

  std::ifstream stream(kProcDirectory + to_string(pid) + kStatusFilename);
  if (stream.is_open()) {
    while (std::getline(stream, line)) {
      std::istringstream linestream(line);
      while (linestream >> key >> value) {
        if (key == "Uid") {
          break;
        }
      }
    }
  }

  return value;
}

// Read and return the user associated with a process
string LinuxParser::User(int pid) {
  string line, user, x, value;
  string uid = LinuxParser::Uid(pid);

  std::ifstream stream(kPasswordPath);
  if (stream.is_open()) {
    while (std::getline(stream, line)) {
      std::replace(line.begin(), line.end(), ':', ' ');
      std::istringstream linestream(line);
      while (linestream >> user >> x >> value) {
        if (value == uid) {
          return user;
        }
      }
    } 
  }

  return user;
}

// Read and return the uptime of a process
long LinuxParser::UpTime(int pid) {
  string line, value;
  long start_time, up_time;

  std::ifstream stream(kProcDirectory + to_string(pid) + kStatFilename);
  if (stream.is_open()) {
    while (std::getline(stream, line)) {
      std::istringstream linestream(line);
      int i = 0;
      while (linestream >> value) {
        i++;
        if (i == 22) {
          try{
            start_time = stol(value);
          } catch(...) {
            start_time = 0;
          }
          up_time = LinuxParser::UpTime() - start_time / sysconf(_SC_CLK_TCK);
          return up_time;
        }
      }
    }
  }

  return up_time;
}
