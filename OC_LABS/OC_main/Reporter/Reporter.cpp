#include <iostream>
#include <fstream>
#include <string>
#include "..\employee.h"
using namespace std;

int main(int argc, char* argv[])
{
    char* filename = argv[1];
    char* reportfile = argv[2];
    int cost = atoi(argv[3]);
    ifstream b_in(filename, ios::binary);
    if (!b_in)
    {
        cout << "файл не открылся";
        return 1;
    }
    ofstream f_out(reportfile);
    f_out << "File report '" << filename << "'\n";
    int res(0);
    employee emp;
    while (b_in.read(reinterpret_cast<char*>(&emp), sizeof(emp)))
    {
        res = cost * emp.hours;
        f_out << emp.num << " " << emp.name << " " << emp.hours << " " << res << "\n";
    }
    b_in.close();
    f_out.close();
    cout << "successfully!" << endl;
}

