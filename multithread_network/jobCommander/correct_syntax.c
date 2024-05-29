#include "correct_syntax.h"

// gives the user instructions on running the program
const char *correct_syntax() {
  return "jobCommander/bin/jobCommander [serverName] [portNum] [jobCommanderInputCommand]\nValid commands: 'issueJob', 'setConcurrency', 'stop', 'poll' and 'exit'\n";
}
