#pragma once

#include <string>
#include <sstream>
#include <vector>
#include <assert.h>
#include <algorithm>
#include <boost/any.hpp>
//namespace mv {
	class  value2 {
	public:
		std::string m_value;
		template<class T> operator T() { T v; std::stringstream s(m_value); s >> v; return v; }
		template<class T> value2& operator =(T v) { std::stringstream s; s << v; m_value = s.str(); return *this; }
		std::string str(){ return m_value; }
		//value& operator=(std::string s)
		//operator std::string(){ return m_value; }
	};
//}

// mapvalue
class mapvalue
{
public:
	// �I�u�W�F�N�g�^�C�v
	enum type {
		NONE,
		VALUE,
		ARRAY,
		OBJECT,
	};
	//mv::value& value() { return m_value; }
	//mv::value value;
	boost::any value;

	// �^�C�v
	type					get_type()						{ return m_type; }; // �^�C�v�̎擾
	type					set_type(type t)				{ return m_type = t; }; // �^�C�v�̎擾

	// ���O
	std::string				get_name()						{ return m_name; }// ���O�̎擾
	//std::string				set_name(std::string name)		{ return m_name = name; }// ���O�̎擾

	// �e�̎擾
	mapvalue*				parent()			{ return m_parent; }
	std::vector<mapvalue*>	parentlist();

	// �l�^�A�N�Z�X
	//template<class T> T		get(T* p)			{ std::stringstream s(m_value); s >> *p; return *p;}
	//template<class T> void	set(T v)			{ std::stringstream s; s << v; m_value = s.str(); }
	
	//template<class T> operator T()				{ T t; this->get(&t); return t; }
	//operator int() { int n; this->get(&n); return n; }

	// �z��y�уI�u�W�F�N�g�^�A�N�Z�X���\�b�h
	size_t					size()const						{ return m_array.size();	}
	void					push_back(mapvalue* p)			{ m_array.push_back(p);					m_array.back()->m_parent = this;	}
	mapvalue&				operator[](int n)				{ return *m_array[n];		}
	mapvalue&				operator[](std::string n)		{ return findandinsert(n);	}

	// �R���X�g���N�^
	mapvalue(std::string name="")							{ m_type = NONE; m_name = name; m_parent = 0;	}
	template<class T>
	mapvalue(std::string name, T& r)						{ m_name = name; ::to_mapvalue(*this, true, &r);	}

	// �f�X�g���N�^
	~mapvalue(){
		for(int i=0; i<m_array.size(); i++) {
			delete m_array[i];
		}
	}

private:
	// ������ł̖��O�����A�N�Z�X
	mapvalue&	findandinsert(std::string name);

	// �����o
	type m_type;
	std::string m_name;
	//mv::value m_value;
	std::vector<mapvalue*> m_array;
	mapvalue* m_parent;
};

// �e���X�g�̍쐬
inline std::vector<mapvalue*> mapvalue::parentlist(){
	std::vector<mapvalue*> v;
	mapvalue* p = this->parent();
	while(p != 0) {
		v.push_back(p);
		p = p->parent();
	}
	std::reverse(v.begin(), v.end());
	return v;
}

// ������ł̖��O�����A�N�Z�X
inline mapvalue&	mapvalue::findandinsert(std::string name)
{
	for(int i=0;i< size(); i++) {
		if(m_array[i]->m_name == name) {
			return *m_array[i];
		}
	}
	m_array.push_back( new mapvalue(name) );
	m_array.back()->m_parent = this;
	return *m_array.back();
}

// �}�N��
#define MAPVALUE_BEGIN(T)		inline void to_mapvalue(mapvalue& map, bool is_obj_to_map, T* p)	{			 map.set_type(mapvalue::OBJECT);
#define		MV_VALUE(v)				::to_mapvalue(map[#v],  is_obj_to_map, &p->v);
#define		MV_PVALUE(v)			::to_mapvalue(map[#v],  is_obj_to_map, p->v);
#define		MV_ARRAY(v)				::mv_array(map[#v],  is_obj_to_map, &p->v);
#define MAPVALUE_END()			}

// MAPVALUE_INNER_BEGIN �ł̃N���X�֐����O���[�o���֐��֕ϊ�
template<class T>
void to_mapvalue(mapvalue& map, bool is_obj_to_map, T* p) {
	p->to_mapvalue(map, is_obj_to_map, p);
}

// �l�w��
template<class T>
void mv_value(mapvalue& s, bool is_obj_to_map, T* p)
{
	s.set_type(mapvalue::VALUE);
	if(is_obj_to_map) {
		s.value = *p;
	}
	else{
		//T v = boost::any_cast<T>(s.value);
		*p = boost::any_cast<T>(s.value);
		//((T)*p) = static_cast<T>(s.value);;
		//*p = static_cast<T>(s.value);
	}
}

// �l�w��
inline void to_mapvalue(mapvalue& map, bool is_obj_to_map, int*				p){	mv_value(map, is_obj_to_map, p); }
inline void to_mapvalue(mapvalue& map, bool is_obj_to_map, float*			p){	mv_value(map, is_obj_to_map, p); }
inline void to_mapvalue(mapvalue& map, bool is_obj_to_map, double*			p){	mv_value(map, is_obj_to_map, p); }
inline void to_mapvalue(mapvalue& map, bool is_obj_to_map, std::string*		p){	mv_value(map, is_obj_to_map, p); }

// ������ϊ�
inline char* itoa(int i)
{
	static char buf[512];
	return itoa(i, buf, 512);
}
// �z��
template<class T>
void mv_array(mapvalue& s, bool is_obj_to_map, T* p)
{
	T& v = *p;

	s.set_type(mapvalue::ARRAY);
	if(is_obj_to_map){
		for(int i=0; i<v.size();i++) {
			// �C���f�b�N�X�𖼑O�ɐݒ�
			//char buf[512];
			//itoa(i, buf, 512);
			std::stringstream ss; ss << i;

			mapvalue* p = new mapvalue(ss.str());
			to_mapvalue(*p,is_obj_to_map,&v[i]);
			s.push_back(p);
		}
	}
	else {
		v.resize(s.size());
		for(int i=0; i<v.size();i++) {
			to_mapvalue(s[i], is_obj_to_map,&v[i]);
		}
	}
}
