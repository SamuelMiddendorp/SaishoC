#pragma once


struct Link;
struct Point
{
	float oldX;
	float oldY;
	float x;
	float y;
	bool isAnchor;
	void AddLink(Link* link);
	void RemoveLinks();
private:
	Link** links = new Link*[5];
	int linkIndex;
};