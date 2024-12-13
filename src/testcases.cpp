#include <iostream>
#include <sstream>
#include <cstring>
#include <unistd.h>
#include "functions.h"
#include <ctime>

#define RESET "\033[0m"
#define RED "\033[31m"
#define GREEN "\033[32m"
#define YELLOW "\033[33m"
#define BLUE "\033[34m"

using namespace std;

string repeat_string(const string &str, int times)
{
    /*
        - Parameters:
            - str: string to be repeated
            - times: number of times to repeat the string

        - Returns:
            - A string that is the concatenation of the input string repeated 'times' times.

        - Description:
            - This function takes a string and an integer as input and returns
            a new string that is the concatenation of the input string repeated 'times' times.
    */

    string result;
    for (int i = 0; i < times; i++)
    {
        result += str;
    }
    return result;
}

void print_separator()
{
    /*
        - Parameters:
            - None

        - Returns:
            - None

        - Description:
            - This function prints a separator line to the console.
    */

    cout << YELLOW << "\n================================================\n"
         << RESET;
}

void run_test(const char *input_text, const char *test_description, int test_num)
{
    /*
        - Parameters:
            - input_text: the input text to be used for the test
            - test_description: a description of the test
            - test_num: the test number

        - Returns:
            - None

        - Description:
            - This function runs a test case with the given input text and description.
            - It redirects the standard input to the input text and then calls the run_main function.
            - It measures the execution time of the run_main function and prints it to the console.
    */

    print_separator();
    cout << BLUE << "Test #" << test_num << ": " << test_description << RESET << endl;
    cout << GREEN << "Input: \"" << input_text << "\"" << RESET << endl;

    clock_t start = clock();
    istringstream input_stream(input_text);
    cin.rdbuf(input_stream.rdbuf());
    run_main(input_text);

    clock_t end = clock();
    double time_taken = double(end - start) / CLOCKS_PER_SEC;
    cout << "\nExecution time: " << time_taken << " seconds" << endl;
}

int main()
{
    int test_counter = 1;

    string large_input = repeat_string("apple ", 100) +
                         repeat_string("DELL ", 50) +
                         repeat_string("HP ", 100) +
                         repeat_string("Lenovo ", 250);

    // Test Cases
    const char *test_cases[] = {
        "apple dell hp lenovo apple dell hp lenovo",

        "apple apple apple dell dell hp lenovo",

        "apple apple apple apple apple apple apple apple apple",

        "apple, dell. hp: lenovo, apple. dell",

        "apple    dell   hp    lenovo   apple   dell",

        large_input.c_str(),

        "apple dell apple dell apple dell",

        "apple dell hp lenovo",

        "apple! dell@ hp# lenovo$ apple% dell^",

        "APPLE Dell HP lenovo Apple DELL hp LENOVO"};
    for (const char *test : test_cases)
    {
        string category = "Test Case " + to_string(test_counter);
        run_test(test, category.c_str(), test_counter++);
    }

    print_separator();
    cout << GREEN << "\nTest Completed: 10 tests executed." << RESET << endl;
    print_separator();

    return 0;
}