#ifndef PROCESSOR_H
#define PROCESSOR_H

class Processor {
 public:
  float Utilization();  // See src/processor.cpp

  // Declare any necessary private members
 private:
   void Update(long total, long idle);
   long pre_total;
   long pre_idle;
};

#endif