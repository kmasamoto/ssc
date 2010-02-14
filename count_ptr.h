#ifndef count_ptr
#define count_ptr

/*
* �Q�ƃJ�E���g�ɂ����S�ȎQ�ƃx�[�X�R���e�i
*/
template <class X>
class count_ptr {
	X*          ptr_;
	unsigned*   cnt_;

	void inc_ref(const count_ptr& r);
	void dec_ref();

public:
	// �R���X�g���N�^
	explicit count_ptr(X* p =0): ptr_(p), cnt_(0)	{ if ( p ) cnt_ = new unsigned(1); }
	count_ptr(const count_ptr& r)	{ inc_ref(r); }

	// �f�X�g���N�^
	~count_ptr()	{ dec_ref(); }

	// �I�y���[�^�I�[�o�[���[�h
	count_ptr& operator=(const count_ptr& r)	{ if (this != &r) { dec_ref(); inc_ref(r); } return *this; }
	X& operator*() const	{return *ptr_; }
	X* operator->() const	{return ptr_; }

	// ����
	X* get() const	{return ptr_; }
	bool unique() const	{return cnt_ ? *cnt_ == 1 : true;}
};

template <class X>
inline void count_ptr<X>::dec_ref()
{
	if ( cnt_ ) {
		if (--*cnt_ == 0) {
			delete cnt_;
			delete ptr_;
		}
		cnt_ = 0;
		ptr_ = 0;
	}
}

template <class X>
inline void count_ptr<X>::inc_ref(const count_ptr<X>& r)
{
	ptr_ = r.ptr_;
	cnt_ = r.cnt_;
	if ( cnt_ ) ++*cnt_;
}

#endif // #ifndef count_ptr
