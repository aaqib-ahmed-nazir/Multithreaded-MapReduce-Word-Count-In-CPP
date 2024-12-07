#include <iostream>
#include "functions.h"

using namespace std;

int main()
{
    cout << "Enter the text: ";
    char input_text[MAX_INPUT_LENGTH];

    if (!cin.getline(input_text, MAX_INPUT_LENGTH))
    {
        cout << "Error reading input or input too large\n";
        return 1;
    }

    return run_main(input_text);
}