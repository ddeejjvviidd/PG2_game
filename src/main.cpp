#include <iostream>

#include "app.hpp"

int main()
{
    std::cout << "Entering main...\n" << std::flush;
    try
    {
        // define our application
        std::cout << "Creating App object...\n" << std::flush;
        App app;
        std::cout << "App constructed...\n" << std::flush;
        std::cout << "Calling init...\n" << std::flush;
        if (app.init())
        {
            return app.run();
        }
        else
        {
            std::cerr << "Initialization failed\n";
            return EXIT_FAILURE;
        }
    }
    catch (std::exception const &e)
    {
        std::cerr << "App failed : " << e.what() << std::endl;
        exit(EXIT_FAILURE);
    }
    return EXIT_SUCCESS;
}
