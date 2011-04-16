#include<iostream>
#include"check/time/time_consistency.h"
#include"check/time/tightening.h"
#include <limits>

int main()
{
  MscSolveTCSP sol;

///////////////////////////////////////////////////

  IntervalMatrix m(5,5);
  MscTimeIntervalD int2(30,40);
  MscTimeIntervalD int3(10,20);
  MscTimeIntervalD int4(40,50);
  MscTimeIntervalD int5(60,70);


  for(unsigned j=0;j<m.size1();j++){
    for(unsigned i=0;i<m.size1();i++){
       if(i==j)
         m(j,i) = MscTimeIntervalD(true,0,0,true);
       else
         m(j,i) = MscTimeIntervalD(false,-D::infinity(),D::infinity(),false);
     }
  }
  m(0,1) = int3;
  m(1,0) = MscTimeIntervalD::interval_inverse(int3);

  m(1,2) = int2;
  m(2,1) = MscTimeIntervalD::interval_inverse(int2);

  m(3,2) = int3;
  m(2,3) = MscTimeIntervalD::interval_inverse(int3);

  m(3,4) = int4;
  m(4,3) = MscTimeIntervalD::interval_inverse(int4);

  m(0,4) = int5;
  m(4,0) = MscTimeIntervalD::interval_inverse(int5);

  IntervalMatrix result;
  FloydWarshall check;
  if(check.check_consistency(m))
    std::cout << "tadaaaaa" << std::endl;
  else
    std::cout << "nenenene" << std::endl;

  result = check.tight(m);
  std::cout << m << std::endl;
  std::cout << result << std::endl;
  std::cout << "//////////////////////////////////////" << std::endl;
/////////////////////////////////////////////////////////////////
  IntervalSetMatrix ms(5,5);
 
  MscTimeIntervalD int6(10,20);
  MscTimeIntervalD int7(10,50);
  MscTimeIntervalD int8(30,40);
  MscTimeIntervalD int9(60,D::infinity());//TODO: infi
  MscTimeIntervalD int10(20,30);   
  MscTimeIntervalD int11(40,50);
  MscTimeIntervalD int12(60,70);
  MscTimeIntervalD int13(20,70);



  MscTimeIntervalSetD ints1;
  MscTimeIntervalSetD ints2;
  MscTimeIntervalSetD ints3;
  MscTimeIntervalSetD ints4;
  MscTimeIntervalSetD ints5;
  MscTimeIntervalSetD ints6;
  MscTimeIntervalSetD ints7;
  MscTimeIntervalSetD ints8;
  
  ints1.insert(int6);
  ints2.insert(int7);

  ints3.insert(int8);
  ints3.insert(int9);

  ints5.insert(int10);
  ints5.insert(int11);

  ints6.insert(int12);
  ints7.insert(int13);
  ints8.insert(int6);

 for(unsigned j=0;j<ms.size1();j++){
    for(unsigned i=0;i<ms.size1();i++){
       if(i==j)
       {
         ms(j,i) = MscTimeIntervalSetD();
	 ms(j,i).insert(MscTimeIntervalD(0,0));
 	}
       else
         ms(j,i) = MscTimeIntervalSetD();
	 ms(j,i).insert(MscTimeIntervalD(false,-D::infinity(),D::infinity(),false));
     }
  }

  ms(0,1) = ints1;
  ms(0,3) = ints2;
  ms(0,2) = ints7;
  ms(0,4) = ints6;
  ms(1,2) = ints3;
  ms(3,2) = ints8;
  ms(3,4) = ints5;
  std::cout << sol.solve(ms) << std::endl;
  std::cout << "//////////////////////////////////////" << std::endl;
/////////////////////////////////////////////////////////////

  IntervalSetMatrix omg(4,4);
 
  MscTimeIntervalD int16(10,20);
  MscTimeIntervalD int17(0,1);
  MscTimeIntervalD int18(0,10);
  MscTimeIntervalD int19(25,50);//TODO: infi
  MscTimeIntervalD int20(0,20);   
  MscTimeIntervalD int21(40,40);



  MscTimeIntervalSetD ints11;
  MscTimeIntervalSetD ints12;
  MscTimeIntervalSetD ints13;
  MscTimeIntervalSetD ints14;
  
  ints11.insert(int17);
  ints11.insert(int16);

  ints12.insert(int18);

  ints13.insert(int20);
  ints13.insert(int21);

  ints14.insert(int19);


 for(unsigned j=0;j<omg.size1();j++){
    for(unsigned i=0;i<omg.size1();i++){
       if(i==j)
       {
         omg(j,i) = MscTimeIntervalSetD();	 
	 omg(j,i).insert(MscTimeIntervalD(0,0));
 	}
       else
       {
         omg(j,i) = MscTimeIntervalSetD();
	 omg(j,i).insert(MscTimeIntervalD(false,-D::infinity(),D::infinity(),false));
       }
     }
  }

  omg(0,1) = ints11;
  omg(1,2) = ints12;
  omg(2,3) = ints13;
  omg(1,3) = ints14;

  omg(1,0) = MscTimeIntervalSetD::interval_inverse(ints11);
  omg(2,1) = MscTimeIntervalSetD::interval_inverse(ints12);
  omg(3,2) = MscTimeIntervalSetD::interval_inverse(ints13);
  omg(3,1) = MscTimeIntervalSetD::interval_inverse(ints14);
  std::cout << omg << std::endl;
  std::cout << sol.solve(omg) << std::endl;
  std::cout << "//////////////////////////////////////" << std::endl;
/////////////////////////////////////////////////////////////

  IntervalSetMatrix karel(5,5);
 
  MscTimeIntervalD in6(10,20);
  MscTimeIntervalD in7(10,50);
  MscTimeIntervalD in8(30,40);
  MscTimeIntervalD in9(60,D::infinity());
  MscTimeIntervalD in10(20,30);   
  MscTimeIntervalD in11(40,50);
  MscTimeIntervalD in12(60,70);
  MscTimeIntervalD in13(20,70);



  MscTimeIntervalSetD ins1;
  MscTimeIntervalSetD ins2;
  MscTimeIntervalSetD ins3;
  MscTimeIntervalSetD ins4;
  MscTimeIntervalSetD ins5;
  MscTimeIntervalSetD ins6;
  MscTimeIntervalSetD ins7;
  MscTimeIntervalSetD ins8;
  
  ins1.insert(int6);

  ins2.insert(in8);
  ins2.insert(in9);

  ins3.insert(in6);

  ins4.insert(in10);
  ins4.insert(in11);

  ins5.insert(in12);

  
 for(unsigned j=0;j<karel.size1();j++){
    for(unsigned i=0;i<karel.size1();i++){
       if(i==j)
       {
         karel(j,i) = MscTimeIntervalSetD();
	 karel(j,i).insert(MscTimeIntervalD(0,0));
 	}
       else
       {
         karel(j,i) = MscTimeIntervalSetD();
	 karel(j,i).insert(MscTimeIntervalD(false,-D::infinity(),D::infinity(),false));
       }
     }
  }

  karel(0,1) = ins1;
  karel(1,0) = MscTimeIntervalSetD::interval_inverse(ins1);

  karel(1,2) = ins2;
  karel(2,1) = MscTimeIntervalSetD::interval_inverse(ins2);

  karel(3,2) = ins3;
  karel(2,3) = MscTimeIntervalSetD::interval_inverse(ins3);

  karel(3,4) = ins4;
  karel(4,3) = MscTimeIntervalSetD::interval_inverse(ins4);

  karel(0,4) = ins5;
  karel(4,0) = MscTimeIntervalSetD::interval_inverse(ins5);

  std::cerr << karel << std::endl;
  IntervalMatrixFunc::print_out(karel);
  std::cout << sol.solve(karel) << std::endl;


  return 0;
}
