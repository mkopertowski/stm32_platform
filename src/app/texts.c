//*****************************************************************************
/** \file    texts.c
 *  \author  Mirek Kopertowski m.kopertowski/at/post.pl
 *  \brief
 */
//*****************************************************************************
#include <app/texts.h>

// create tables with strings
#define CREATE_TXT(A,B) const char a##A[] = B;

#include <app/texts_def.h>

#undef CREATE_TXT

// table of pointers to strings; indexed by E_TXT_ID
const char *aTextMap[] =
{
  0,
#define CREATE_TXT(A,B) a##A,
#include <app/texts_def.h>
#undef CREATE_TXT
  0
};

//*****************************************************************************
/** \brief   Get static text stored in program space memory
 *
 *
 *  \return  pointer to string
 */
//*****************************************************************************
char *TXT_pcGetText(E_TXT_ID eTxtId)
{
    switch(eTxtId)
    {
        case TXT_ID_NONE:
        case TXT_ID_LAST:
            return 0;
        default:
            return aTextMap[eTxtId];
    }
}

