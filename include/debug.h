#ifndef H_DEBUG
#define H_DEBUG

void dumpEphemeride(eph_t* ephemerides, int index);
void printNavmessage(unsigned long frame[SUBFRAME_COUNT][WORD_COUNT]);
void dumpChannels(gtime_t simulationTime, Channel* channels, int channelCount);

#endif