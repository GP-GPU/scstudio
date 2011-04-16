#include "data/time.h"
#define TEST_ASSERT(x) if(!(x)) \
{ \
  std::cerr << __FILE__ \
  << ": Assertion "  #x " failed on line " \
  <<  __LINE__  << std::endl; return 1; \
}

using namespace std;


int main()
{
  MscTimeIntervalSet<double> set1;
  MscTimeIntervalSet<double> set2;
  MscTimeIntervalSet<double> control;
  MscTimeIntervalSet<double> result;
  
  MscTimeInterval<double> inter1(1,4);
  MscTimeInterval<double> inter2(6,8);

  MscTimeInterval<double> inter3(0,1);
  MscTimeInterval<double> inter4(3,5);
  MscTimeInterval<double> inter5(6,7);

  
  MscTimeInterval<double> inter6(1,1);
  MscTimeInterval<double> inter7(3,4);
  
  
  set1.insert(inter1);
  set1.insert(inter2);

  set2.insert(inter3);
  set2.insert(inter4);
  set2.insert(inter5);

  control.insert(inter6);
  control.insert(inter7);
  control.insert(inter5);
  
  result = MscTimeIntervalSet<double>::set_intersection(set1,set2);
  cout << result << endl;
  cout << control << endl;

  TEST_ASSERT(control==result);

  set1.clear();
  set2.clear();
  result.clear();
  control.clear();

  inter1.set(1,2);
  inter2.set(4,5);
  inter3.set(7,9);


  inter4.set(1,5);
  inter5.set(10,12);

  inter6.set(1,5);

  control.insert(inter5);
  control.insert(inter3);
  control.insert(inter6);

  set1.insert(inter1);
  set1.insert(inter2);
  set1.insert(inter3);

  set2.insert(inter4);
  set2.insert(inter5);

  result=MscTimeIntervalSet<double>::components_max(set1,set2);

  cout << result << endl;
  cout << control << endl;
  TEST_ASSERT(result==control);


  set1.clear();
  set2.clear();
  result.clear();
  control.clear();

  inter1.set(-1,0);
  inter2.set(2,4);

  inter4.set(0,1);
  inter5.set(4,4);

  inter6.set(-1,1);
  inter7.set(2,5);
  inter3.set(6,8);

  control.insert(inter6);
  control.insert(inter7);
  control.insert(inter3);

  set1.insert(inter1);
  set1.insert(inter2);

  set2.insert(inter4);
  set2.insert(inter5);

  result=set1+set2;

  cout << result << endl;
  cout << control << endl;
  TEST_ASSERT(result==control);


  MscTimeIntervalSet<double> s1;
  MscTimeIntervalSet<double> s2;
  MscTimeIntervalSet<double> s3;
  MscTimeIntervalSet<double> s4;

  MscTimeInterval<double> i1(10,13);
  MscTimeInterval<double> i2(6,8);
  MscTimeInterval<double> i3(6,10);
  MscTimeInterval<double> i4(1,2);
  MscTimeInterval<double> i5(3,4);
  MscTimeInterval<double> i6("(0,inf)");
  MscTimeInterval<double> i7("[-1]");
  s1.insert(i1);
  s2.insert(i2);
  s3.insert(i3);
  s3 = s1 - s2 -s3;
  s4.insert(i4);
  s4.insert(i5);
  
  std::cerr << std::endl;
  std::cerr << std::endl;
  std::cerr << std::endl;
  std::cerr << s3 << std::endl;
  std::cerr << s4 << std::endl;
  std::cerr <<"intersection: " << MscTimeIntervalSet<double>::set_intersection(i6,i7) << std::endl;
  return 0;
}
