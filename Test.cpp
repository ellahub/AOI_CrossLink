//author: atao 1628025718@qq.com
#include <iostream>
#include <cstdlib>
#include <list>
#include <memory>

#include "AOI.h"
#include "SceneRole.h"

using namespace CL;

int main()
{
	int max = 100;
	AOI aoi(max, max);
	for (int i = 0; i < max; i++)
	{
		for (int j = 0; j < max; j++)
		{
			SceneRole* role = new SceneRole(i*100+j, i, j);
			aoi.enter(role);
		}
		
	}
	aoi.stat();

	// some body
	SceneRole* role = new SceneRole(99999, 0, 0);
	aoi.enter(role);
	printf("start move 100 step\n");
	for (auto i = 0; i < 10; i++)
	{
		//printf("move to %d %d\n", i, i);
		aoi.move(role, role->x() + 1, role->y() + 1);
		role->set_x(role->x() + 1);
		role->set_y(role->y() + 1);
	}
	printf("move 100 step done.\n");
	
	for (int i = 0; i < max; i++)
	{
		for (int j = 0; j < max; j++)
		{
			aoi.leave(i * 100 + j);
		}
		//aoi.leave(i);
	}
	aoi.leave(99999);
	aoi.stat();
	getchar();
	return 0;
}