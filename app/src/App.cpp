/**
 * @file app/App.cpp
 * @author Tomas Polasek
 * @brief The application class and main function.
 */

#include "App.h"

namespace app
{
    App::App()
    {
        std::cout << "Hello from app!" << std::endl;
    }
}

int main(int argc, char* argv[])
{
    std::cout << "Hello from main!" << std::endl;

    prof::PrintCrawler pc;
    PROF_DUMP(pc);
}

