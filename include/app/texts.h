#ifndef TEXTS_H
#define TEXTS_H

typedef enum
{
  TXT_ID_NONE = 0,

#define CREATE_TXT(A,B) A,
#include <app/texts_def.h>
#undef CREATE_TXT

  TXT_ID_LAST

} E_TXT_ID;

char *TXT_pcGetText(E_TXT_ID);

#endif /* TEXTS_H */
