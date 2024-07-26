#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <unistd.h>
#include <string>
#include <sstream>
#include <iostream>
#include "crtmodules/CRT.h"
#include <sys/types.h>
#include <signal.h>
//#include "TRACE/tracemf.h"
#include "logging/Logging.hpp"

//using namespace std;
using std::endl;
using std::string;
using std::istringstream;

int stopallboards(const char *online_path){ 
  int PMTINI,PMTFIN; 

  PMTINI = 1;
  PMTFIN = dunedaq::crtmodules::getnumpmt();

  dunedaq::crtmodules::stoptakedata(PMTINI,PMTFIN,online_path);               //Stops taking data

  sleep(2);

  char pidline[1024];
  char *pid;
  int sig = 9;
  unsigned pids_found= 0;
  int pidno[5];
  FILE *fp = popen("/sbin/pidof crt_readout","r");
  if (fp) {
    pidline[0]='x'; pidline[1]='\0';
    char *ret= fgets(pidline,sizeof(pidline),fp);
    if (ret) {
      TLOG_DEBUG(1)<<"fp="<<(void*)fp<<" pidline is "<<pidline;
      pid = strtok(pidline," ");
      TLOG_DEBUG(1) << "pid is " << pid;
      while (pid != NULL && pids_found < (sizeof(pidno)/sizeof(pidno[0]))) {
	pidno[pids_found] = atoi(pid);
	pid = strtok(NULL , " ");
	++pids_found;
      }
    } else TLOG_DEBUG(1) <<"ret="<<(void*)ret <<" NO (stdout) OUTPUT; pidline[0]="<<pidline[0];
    for (unsigned id=0; id<pids_found; ++id) {
      if ( pidno[id] > 0 ) {
	TLOG_DEBUG()<<"killing pid: "<<pidno[id];
	kill(pidno[id],sig);
      }
    }
    pclose(fp);
  } 
  TLOG_DEBUG() << "pids found: " << pids_found;

  return 0;
}
