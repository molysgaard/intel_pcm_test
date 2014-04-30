#include <iostream>
#include <vector>
#include "IntelPCM/cpucounters.h"

using namespace std;

int n = 1000;
float *a,*b,*c;

void kernel(float *a, float *b, float *c, int n){
  for(int i=0; i<n; i++){
    for(int j=0; j<n; j++){
      for(int k=0; k<n; k++){
        c[i*n+j] += a[i*n+k]*b[k*n+j];
      }
    }
  }
}

void initialize(){
  a = new float[n*n];
  b = new float[n*n];
  c = new float[n*n];
  for(int i=0; i<n; i++){
    a[i] = 1.0;
    b[i] = 2.0;
    c[i] = 0.0;
  }
}

void setup_generic(PCM *m){
  PCM::ErrorCode status;

  // program counters, and on a failure just exit
  status = m->program();

  if (status != PCM::Success){
    cout << "Could not program PCM" << endl;
    exit(1);
  }

}

void setup_custom(PCM *m, PCM::CustomCoreEventDescription *events){
  PCM::ErrorCode status;

  // this array of structs is the programmable counters
  // available. The hardware limit is 4.
  // program counters, and on a failure just exit
  status = m->program(PCM::CUSTOM_CORE_EVENTS, events);

  if (status != PCM::Success){
    cout << "Could not program PCM" << endl;
    exit(1);
  }
}


void bench(PCM *m, SystemCounterState *before, SystemCounterState *after){
  initialize();

  *before = getSystemCounterState();
  kernel(a, b, c, n);
  *after = getSystemCounterState();
  m->cleanup();
}

int main(int argc,char *argv[]){
  PCM *m = PCM::getInstance();
  if(argc>=2 && !strcmp(argv[1],"reset")){
    cout << "resetting" << endl;
    m->resetPMU();
    exit(0);
  }

  // num_events must be divisible by 4
  int num_events = 8;
  PCM::CustomCoreEventDescription custom_events[num_events];
  vector<string> event_names;
  // The custom_events array contains num_events PCM::CustomCoreEventDescriptions.
  // The first elem in the struct is the Event Num and second is the second the Umask Value.
  custom_events[0] = {0x0e,0x01};
  event_names.push_back("UOPS_ISSUED.ANY");

  custom_events[1] = {0x0e,0x10};
  event_names.push_back("UOPS_ISSUED.FLAGS_MERGE");

  custom_events[2] = {0x0e,0x20};
  event_names.push_back("UOPS_ISSUED.SLOW_LEA");

  custom_events[3] = {0x0e,0x40};
  event_names.push_back("UOPS_ISSUED.SiNGLE_MUL");

  custom_events[4] = {0x0e,0x01};
  event_names.push_back("UOPS_ISSUED.ANY");

  custom_events[5] = {0x0e,0x10};
  event_names.push_back("UOPS_ISSUED.FLAGS_MERGE");

  custom_events[6] = {0x0e,0x20};
  event_names.push_back("UOPS_ISSUED.SLOW_LEA");

  custom_events[7] = {0x0e,0x40};
  event_names.push_back("UOPS_ISSUED.SiNGLE_MUL");

  uint64 counts[num_events];

  SystemCounterState before1, after1, before2, after2;
  setup_generic(m);
  bench(m, &before1, &after1);

  for(int i=0; i<num_events; i+=4){
    m = PCM::getInstance();
    setup_custom(m, &custom_events[i]);
    bench(m, &before2, &after2);

    counts[i+0] = getNumberOfCustomEvents(0,before2, after2); // read number of occurred events from counter 0
    counts[i+1] = getNumberOfCustomEvents(1,before2, after2); // read number of occurred events from counter 1
    counts[i+2] = getNumberOfCustomEvents(2,before2, after2); // read number of occurred events from counter 2
    counts[i+3] = getNumberOfCustomEvents(3,before2, after2); // read number of occurred events from counter 3
  }

  cout << endl
       << "Cycles:" << getCycles(before1, after1) << endl
       << "Instructions per clock:" << getIPC(before1,after1) << endl
       << "L3 cache hit ratio:" << getL3CacheHitRatio(before1,after1) << endl
       << "Bytes read:" << getBytesReadFromMC(before1,after1) << endl;
  for(int i=0; i<num_events; i++){
    cout << event_names[i] << i << ": " << counts[i] << endl;
  }
}
