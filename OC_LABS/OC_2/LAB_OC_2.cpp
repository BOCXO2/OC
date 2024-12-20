#include <iostream>
#include <windows.h>
#include <vector>

using namespace std;

struct MinMaxFinder {
    vector<double>* data;
    double minimumValue;
    double maximumValue;

    MinMaxFinder(vector<double>* data) : data(data), minimumValue(0.0), maximumValue(0.0) {}
};

struct ArrayAverage {
    vector<double>* data;
    double averageValue;

    ArrayAverage(vector<double>* data) : data(data), averageValue(0.0) {}
};

DWORD WINAPI findMinMax(LPVOID lpParam) {
    MinMaxFinder* minMaxData = (MinMaxFinder*)lpParam;
    vector<double>& arr = *(minMaxData->data);

    minMaxData->minimumValue = arr[0];
    minMaxData->maximumValue = arr[0];

    for (int i = 1; i < arr.size(); ++i) {
        if (arr[i] < minMaxData->minimumValue) {
            minMaxData->minimumValue = arr[i];
        }
        Sleep(6);
        if (arr[i] > minMaxData->maximumValue) {
            minMaxData->maximumValue = arr[i];
        }
        Sleep(6);
    }
    cout << "Min value: " << minMaxData->minimumValue << endl;
    cout << "Max value: " << minMaxData->maximumValue << endl;
    return 0;
}

DWORD WINAPI calculateAverage(LPVOID lpParam) {
    ArrayAverage* avgData = (ArrayAverage*)lpParam;
    vector<double>& arr = *(avgData->data);

    double totalSum = 0;
    for (int i = 0; i < arr.size(); ++i) {
        totalSum += arr[i];
        Sleep(12);
    }
    avgData->averageValue = totalSum / arr.size();
    cout << "Average: " << avgData->averageValue << endl;
    return 0;
}

int main() {
    int arraySize;
    cout << "Enter size: ";
    cin >> arraySize;
    vector<double> arr(arraySize);
    cout << "Enter elements: ";
    for (int i = 0; i < arraySize; ++i) {
        cin >> arr[i];
    }

    MinMaxFinder minMax(&arr);
    ArrayAverage avg(&arr);

    HANDLE hMinMax, hAverage;
    DWORD minMaxID, avgID;

    hMinMax = CreateThread(NULL, 0, findMinMax, &minMax, 0, &minMaxID);
    if (hMinMax == NULL)
        return GetLastError();

    hAverage = CreateThread(NULL, 0, calculateAverage, &avg, 0, &avgID);
    if (hAverage == NULL)
        return GetLastError();

    //ждем пока поток закончит свою работу
    WaitForSingleObject(hMinMax, INFINITE);
    //закрываем дескриптор потока
    CloseHandle(hMinMax);

    WaitForSingleObject(hAverage, INFINITE);
    CloseHandle(hAverage);

    for (int i = 0; i < arraySize; ++i) {
        if (arr[i] == minMax.minimumValue || arr[i] == minMax.maximumValue) {
            arr[i] = avg.averageValue;
        }
    }

    cout << "Updated array: ";
    for (double val : arr) {
        cout << val << " ";
    }
    cout << endl;

    return 0;
}