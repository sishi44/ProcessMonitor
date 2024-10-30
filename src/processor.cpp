#include "linux_parser.h"
#include "processor.h"

// Return the aggregate CPU utilization
float Processor::Utilization() {
    float utilization;

    long current_total = LinuxParser::Jiffies();
    long current_idle = LinuxParser::IdleJiffies();

    float delta_total = current_total - pre_total;
    float delta_idle = current_idle - pre_idle;

    Processor::Update(current_total, current_idle);

    utilization = (delta_total - delta_idle) / delta_total;

    return utilization;
}

// Update private parameters of class Processor
void  Processor::Update(long total, long idle) {
    pre_total = total;
    pre_idle = idle;
}
