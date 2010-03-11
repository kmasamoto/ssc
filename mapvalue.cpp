// mapvalue.cpp : コンソール アプリケーション用のエントリ ポイントの定義
//

#include "stdafx.h"
#include "mapvalue.h"
#include "mv_ini.h"

struct Hoge2 {
	int z;
	int x;
	int y;

	MAPVALUE_INNER_BEGIN(Hoge2)
		MV_VALUE(z)
		MV_VALUE(x)
		MV_VALUE(y)
	MAPVALUE_END()
};

struct Hoge {
	int x;
	int y;
	int z;
	Hoge2 obj;
	Hoge2* pobj;
	std::vector<int> array_int;
	std::vector<Hoge2> array_obj;
};

MAPVALUE_BEGIN(Hoge)
	MV_VALUE(x)
	MV_VALUE(y)
	MV_VALUE(z)
	MV_VALUE(obj)
	MV_PVALUE(pobj)
	MV_ARRAY(array_int)
	MV_ARRAY(array_obj)
MAPVALUE_END()

int main(int argc, char* argv[])
{
	std::string s="1";
	std::stringstream ss(s);
	int n; ss >> n;

	Hoge h;
	h.pobj = &h.obj;
	h.x = 1;
	h.z = 2;
	h.y = 3;
	h.obj.x = 5;
	h.obj.y = 6;
	h.obj.z = 7;
	h.array_int.push_back(1);
	h.array_obj.push_back(h.obj);

	mv_write_ini(&h, "C:\\test2.ini", "h");

	Hoge h2;
	h2.pobj = &h2.obj;
	mv_read_ini(&h2, "C:\\test2.ini", "h");

	std::vector<int> v;
	v.resize(100);
	v.erase(&v[3]);
	v.erase(v.begin() + 3);

	return 0;
}
