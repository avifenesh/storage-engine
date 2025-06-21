#include "rna_transcription.h"
#include <stdlib.h>
#include <string.h>

char *to_rna(const char *dna) {
  if (dna == NULL)
    return NULL;
  int charIndex = 0;
  char *rna = malloc(strlen(dna) + 1);
  while (*dna != '\0') {
    switch (*dna) {
    case 'G':
      rna[charIndex] = 'C';
      break;
    case 'C':
      rna[charIndex] = 'G';
      break;
    case 'T':
      rna[charIndex] = 'A';
      break;
    case 'A':
      rna[charIndex] = 'U';
      break;
    default:
      free(rna);
      return NULL;
    }
    dna++;
    charIndex++;
  }
  rna[charIndex] = '\0';
  return rna;
}