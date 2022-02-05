
#include "hp_charset.h"

// Credits to Xi from EEvblog for decoding!

#define Da  (1<< 2)
#define Db  (1<< 0)
#define Dc  (1<< 9)
#define Dd  (1<< 1)
#define De  (1<<12)
#define Df  (1<< 3)
#define Dg1 (1<<10)
#define Dg2 (1<< 4)
#define Dk  (1<< 5)
#define Dm  (1<<11)
#define Dn  (1<< 8)
#define Dr  (1<< 7)
#define Ds  (1<<13)
#define Dt  (1<< 6)

const uint16_t hp_charset[] PROGMEM =
{
	Da|Db|Dd|De|Df|Dg2|Dm,  // 0 : @
	Da|Db|Dc|De|Df|Dg1|Dg2, // 1 : A
	Da|Db|Dc|Dd|Dg2|Dm|Ds,  // 2 : B
	Da|Dd|De|Df,            // 3 : C
	Da|Db|Dc|Dd|Dm|Ds,      // 4 : D
	Da|Dd|De|Df|Dg1|Dg2,    // 5 : E
	Da|De|Df|Dg1|Dg2,       // 6 : F
	Da|Dc|Dd|De|Df|Dg2,     // 7 : G
	Db|Dc|De|Df|Dg1|Dg2,    // 8 : H
	Da|Dd|Dm|Ds,            // 9 : I
	Db|Dc|Dd|De,            //10 : J
	De|Df|Dg1|Dn|Dt,        //11 : K
	Dd|De|Df,               //12 : L
	Db|Dc|De|Df|Dk|Dn,      //13 : M
	Db|Dc|De|Df|Dk|Dt,      //14 : N
	Da|Db|Dc|Dd|De|Df,      //15 : O
	Da|Db|De|Df|Dg1|Dg2,    //16 : P
	Da|Db|Dc|Dd|De|Df|Dt,   //17 : Q
	Da|Db|De|Df|Dg1|Dg2|Dt, //18 : R
	Da|Dc|Dd|Df|Dg1|Dg2,    //19 : S
	Da|Dm|Ds,               //20 : T
	Db|Dc|Dd|De|Df,         //21 : U
	De|Df|Dn|Dr,            //22 : V
	Db|Dc|De|Df|Dr|Dt,      //23 : W
	Dk|Dn|Dr|Dt,            //24 : X
	Dk|Dn|Ds,               //25 : Y
	Da|Dd|Dn|Dr,            //26 : Z
	Da|Dd|De|Df,            //27 : [
	Dk|Dt,                  /*28 : \ */
	Da|Db|Dc|Dd,            //29 : ]
	Da|Db|Dn|Dr,            //30 : top-right pointing arrow
	Dd,                     //31 : _
	0,                      //32 :   (space)
	Dm|Ds,                  //33 : !
	Df|Dm,                  //34 : "
	Db|Dc|Dd|Dg1|Dg2|Dm|Ds, //35 : #
	Da|Dc|Dd|Df|Dg1|Dg2|Dm|Ds, //: $
	Dc|Df|Dg1|Dg2|Dk|Dn|Dr|Dt, //: %
	Da|Dc|Dd|Dk|Dn|Dr|Dt,   //38 : &
	Dm,                     //39 : '
	Dn|Dt,                  //40 : (
	Dk|Dr,                  //41 : )
	Dg1|Dg2|Dk|Dm|Dn|Dr|Ds|Dt, //: *
	Dg1|Dg2|Dm|Ds,          //43 : +
	Dg1|Dg2|Dn|Dt,          //44 : <-
	Dg1|Dg2,                //45 : -
	Dg1|Dg2|Dk|Dr,          //46 : ->
	Dr|Dn,                  //47 : /
	Da|Db|Dc|Dd|De|Df|Dn|Dr,//48 : 0
	Db|Dc,                  //49 : 1
	Da|Db|Dd|De|Dg1|Dg2,    //50 : 2
	Da|Db|Dc|Dd|Dg1|Dg2,    //51 : 3
	Db|Dc|Df|Dg1|Dg2,       //52 : 4
	Da|Dc|Dd|Dk|Dg2,        //53 : 5
	Da|Dc|Dd|De|Df|Dg1|Dg2, //54 : 6
	Da|Db|Dc,               //55 : 7
	Da|Db|Dc|Dd|De|Df|Dg1|Dg2, //: 8
	Da|Db|Dc|Dd|Df|Dg1|Dg2, //57 : 9
	Da|Db|Dc|Dd|De|Df|Dg1|Dg2|Dk|Dm|Dn|Dr|Ds|Dt, //58 : all digits lit
	Ds,                     //59 : ;
	Dd|Dn|Dr,               //60 : <
	Dd|Dg1|Dg2,             //61 : =
	Dd|Dk|Dt,               //62 : >
	Da|Db|Df|Dg2|Ds,        //63 : ?
	
	De|Df|Dg1|Dg2,          //64 : |-
	Dc|Dd|De|Dg1|Dt,        //65 : a
	Dc|Dd|De|Df|Dg1|Dg2,    //66 : b
	Dd|De|Dg1|Dg2,          //67 : c
	Db|Dc|Dd|De|Dg1|Dg2,    //68 : d
	Dd|De|Dg1|Dr,           //69 : e
	Da,                     //70 : top segment (overscore)
	Da|Dm,                  //71 : hourglass (1/5)
	Da|Dm|Dr,               //72 : hourglass (2/5)
	Da|Dm|Dr|Dt,            //73 : hourglass (3/5)
	Da|Dm|Dr|Dt|Dg1,        //74 : hourglass (4/5)
	Da|Dm|Dr|Dt|Dg1|Dg2,    //75 : hourglass (5/5)
	Db|Dg2|Dm|Dr,           //76 : µ
	Dd|Dg1|Dg2|Dn|Dr,       //77 : =/= (not equal)
	Da|Dd|Dk|Dr,            //78 : SIGMA sign
	Dd|Dn|Dr|Ds             //79 : antenna ?
};
