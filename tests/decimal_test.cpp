#include"data/time.h"

#define TEST_ASSERT(x) if(!(x)) \
{ \
  std::cerr << __FILE__ \
  << ": Assertion "  #x " failed on line " \
  <<  __LINE__  << std::endl; return 1; \
}

int main()
{
  // Constructor
  DecScaled dec1(50,1);
  DecScaled dec2(5,2);
  TEST_ASSERT(dec1==dec2);


  DecScaled dec3(1,0);
  DecScaled dec4(1,0);
  DecScaled dec5(1,3);
  DecScaled dec6(123,5);
  DecScaled dec7(23000000432LL,-4);
  DecScaled dec8(146000000432LL,-4);
  DecScaled dec9(1,0);

  // addition 
  dec3 = dec2+dec1;
  dec4 = dec1+dec2;
  dec9=dec6+dec7;

  TEST_ASSERT(dec9==dec8);
  TEST_ASSERT(dec3==dec4);
  TEST_ASSERT(dec3==dec5);
  
  // subtraction
  dec5=dec3-dec1;
  TEST_ASSERT((dec3-dec1)==dec2);
  TEST_ASSERT((dec1-dec3)==-dec2);
  
  // multiplication
  DecScaled dec_m1(25,-3);
  DecScaled dec_m2(25,-3);
  

  return 0;
}

