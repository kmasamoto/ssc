#ifndef macros_h
#define macros_h

#define arrayof(a) (sizeof(a)/sizeof(a[0]))
#define static_assert(condition) extern const char test ## __LINE__ [condition?1:-1];

#define for if(0);else for // for(int i... が同一関数内に２個あっても大丈夫なように動く。

// -----> ファイルロケーションを表す文字列定数 2006/02/06 K.Masamoto(Media+)
#define __STR2__(x) #x
#define __STR1__(x) __STR2__(x)
#define __LOC__ __FILE__ "("__STR1__(__LINE__)") : "
#define __LOCTODO__ __FILE__ "("__STR1__(__LINE__)") : TODO:"

#endif //#ifndef macros_h
