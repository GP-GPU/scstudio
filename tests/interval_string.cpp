#include "data/time.h"
#include<string>


#define TEST_ASSERT(x) if(!(x)) \
{ \
  std::cerr << __FILE__ \
  << ": Assertion "  #x " failed on line " \
  <<  __LINE__  << std::endl; return 1; \
}
/**
 * \brief Trying to construct interval from string
 * \param error says if the interval string is expected to be valid
 * \return 0 to test successfully else 1
 */

int test_interval_set(std::string int_string,bool valid)
{
  std::cerr << "Interval set string: " << int_string << " expected to be ";
  if(valid)
    std::cerr << "valid";
  else
    std::cerr << "INvalid";
  std::cerr << std::endl;
  try 
  {
    MscTimeIntervalSet<double> set(int_string);
    std::cerr << "Interval set: " << set << std::endl;
  }
  catch(MscIntervalStringConversionError ex){
    std::cerr << "Exception: " << ex.what() << std::endl;
    if(valid)
      std::cerr << "!!! ERROR" << std::endl;
    std::cerr << std::endl;
    return valid;
  }
  if(!valid)
    std::cerr << "!!! ERROR" << std::endl;
  std::cerr << std::endl;
  return !valid;
}

int main()
{
/*
  MscTimeInterval<double> dou(false,10,18,true);
  MscTimeInterval<DecScaled> dec(false,10,18,true);

//  MscTimeInterval<double> inter_double(std::string("(     10  ,  18] sd "));
//  MscTimeInterval<DecScaled> inter_dec(std::string(" (  1 0   , 1   8]    "));
//  std::cerr << inter_double << std::endl;
//  std::cerr << dou << std::endl;

//  TEST_ASSERT(dou==inter_double);

//  std::cerr << inter_dec << std::endl;
//  std::cerr << dec << std::endl;

//  TEST_ASSERT(dec==inter_dec);

  try {
    MscTimeInterval<double> a(std::string("  9 9 "));
    TEST_ASSERT(false);
  } 
  catch(MscIntervalStringConversionError){}

  try { 
    MscTimeInterval<DecScaled> b(std::string("5,4]"));
    TEST_ASSERT(false);
  }
  catch(MscIntervalStringConversionError){}

  try { 
    MscTimeInterval<DecScaled> c(std::string("[5,4"));
    TEST_ASSERT(false);
  }
  catch(MscIntervalStringConversionError){}
*/
  int ret;
  ret = 0;


  ret+=test_interval_set("10+[10]+50+[30,40)",true);
  ret+=test_interval_set("(1,2) + (1.2,13]",true);
  ret+=test_interval_set("[-1e+10,+2e-2]",true);
  ret+=test_interval_set("(-inf,+inf)",true);
  ret+=test_interval_set(" ( inf, 3.23) + [4,6543]",false);
  ret+=test_interval_set("(5.431,4.876)+(1,2)",false);
  ret+=test_interval_set("(0.123,inf]",false);
  ret+=test_interval_set("( inf, 3.23,1) + [4,6543]",false);
  ret+=test_interval_set("[(4)]",false);
  ret+=test_interval_set("[-inf,1)",false);
  ret+=test_interval_set("(-inf,1)",true);


  std::cerr << std::endl << "Number of errors: " << ret << std::endl;

//  MscTimeInterval<DecScaled> c(std::string("405"));
  return ret;
}
