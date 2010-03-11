#pragma once

#include <string>
#include <sstream>
#include <vector>
#include <assert.h>
#include <algorithm>

// mapvalue
class mapvalue
{
public:
	// オブジェクトタイプ
	enum type {
		NONE,
		VALUE,
		ARRAY,
		OBJECT,
	};

	// 取得関数への引数
	enum copy {
		map_to_obj,
		obj_to_map,
	};

	// タイプ
	type					get_type()						{ return m_type; }; // タイプの取得
	type					set_type(type t)				{ return m_type = t; }; // タイプの取得

	// 名前
	std::string				get_name()						{ return m_name; }// 名前の取得
	std::string				set_name(std::string name)		{ return m_name = name; }// 名前の取得

	// 親の取得
	mapvalue*				parent()			{ return m_parent; }
	std::vector<mapvalue*>	parentlist();

	// 値型アクセス
	template<class T> T		get(T* p)			{ std::stringstream s(m_value); s >> *p; return *p;}
	template<class T> void	set(T v)			{ std::stringstream s; s << v; m_value = s.str(); }

	// 配列及びオブジェクト型アクセスメソッド
	size_t					size()const						{ return m_array.size();	}
	void					push_back(mapvalue* p)			{ m_array.push_back(p);					m_array.back()->m_parent = this;	}
	mapvalue&				operator[](int n)				{ return *m_array[n];		}
	mapvalue&				operator[](std::string n)		{ return findandinsert(n);	}

	// コンストラクタ
	mapvalue(std::string name="")							{ m_type = NONE; m_name = name; m_parent = 0;	}
	// デストラクタ
	~mapvalue(){
		for(int i=0; i<m_array.size(); i++) {
			delete m_array[i];
		}
	}

private:
	// 文字列での名前検索アクセス
	mapvalue&	findandinsert(std::string name);

	// メンバ
	type m_type;
	std::string m_name;
	std::string m_value;
	std::vector<mapvalue*> m_array;
	mapvalue* m_parent;
};

// 親リストの作成
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

// 文字列での名前検索アクセス
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

// マクロ
//#define MAPVALUE_INNER_BEGIN(T)	inline void to_mapvalue(mapvalue& map, bool is_obj_to_map)			{ T* p=this;
#define MAPVALUE_BEGIN(T)		inline void to_mapvalue(mapvalue& map, bool is_obj_to_map, T* p)	{			 map.set_type(mapvalue::OBJECT);
#define		MV_VALUE(v)				::to_mapvalue(map[#v],  is_obj_to_map, &p->v);
#define		MV_PVALUE(v)			::to_mapvalue(map[#v],  is_obj_to_map, p->v);
#define		MV_ARRAY(v)				::mv_array(map[#v],  is_obj_to_map, &p->v);
#define MAPVALUE_END()			}

// MAPVALUE_INNER_BEGIN でのクラス関数をグローバル関数へ変換
template<class T>
void to_mapvalue(mapvalue& map, bool is_obj_to_map, T* p) {
	//map.set_type(mapvalue::OBJECT);
	p->to_mapvalue(map, is_obj_to_map, p);
}

// 値指定
template<class T>
void mv_value(mapvalue& s, bool is_obj_to_map, T* p)
{
	s.set_type(mapvalue::VALUE);
	//s.set_name(name);
	if(is_obj_to_map) {
		s.set(*p);
	}
	else{
		s.get(p);
	}
}

// 値指定
inline void to_mapvalue(mapvalue& map, bool is_obj_to_map, int*				p){	mv_value(map, is_obj_to_map, p); }
inline void to_mapvalue(mapvalue& map, bool is_obj_to_map, float*			p){	mv_value(map, is_obj_to_map, p); }
inline void to_mapvalue(mapvalue& map, bool is_obj_to_map, double*			p){	mv_value(map, is_obj_to_map, p); }
inline void to_mapvalue(mapvalue& map, bool is_obj_to_map, std::string*		p){	mv_value(map, is_obj_to_map, p); }

// 文字列変換
inline char* itoa(int i)
{
	static char buf[512];
	return itoa(i, buf, 512);
}
// 配列
template<class T>
void mv_array(mapvalue& s, bool is_obj_to_map, T* p)
{
	T& v = *p;

	s.set_type(mapvalue::ARRAY);
	if(is_obj_to_map){
		for(int i=0; i<v.size();i++) {
			mapvalue* p = new mapvalue(itoa(i));
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
