#include "point.h";
#include "link.h";
#include <iostream>

void Point::AddLink(Link* l)
{
	links[linkIndex++] = l;
}
void Point::RemoveLinks()
{
	for (auto i = 0; i < linkIndex; i++)
	{
		links[i]->p1 = nullptr;
		links[i]->p2 = nullptr;
		
	}
}
