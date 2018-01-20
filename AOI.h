#pragma once
//author: atao 1628025718@qq.com
#include <cstdint>
#include <list>
#include <set>
#include <map>
#include <iostream>
#include <algorithm>
#include <iterator>
using namespace std;
namespace CL
{
class SceneRole;
class AOI
{
	typedef uint16_t Point;
	typedef uint64_t Id;
	typedef set<Id> Ids;
	typedef map<Point, Ids> Point2Id;
public:
	AOI(uint16_t width, uint16_t height);
	~AOI();

	//API
	void enter(SceneRole*);
	void leave(uint64_t);
	void move(SceneRole*, Point next_x, Point next_y);
	//debug
	void stat()
	{
		cout << "roles size:" << roles_.size() << endl;
		cout << "listx size:" << listx_.size() << endl;
		cout << "listy size:" << listy_.size() << endl;
	}
private:
	//加入链表
	void add_list(list <Point2Id>&, Id id, Point p, map <Point, list<Point2Id>::iterator>&);
	//离开链表
	void leave_list(list <Point2Id>&, Point, Id, map <Point, list<Point2Id>::iterator>&);
	//初始化链表节点元素
	void init_point2id(Ids&, Point2Id&, Id id, Point point);
	//视野玩家
	void CL::AOI::get_view(Point x, Point y, SceneRole* role, Ids& view_ids);
private:
	Point width_;
	Point height_;
	//x,y坐标对应的玩家id集合
	list <Point2Id> listx_;
	list <Point2Id> listy_;
	//所有玩家
	map <Id, SceneRole*> roles_;
	//辅助方法，因为listx_, listy_随机访问效率很低，这里提供x到对应链表迭代器的map映射。
	map <Point, list<Point2Id>::iterator> x2listx_;
	map <Point, list<Point2Id>::iterator> y2listy_;

};//class AOI
}//namespace CL

