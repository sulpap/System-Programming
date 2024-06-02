#ifndef UTILITY_H
#define UTILITY_H

void perror_exit(char *message);
void sanitize(char *str);
const char *correct_syntax();
void remove_first_word(char *sentence);

#endif