#include <iostream>

#include "app.hpp"

// define our application
App app;

int main()
{
	try {
		if (app.init())
			return app.run();
	}
	catch (std::exception const& e) {
		std::cerr << "App failed : " << e.what() << std::endl;
		exit(EXIT_FAILURE);
	}
	return EXIT_SUCCESS;
}
