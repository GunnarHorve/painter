// PointMapTest2.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <iostream>
#include "PointMap.cpp"


int _tmain(int argc, _TCHAR* argv[])
{
	std::cout << "hi!" << std::endl;
	
	PointMap p = PointMap("black");

	p.addPoint(1, 2);
	p.addPoint(3, 4);
	p.addPoint(5, 6);

	std::cout << "there are " << p.getNumberOfPoints() << "points (should be 2)" << std::endl;
	std::cout << "the x position of the first point: " << p.getPointAt(0).getX() << std::endl;
	std::cout << "the y position of the second point: " << p.getPointAt(1).getY() << std::endl;

	std::cout << "press enter to continue" << std::endl;
	std::cin.get();
	p.setPos(0);
	std::cout << "currentPos (should be 0): " << p.getCurrentPos() << std::endl;
	std::cout << "the y position of the second point: " << p.getNext().getY() << std::endl;
	std::cout << "currentPos (should be 1): " << p.getCurrentPos() << std::endl;
	std::cout << "the x position of the third point: " << p.getNext().getX() << std::endl;
	std::cout << "currentPos (should be 2): " << p.getCurrentPos() << std::endl;

	std::cout << "press enter to continue" << std::endl;
	std::cin.get();

	std::cout << "testing setting position" << std::endl;
	std::cout << "true : " << p.setPos(-1) << std::endl;
	std::cout << "false : " << p.setPos(-2) << std::endl;
	std::cout << "true : " << p.setPos(1) << std::endl;
	std::cout << "false : " << p.setPos(30) << std::endl;
	
	std::cout << "press enter to escape" << std::endl;
	std::cin.get();

	return 0;
	
	
}

