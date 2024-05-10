#include "correct_syntax.h"

const char *correct_syntax() {
  return "jobCommander/bin/jobCommander command <command arguments>\nValid commands: 'issueJob', 'setConcurrency', 'stop', 'poll' and 'exit'\n";
}
