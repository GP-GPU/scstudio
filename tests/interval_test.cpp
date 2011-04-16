#include "data/time.h"

#define TEST_ASSERT(x) if(!(x)) \
{ \
  std::cerr << __FILE__ \
  << ": Assertion "  #x " failed on line " \
  <<  __LINE__  << std::endl; return 1; \
}

int main()
{
  /* MscIntervalCouple */
  MscIntervalCouple<double> couple1(true,9);
  MscIntervalCouple<double> couple2(true,10);
  MscIntervalCouple<double> couple3(true,9);
  
  TEST_ASSERT(couple1<couple2)
  TEST_ASSERT(!(couple1<couple3))

  couple3.set_closed(false);
  try {
    couple1<couple3;
    TEST_ASSERT(false)
  }
  catch(MscIntervalCoupleUncomparable ex)
  {
  }
  TEST_ASSERT(couple1==std::min(couple1,couple2))
  TEST_ASSERT(couple2==std::max(couple1,couple2))

  MscIntervalCouple<double> couple4(false,0);
  couple4=couple1+couple2;
  TEST_ASSERT(couple4.get_closed())
  TEST_ASSERT(couple4.get_value()==19)

  couple3.set_value(98);
  couple4=couple1+couple3;
  TEST_ASSERT(!couple4.get_closed())
  TEST_ASSERT(couple4.get_value()==107)
  /* MscTimeInterval */  
  MscTimeInterval<double> interval2(10);
  MscTimeInterval<double> interval3(14,15);
  MscTimeInterval<double> interval4(9,15);
  MscTimeInterval<double> interval6(false,14,15,false);
  MscTimeInterval<double> interval9(14);
  MscTimeInterval<double> interval11(couple1,couple2);

  TEST_ASSERT(10==interval2.get_begin_value())
  TEST_ASSERT(10==interval2.get_end_value())
  TEST_ASSERT(14==interval3.get_begin_value())
  TEST_ASSERT(15==interval3.get_end_value())

  MscTimeInterval<double> interval;
  interval = interval2+interval3;
  TEST_ASSERT(24==interval.get_begin_value())
  TEST_ASSERT(25==interval.get_end_value())

  MscTimeInterval<double> interval5;
  MscTimeInterval<double> interval7;
  MscTimeInterval<double> interval8;
  MscTimeInterval<double> interval10;

  interval5 = MscTimeInterval<double>::interval_intersection(interval3,interval4);
  interval7 = MscTimeInterval<double>::interval_intersection(interval4,interval3);
  interval8 = MscTimeInterval<double>::interval_intersection(interval4,interval6);
  TEST_ASSERT(interval5.is_valid())
  TEST_ASSERT(interval5==interval7)
  TEST_ASSERT(interval7==interval5)
  TEST_ASSERT(14==interval5.get_begin_value())
  TEST_ASSERT(15==interval5.get_end_value())
 
  TEST_ASSERT(interval5.get_begin_closed())
  TEST_ASSERT(interval5.get_end_closed())


  TEST_ASSERT(interval7!=interval8)
  TEST_ASSERT(interval7.get_begin_value()==interval8.get_begin_value())
  TEST_ASSERT(interval7.get_end_value()==interval8.get_end_value())
  TEST_ASSERT(!interval8.get_begin_closed())
  TEST_ASSERT(!interval8.get_end_closed())

  interval10 = MscTimeInterval<double>::interval_intersection(interval9,interval6);
  TEST_ASSERT(!interval10.is_valid())
  interval10 = MscTimeInterval<double>::interval_intersection(interval9,interval3);
  TEST_ASSERT(interval10.is_valid())
  TEST_ASSERT(interval10==interval9)
 
  interval10 = interval10*3;
  TEST_ASSERT(42==interval10.get_begin_value())
  TEST_ASSERT(42==interval10.get_end_value())
  TEST_ASSERT(interval10.get_begin_closed())
  TEST_ASSERT(interval10.get_end_closed())


  MscTimeInterval<double> inter1(10,18);
  MscTimeInterval<double> inter2(4,11);
  MscTimeInterval<double> inter3(4,9);

  interval10 = MscTimeInterval<double>::interval_union(inter1,inter2);
  TEST_ASSERT(interval10.is_valid())
  TEST_ASSERT(4==interval10.get_begin_value())
  TEST_ASSERT(18==interval10.get_end_value())
 
  interval10 = MscTimeInterval<double>::interval_union(inter1,inter3);
  TEST_ASSERT(!interval10.is_valid())
  return 0;
}
