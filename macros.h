#ifndef macros_h
#define macros_h

#define arrayof(a) (sizeof(a)/sizeof(a[0]))
#define static_assert(condition) extern const char test ## __LINE__ [condition?1:-1];

#define for if(0);else for // for(int i... ������֐����ɂQ�����Ă����v�Ȃ悤�ɓ����B

// -----> �t�@�C�����P�[�V������\��������萔 2006/02/06 K.Masamoto(Media+)
#define __STR2__(x) #x
#define __STR1__(x) __STR2__(x)
#define __LOC__ __FILE__ "("__STR1__(__LINE__)") : "
#define __LOCTODO__ __FILE__ "("__STR1__(__LINE__)") : TODO:"

#endif //#ifndef macros_h
