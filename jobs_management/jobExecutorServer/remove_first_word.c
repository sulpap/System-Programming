#include <string.h>
#include "remove_first_word.h"

void remove_first_word(char *sentence) {
    int i;
    int len = strlen(sentence);

    // Find the position of the first space
    for (i = 0; i < len; i++) {
        if (sentence[i] == ' ') {
            break;
        }
    }

    // Move the rest of the string forward
    memmove(sentence, sentence + i + 1, len - i);

    // Add null terminator to the end of the modified string
    sentence[len - i] = '\0';
}
