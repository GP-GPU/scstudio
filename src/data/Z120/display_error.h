#ifndef _Z120Display_H
#define _Z120Display_H

#include<antlr3.h>

#ifdef __cplusplus
extern "C" {
#endif

void reportError (pANTLR3_BASE_RECOGNIZER recognizer);

void display_error(struct Context* context, pANTLR3_BASE_RECOGNIZER recognizer, pANTLR3_UINT8 *tokenNames);

void lexer_error(struct s_Z120* z, pANTLR3_BASE_RECOGNIZER recognizer, pANTLR3_UINT8 *tokenNames);

#ifdef __cplusplus
}
#endif

#endif // _Z120Display_H

// $Id: display_error.h 1006 2010-12-08 20:34:49Z madzin $
