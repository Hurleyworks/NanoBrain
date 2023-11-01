/*
MIT License

Copyright (c) 2023 Steve Hurley

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

extern Jahley::App* Jahley::CreateApplication();


// This C++ code defines a function main which is the entry point of the program.The function takes two parameters
// : argc, an integer representing the number of command - line arguments passed to the program, and argv, 
// an array of strings representing those arguments.

// The code block within the function starts by creating a ScopedStopWatch object named sw 
// with the _FN_ argument.ScopedStopWatch is a custom class that measures the time elapsed 
// between its creation and destruction._FN_ is a preprocessor macro that expands to the name 
// of the current function, used here to print the name of the function being timed.

// The next line creates a pointer to an object of the Jahley::App class, using the 
// Jahley::CreateApplication() function.The purpose of this object is not clear from 
// the provided code snippet, but it is likely the main application object for the program.
// The if statement checks whether the Jahley::App object is a windowed application,
// and if so, it calls its run() method to start the application loop.

// After the if statement, the Jahley::App object is deleted to release its memory.

// The final lines of the function use the std::cout and std::cin objects to print a message
// to the console and wait for the user to press Enter before exiting the program.
// Overall, this code appears to be a simple boilerplate for a C++ program that creates
// an application object, starts the main loop for a windowed application, and prints a message
// to the console before exiting.

int main (int argc, char** argv)
{
    {
        ScopedStopWatch sw (_FN_);

        Jahley::App* app = Jahley::CreateApplication();
        if (app->isWindowApp())
        {
            app->run();
        }

        delete app;
    }

    std::cout << "Press ENTER to continue...";
    std::cin.ignore (std::numeric_limits<std::streamsize>::max(), '\n');
}