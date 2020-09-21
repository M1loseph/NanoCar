#ifndef __COMMANDS_H__
#define __COMMANDS_H__

namespace commands
{
  namespace driving
  {
    const char *FORWARD_L = "FORWARD_L";
    const char *FORWARD_R = "FORWARD_R";
    const char *FORWARD = "FORWARD";
    const char *BACKWARD_L = "BACKWARD_L";
    const char *BACKWARD_R = "BACKWARD_R";
    const char *BACKWARD = "BACKWARD";
    const char *STOP_L = "STOP_L";
    const char *STOP_R = "STOP_R";
    const char *STOP = "STOP";
    const char *FASTER_L = "FASTER_L";
    const char *FASTER_R = "FASTER_R";
    const char *FASTER = "FASTER";
    const char *SLOWER_L = "SLOWER_L";
    const char *SLOWER_R = "SLOWER_R";
    const char *SLOWER = "SLOWER";
    const char *KEEP_SPEED_L = "KEEP_SPEED_L";
    const char *KEEP_SPEED_R = "KEEP_SPEED_R";
    const char *KEEP_SPEED = "KEEP_SPEED";
    const char *SET_SPEED_L = "SET_SPEED_L";
    const char *SET_SPEED_R = "SET_SPEED_R";
    const char *SET_SPEED = "SET_SPEED";
  } // namespace driving

  // const char *ARM = "ARM";
  // const char *MP3 = "MP3";
  // const char *INTERVAL = "INTERVAL";
  // this one is only for scripts, sending it to tank is useless
  // const char *DELAY = "DELAY";
  // const char *EXECUTE = "EXECUTE";
  // const char *LOG = "LOG";
  // all sensors
  // const char *FULL_JSON = "FULL_JSON";

  // const char *ARM_STATE = "ARM_STATE";
  // const char *DISTANCES = "DISTANCES";
} // namespace commands

#endif //__COMMANDS_H__