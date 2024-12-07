#include <iostream>
#include <sstream>
#include <cstring>
#include <unistd.h>
#include "functions.h"
#include <ctime>

// ANSI color codes for better output formatting
#define RESET "\033[0m"
#define RED "\033[31m"
#define GREEN "\033[32m"
#define YELLOW "\033[33m"
#define BLUE "\033[34m"

using namespace std;

// Add helper function for string repetition
string repeat_string(const string &str, int times)
{
    string result;
    for (int i = 0; i < times; i++)
    {
        result += str;
    }
    return result;
}

void print_separator()
{
    cout << YELLOW << "\n================================================\n"
         << RESET;
}

void run_test(const char *input_text, const char *test_description, int test_num)
{
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

    // Create a large input string for stress testing
    string large_input = repeat_string("apple ", 100) +
                         repeat_string("DELL ", 50) +
                         repeat_string("HP ", 100) +
                         repeat_string("Lenovo ", 250);

    // Define 10 comprehensive test cases covering all scenarios
    const char *test_cases[] = {
        // 1. Basic input test with equal word counts
        "apple dell hp lenovo apple dell hp lenovo",

        // 2. Different frequency test
        "apple apple apple dell dell hp lenovo",

        // 3. Single word repeated test
        "apple apple apple apple apple apple apple apple apple",

        // 4. Simple punctuation test
        "apple, dell. hp: lenovo, apple. dell",

        // 5. Mixed spacing test
        "apple    dell   hp    lenovo   apple   dell",

        // 6. Large input stress test (1750 groups of 4 words = 7000 words total)
        large_input.c_str(),

        // 7. Short repeating pattern
        "apple dell apple dell apple dell",

        // 8. All words once
        "apple dell hp lenovo",

        // 9. Words with special characters
        "apple! dell@ hp# lenovo$ apple% dell^",

        // 10. Words with mixed case (to test case sensitivity)
        "APPLE Dell HP lenovo Apple DELL hp LENOVO"};
    // Run all test cases
    for (const char *test : test_cases)
    {
        string category = "Test Case " + to_string(test_counter);
        run_test(test, category.c_str(), test_counter++);
    }

    print_separator();
    cout << GREEN << "\nTest Suite Completed: 10 tests executed." << RESET << endl;
    print_separator();

    return 0;
}