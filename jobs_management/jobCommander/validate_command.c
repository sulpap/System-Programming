#include <string.h>
#include "validate_command.h"

// checks compatibility
int validate_command(const char *command) {
  return (
    strcmp("issu", command) == 0 ||
    strcmp("issueJob", command) == 0 ||
    strcmp("setConcurrency", command) == 0 ||
    strcmp("stop", command) == 0 ||
    strcmp("poll", command) == 0 ||
    strcmp("exit", command) == 0
  );
}
