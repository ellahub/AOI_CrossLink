#include "AOI.h"
#include "SceneRole.h"
//author: atao 1628025718@qq.com

CL::AOI::AOI(uint16_t width, uint16_t height) :
	width_(width), height_(height)
{
}


CL::AOI::~AOI()
{
}


void CL::AOI::enter(SceneRole* role)
{
	roles_.insert({ role->id(), role });
	//加入listx_
	add_list(listx_, role->id(), role->x(), x2listx_);
	//加入listy_
	add_list(listy_, role->id(), role->y(), y2listy_);
}

void CL::AOI::leave(uint64_t id)
{
	auto it = roles_.find(id);
	if (it != roles_.end())
	{
		Point x = it->second->x();
		Point y = it->second->y();
		//离开对应的链表
		leave_list(listx_, it->second->x(), it->second->id(), x2listx_);
		leave_list(listy_, it->second->y(), it->second->id(), y2listy_);
		//update roles_
		roles_.erase(id);
	}
	
}

//加入链表
void CL::AOI::add_list(list <Point2Id>& points, Id id, Point point, map <Point, list<Point2Id>::iterator>& maps)
{
	// 记录是否将数据插入到了合适的位置
	bool is_updated = false;
	//遍历链表，找到合适的地方进行插入
	for (auto it = points.begin(); it != points.end(); ++it)
	{
		Point the_point = it->begin()->first;
		if (the_point == point) // 插入到当前位置
		{
			it->begin()->second.insert(id);
			is_updated = true;
			break;
		}
		else if (the_point > point) //插入到当前位置的前一个位置
		{
			Ids ids;
			Point2Id data;
			init_point2id(ids, data, id, point);
			data.insert({ point, ids });
			points.insert(it, data);
			is_updated = true;
			//建立索引
			maps.insert({ point, --it });
			break;
		}
		else //继续向后查找
		{
			continue;
		}
	}
	//如果比现有所有点的值都大，那么插入到最后
	if (!is_updated)
	{
		Ids ids;
		Point2Id data;
		init_point2id(ids, data, id, point);
		auto it = points.end();
		points.insert(it, data);
		maps.insert({ point, --it });
		is_updated = true;
	}
}

//离开
void CL::AOI::leave_list(list <Point2Id>& points, Point point, Id id, map <Point, list<Point2Id>::iterator>& maps)
{
	//简单的通过遍历从指定的位置上删除
	//TODO 最好通过位置直接映射到对应链表的节点，然后直接删除
	/*
	for (auto it = points.begin(); it != points.end(); ++it)
	{
		Point the_point = it->begin()->first;
		if (the_point == point)
		{
			it->begin()->second.erase(id);
			if (it->begin()->second.size() == 0) //如果这个坐标已经没有数据了，那就删掉。
			{
				points.erase(it);
			}
			break;
		}
	}
	*/
	// 直接使用坐标到链表的maps，快速更新链表
	auto it = maps.find(point);
	if ( it != maps.end())
	{
		//直接找到对应的list位置
		auto list_it = it->second;
		Point the_point = list_it->begin()->first;
		if (the_point == point)
		{
			list_it->begin()->second.erase(id);
			if (list_it->begin()->second.size() == 0) //如果这个坐标已经没有数据了，那就删掉。
			{
				points.erase(list_it);
				maps.erase(point);//同步删除映射关系
			}
		}
	}
}

// 移动
// 算法库提供的集合操作：
// 交集： set_intersection
// 并集:  set_union
// 差集：set_difference (A差B 和 B差A的意义不一样)
//获取离开和进入视野的玩家
//假设移动过后的视野是A，移动之前的视野是B，
//那么A-B就是新进入自己视野的对象
//B-A就是离开视野的对象
//A+B就是需要通知移动消息的集合
void CL::AOI::move(SceneRole* role, Point next_x, Point next_y)
{
	//更新位置
	Point old_x = role->x();
	Point old_y = role->y();
	Id id = role->id();

	if (old_x != next_x)
	{
		//更新listx_
		leave_list(listx_, old_x, id, x2listx_);
		add_list(listx_, role->id(), next_x, x2listx_);
	}
	if (old_y != next_y)
	{
		leave_list(listy_, old_y, id, y2listy_);
		add_list(listy_, role->id(), next_y, y2listy_);
	}
	
	// 通知离开、进入、移动消息
	Ids old_views, new_views;
	get_view(old_x, old_y, role, old_views);
	get_view(next_x, next_y, role, new_views);
	Ids leave_ids, enter_ids, move_ids;
	//通知离开的集合
	std::set_difference(old_views.begin(), old_views.end(),
		new_views.begin(), new_views.end(),
		std::inserter(leave_ids, leave_ids.begin()));
	//通知进入的集合
	std::set_difference(new_views.begin(), new_views.end(),
		old_views.begin(), old_views.end(),
		std::inserter(enter_ids, enter_ids.begin()));
	//通知移动的集合
	std::set_union(new_views.begin(), new_views.end(),
		old_views.begin(), old_views.end(),
		std::inserter(move_ids, move_ids.begin()));
	//通知对应的消息
	//...
	//....

}

void CL::AOI::init_point2id(Ids& ids, Point2Id& data, Id id, Point point)
{
	ids.insert(id);
	data.insert({ point, ids });
}

void CL::AOI::get_view(Point x, Point y, SceneRole* role, Ids& view_ids)
{
	Ids xids, yids;
	//x轴上范围内的玩家
	//Point x = role->x();
	auto xit = x2listx_.find(x);
	if (xit != x2listx_.end())
	{
		auto listx_node = xit->second; //链表节点,节点是一个map，而且只有一个key，key就是x坐标，对应一个玩家id集合
		auto the_x_node = listx_node;
		//并集
		std::set_union(xids.begin(), xids.end(),
			listx_node->begin()->second.begin(), listx_node->begin()->second.end(),
			std::inserter(xids, xids.begin())); //将这部分玩家id集合加入ids中
		while (++listx_node != listx_.end()) //向后找合适节点中的玩家id
		{
			Point the_x = listx_node->begin()->first;
			if (std::abs(x - the_x) <= role->view_x()) //在视野范围内
			{
				std::set_union(xids.begin(), xids.end(),
					listx_node->begin()->second.begin(), listx_node->begin()->second.end(),
					std::inserter(xids, xids.begin()));
			}
			else
			{
				break;
			}
		}
		listx_node = the_x_node;
		// 需要处理listx_node已经是链表头部的情况
		if (listx_node != listx_.begin())
		{
			while (--listx_node != listx_.begin()) //向前找合适节点中的玩家id
			{
				Point the_x = listx_node->begin()->first;
				if (std::abs(x - the_x) <= role->view_x()) //在视野范围内
				{
					std::set_union(xids.begin(), xids.end(),
						listx_node->begin()->second.begin(), listx_node->begin()->second.end(),
						std::inserter(xids, xids.begin()));
				}
				else
				{
					break;
				}
			}
		}
	}

	//y轴上范围内的玩家
	//Point y = role->y();
	auto yit = y2listy_.find(y);
	if (yit != y2listy_.end())
	{
		auto listy_node = yit->second; //链表节点,节点是一个map，而且只有一个key，key就是y坐标，对应一个玩家id集合
		auto the_y_node = listy_node;
		std::set_union(yids.begin(), yids.end(),
			listy_node->begin()->second.begin(), listy_node->begin()->second.end(),
			std::inserter(yids, yids.begin())); //将这部分玩家id集合加入ids中
		while (++listy_node != listy_.end()) //向后找合适节点中的玩家id
		{
			Point the_y = listy_node->begin()->first;
			if (std::abs(y - the_y) <= role->view_y()) //在视野范围内
			{
				std::set_union(yids.begin(), yids.end(),
					listy_node->begin()->second.begin(), listy_node->begin()->second.end(),
					std::inserter(yids, yids.begin()));
			}
			else
			{
				break;
			}
		}
		listy_node = the_y_node;
		if (listy_node != listy_.begin())
		{
			while (--listy_node != listy_.begin()) //向前找合适节点中的玩家id
			{
				Point the_y = listy_node->begin()->first;
				if (std::abs(y - the_y) <= role->view_y()) //在视野范围内
				{
					std::set_union(yids.begin(), yids.end(),
						listy_node->begin()->second.begin(), listy_node->begin()->second.end(),
						std::inserter(yids, yids.begin()));
				}
				else
				{
					break;
				}
			}
		}
	}

	//求x和y轴视野的交集
	std::set_intersection(xids.begin(), xids.end(), 
		yids.begin(), yids.end(), 
		std::inserter(view_ids, view_ids.begin()));
}