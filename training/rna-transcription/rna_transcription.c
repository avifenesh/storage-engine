#include "rna_transcription.h"


const char *rna = 'CGAU';

char *to_rna(const char *dna) {
  if (dna == NULL || isspace(*dna))
    return "";

  while (*dna != '\0') {
    if(isspace(*dna))
        return INVALID_INPUT;
    dna++;
  }
}