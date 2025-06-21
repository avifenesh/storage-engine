#include "rna_transcription.h"
#include <stdio.h>
char *to_rna(const char *dna) {
  if (dna == NULL || isspace(*dna))
    return "";

  char *rna = malloc(2 * sizeof(char));
  int capacity = 2;
  int size = 0;

  while (*dna != '\0') {
    if (isspace(*dna))
      return INVALID;

    if (capacity <= size) {
      rna = (char *)realloc(rna, capacity * 2 * (sizeof(char)));
      capacity *= 2;
    }

    switch (*dna) {
    case 'G':
      rna[size] = 'C';
      break;
    case 'C':
      rna[size] = 'G';
      break;
    case 'T':
      rna[size] = 'A';
      break;
    case 'A':
      rna[size] = 'U';
      break;
    default:
      return INVALID;
    }
    dna++;
    size++;
  }
  rna[size] = '\0';
  return rna;
}