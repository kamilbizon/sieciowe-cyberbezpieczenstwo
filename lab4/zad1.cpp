#include <iostream>

int main() {
	int i;
	int* j =  new int();

	std::cout << &i << " " << i << " " << main << std::endl;

	delete j;
}