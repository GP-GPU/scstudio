#include "data/Z120/display_error.h"
#include "data/Z120/Context_Impl.h"

// Implementation based on src/runtime/C/antlr3baserecognizer.c, display()

void reportError (pANTLR3_BASE_RECOGNIZER recognizer)
{
  if  (recognizer->state->errorRecovery == ANTLR3_TRUE)
  {
    return;
  }

  // Signal we are in error recovery now
  recognizer->state->errorRecovery = ANTLR3_TRUE;

  // Indicate this recognizer had an error while processing.
  recognizer->state->errorCount++;

  // Call the error display routine
  recognizer->displayRecognitionError(recognizer, recognizer->state->tokenNames);
}

void display_error(struct Context* context, pANTLR3_BASE_RECOGNIZER recognizer, pANTLR3_UINT8 *tokenNames)
{
  stringize report;

  // Retrieve some info for easy reading.
  pANTLR3_EXCEPTION ex = recognizer->state->exception;
  pANTLR3_COMMON_TOKEN    theToken= (pANTLR3_COMMON_TOKEN)(recognizer->state->exception->token);

  // See if there is a 'filename' we can use
  if (ex->streamName == NULL)
  {
    if (((pANTLR3_COMMON_TOKEN)(ex->token))->type == ANTLR3_TOKEN_EOF)
    {
      report << "<EOF>";
    }
    else
    {
      report << "<unknown>";
    }
  }
  else
  {
    pANTLR3_STRING ftext = ex->streamName->to8(ex->streamName);

    char* last_slash = (char *)ftext->chars;
    bool slash_present = false;
    // strip file path
    for(char *ch = (char *)ftext->chars; *ch != 0; ch++)
    {
      if(*ch == '\\' || *ch == '/')
      {
        last_slash = ch;
        slash_present = true;
      }
    }

    if(slash_present)
    {
      report << last_slash+1
        << "[" << theToken->getLine(theToken)
        << "," << theToken->getCharPositionInLine(theToken) << "]";
    }
    else
    {
      report << last_slash
        << "[" << theToken->getLine(theToken) 
        << "," << theToken->getCharPositionInLine(theToken) << "]";
    }
  }

  report << " ";

  // Note that in the general case, errors thrown by tree parsers indicate a problem
  // with the output of the parser or with the tree grammar itself. The job of the parser
  // is to produce a perfect (in traversal terms) syntactically correct tree, so errors
  // at that stage should really be semantic errors that your own code determines and handles
  // in whatever way is appropriate.
  switch (ex->type)
  {
    case ANTLR3_UNWANTED_TOKEN_EXCEPTION:

      // Indicates that the recognizer was fed a token which seesm to be
      // spurious input. We can detect this when the token that follows
      // this unwanted token would normally be part of the syntactically
      // correct stream. Then we can see that the token we are looking at
      // is just something that should not be there and throw this exception.
      if (tokenNames == NULL)
      {
        report << "Unwanted input";
      }
      else
      {
        if (ex->expecting == ANTLR3_TOKEN_EOF)
        {
          report << "Unwanted input: expected <EOF>";
        }
        else
        {
          report << "Unwanted input: expected " << (char*)(tokenNames[ex->expecting]);
        }
      }
      break;

    case ANTLR3_MISSING_TOKEN_EXCEPTION:

      // Indicates that the recognizer detected that the token we just
      // hit would be valid syntactically if preceeded by a particular 
      // token. Perhaps a missing ';' at line end or a missing ',' in an
      // expression list, and such like.
      if (tokenNames == NULL)
      {
        report << "Missing token (" << ex->expecting << ")";
      }
      else
      {
        if (ex->expecting == ANTLR3_TOKEN_EOF)
        {
          report << "Missing <EOF>";
        }
        else
        {
          report << "Missing " << (char*)(tokenNames[ex->expecting]);
        }
      }
      break;

    case ANTLR3_RECOGNITION_EXCEPTION:

      // Indicates that the recognizer received a token
      // in the input that was not predicted. This is the basic exception type 
      // from which all others are derived. So we assume it was a syntax error.
      // You may get this if there are not more tokens and more are needed
      // to complete a parse for instance.
      report << "Syntax error";
      break;

    case ANTLR3_MISMATCHED_TOKEN_EXCEPTION:

      // We were expecting to see one thing and got another. This is the
      // most common error if we coudl not detect a missing or unwanted token.
      // Here you can spend your efforts to
      // derive more useful error messages based on the expected
      // token set and the last token and so on. The error following
      // bitmaps do a good job of reducing the set that we were looking
      // for down to something small. Knowing what you are parsing may be
      // able to allow you to be even more specific about an error.
      if (tokenNames == NULL)
      {
        report << "Syntax error";
      }
      else
      {
        if (ex->expecting == ANTLR3_TOKEN_EOF)
        {
          report << "Expected <EOF>";
        }
        else
        {
          report << "Expected " << tokenNames[ex->expecting];
        }
      }
      break;

    case ANTLR3_NO_VIABLE_ALT_EXCEPTION:

      // We could not pick any alt decision from the input given
      // so god knows what happened - however when you examine your grammar,
      // you should. It means that at the point where the current token occurred
      // that the DFA indicates nowhere to go from here.
      report << "Cannot match to any predicted input";
      break;

    case ANTLR3_MISMATCHED_SET_EXCEPTION:
    {

      // This means we were able to deal with one of a set of
      // possible tokens at this point, but we did not see any
      // member of that set.
      report << "Unexpected input";

      // What tokens could we have accepted at this point in the parse?
      pANTLR3_BITSET errBits = antlr3BitsetLoad(ex->expectingSet);
      ANTLR3_UINT32 numbits = errBits->numBits(errBits);
      ANTLR3_UINT32 size = errBits->size(errBits);

      if (size > 0)
      {
        ANTLR3_UINT32 count = 0;
        report << ": expected one of";

        // However many tokens we could have dealt with here, it is usually
        // not useful to print ALL of the set here. I arbitrarily chose 8
        // here, but you should do whatever makes sense for you of course.
        // No token number 0, so look for bit 1 and on.
        for(ANTLR3_UINT32 bit = 1; bit < numbits && count < 8 && count < size; bit++)
        {
          // TODO: This doesn;t look right - should be asking if the bit is set!!
          if (tokenNames[bit])
          {
            report << (count > 0 ? ", " : " ") << "<" << tokenNames[bit] << ">";
            count++;
          }
        }
      }

      break;
    }

    case ANTLR3_EARLY_EXIT_EXCEPTION:

      // We entered a loop requiring a number of token sequences
      // but found a token that ended that sequence earlier than
      // we should have done.
      report << "Missing elements";
      break;

    default:

      // We don't handle any other exceptions here, but you can
      // if you wish. If we get an exception that hits this point
      // then we are just going to report what we know about the
      // token.
      report << "Syntax not recognized";
      break;
  }

  report << ".";

  context->z->print_report(RS_ERROR, report);
}

void lexer_error(struct s_Z120* z, pANTLR3_BASE_RECOGNIZER recognizer, pANTLR3_UINT8 *tokenNames)
{
  Z120* printer = static_cast<Z120*> (z);
  stringize report;

  // Retrieve some info for easy reading.
  pANTLR3_LEXER lexer = (pANTLR3_LEXER)(recognizer->super);
  pANTLR3_EXCEPTION ex = lexer->rec->state->exception;

  // See if there is a 'filename' we can use
  if(ex->name == NULL)
    report << "-unknown source-";
  else
  {
    pANTLR3_STRING ftext = ex->streamName->to8(ex->streamName);

    char* last_slash = (char *)ftext->chars;
    bool slash_present = false;
    // strip file path
    for(char *ch = (char *)ftext->chars; *ch != 0; ch++)
    {
      if(*ch == '\\' || *ch == '/')
      {
        last_slash = ch;
        slash_present = true;
      }
    }

    if(slash_present)
      report << last_slash+1;
    else
      report << last_slash;
  }
  report << "[" << recognizer->state->exception->line << "," << ex->charPositionInLine+1 << "] Lexer error.";

  printer->print_report(RS_ERROR, report);
}

// $Id: display_error.cpp 1013 2010-12-13 16:34:52Z madzin $
