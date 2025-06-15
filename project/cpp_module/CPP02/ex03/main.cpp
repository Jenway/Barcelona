#include "Point.hpp"

bool bsp(Point const& a, Point const& b , Point const& c, Point const& point);

static void	printData(const Point& A, const Point& B, const Point& C, const Point& P)
{
	std::cout << "Triangle vertices are "
	<< "A(" << A.getX() << ", " << A.getY() << "), "
	<< "B(" << B.getX() << ", " << B.getY() << "), "
	<< "C(" << C.getX() << ", " << C.getY() << ")"
	<< "\n\n"
	<< "Point P is "
	<< "P(" << P.getX() << ", " << P.getY() << ")\n" << std::endl;
}

int main( void ) 
{
	Point		A(0.f, 0.f);
	Point		B(10.f, 30.f);
	Point		C(20.f, 0.f);
	Point		P(10.f, 15.f);
	std::string	answer;

	answer = bsp(A, B, C, P) ? " " : " not ";
	printData(A, B, C, P);
	std::cout << "Point P is" << answer << "in the triangle ABC!" << std::endl;

	return 0;
}