#ifndef _CLOCK_H_
#define _CLOCK_H_

const char ntpServer[] = "us.pool.ntp.org";
const int timeZone = 1; 
const unsigned int ntpLocalPort = 61980;

void clockSetup();

#endif
