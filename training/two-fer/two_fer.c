#include "two_fer.h"
#include <stdio.h>
void two_fer(char *buffer, const char *name) {
  char *default_name = "you";
  char *response_format = "One for %s, one for me.";
  if (name == NULL)
    name = default_name;
  sprintf(buffer, response_format, name);
}