#pragma once

// ini 
#include <windows.h> 

std::string mv_ini_path(mapvalue* p)
{
	std::vector<mapvalue*> list = p->parentlist();
	list.push_back(p);
	std::string s;

	if( !list.empty() ) {
		bool bEndKakko=false;
		for(int i=1; i<list.size(); i++) {
			s += list[i]->get_name();
			if ( bEndKakko ) {
				s+="]";
				bEndKakko = false;
			}
			if(list[i]->get_type() == mapvalue::OBJECT) {
				s+=".";
			}
			else if (list[i]->get_type() == mapvalue::ARRAY && i != list.size() - 1 ) {
				s+="[";
				bEndKakko = true;
			}
		}
	}

	return s;
}

// ini ファイルのリードライト
template <class T>
void mv_write_ini(T* t, char* filename, char* section)
{
	mapvalue m(section);
	to_mapvalue(m, /*is_obj_to_map*/true, t);
	mv_write_ini_path(&m, filename, section, "");
}

void mv_write_ini_path(mapvalue* p, char* filename, char* section, char* path)
{
	mapvalue& m = *p;
	if(m.get_type() == mapvalue::NONE) {
		assert(0);
	}
	else if(m.get_type() == mapvalue::VALUE) {
		std::string s = mv_ini_path(&m);
		::WritePrivateProfileString(section, s.c_str(), boost::any_cast<const char*>(m.value), filename);
	}
	else {
		if( m.get_type() == mapvalue::ARRAY ) {
			std::string s = mv_ini_path(&m);
			s += ".size";
			char buf[512];
			::WritePrivateProfileString(section, s.c_str(), itoa(m.size(),buf,512), filename);
		}
		for(int i=0; i<m.size(); i++) {
			mv_write_ini_path(&m[i], filename, section, path );
		}
	}
}

// 読み込み
template <class T>
void mv_read_ini(T* t, char* filename, char* section)
{
	mapvalue m(section);
	to_mapvalue(m, /*is_obj_to_map*/true, t);
	mv_read_ini(&m, filename, section);
	to_mapvalue(m, /*is_obj_to_map*/false, t);
}

void mv_read_ini(mapvalue* p, char* filename, char* section)
{
	mapvalue& m = *p;
	if(m.get_type() == mapvalue::NONE) {
		assert(0);
	}
	else if(m.get_type() == mapvalue::VALUE) {
		std::string s = mv_ini_path(&m);

		char buf[512];
		::GetPrivateProfileString(section, s.c_str(), "", buf, 512, filename);
		m.value = buf;
	}
	else if(m.get_type() == mapvalue::ARRAY) {
		// 配列データ読み込み
		std::string s = mv_ini_path(&m);
		std::string size = s + ".size";
		int nSize = ::GetPrivateProfileInt(section, size.c_str(), 0, filename);

		for(int i=0; i<nSize; i++) {
			char buf[512];
			std::string key = s;
			key += "[";
			key += itoa(i, buf, 512);
			key += "]";

			::GetPrivateProfileString(section, key.c_str(), "", buf, 512, filename);
			if(buf[0] != '\0') {
				mapvalue* p = new mapvalue(key.c_str());
				p->value = buf;
				m.push_back( p );
			}
		}

	}
	else if(m.get_type() == mapvalue::OBJECT) {
		// オブジェクトの読み込み
		for(int i=0; i<m.size(); i++) {
			mv_read_ini(&m[i], filename, section );
		}
	}
}
