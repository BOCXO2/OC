#include <iostream>
#include <fstream>
#include <string>
#include "..\employee.h"
using namespace std;

int main(int argc, char* argv[])
{
    char* filename = argv[1];
    int length = atoi(argv[2]);

    ofstream fout(filename, ios::binary);
    if (!fout)
    {
        cout << "файл не открылся";
        return 1;
    }
    for (int i = 0; i < length; i++)
    {
        employee emp;
        cout << "Enter number: ";
        cin >> emp.num;
        cout << "Enter name: ";
        cin >> emp.name;
        cout << "Enter hours: ";
        cin >> emp.hours;

        fout.write(reinterpret_cast<char*>(&emp), sizeof(emp));
    }
    fout.close();
    cout << "successfully!" << endl;
}

