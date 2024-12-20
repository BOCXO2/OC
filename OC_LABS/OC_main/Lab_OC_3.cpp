// Опришко Станислав, 5 группа
// ОС Лаб3
/*
* Основная логика:
1. Пользователь вводит размер массива и количество потоков.
2. Запускаются потоки, каждый из которых помечает случайные элементы массива своим ID.
3. Программа выводит массив и предлагает пользователю остановить поток.
4. Поток очищает свои метки в массиве и завершает работу.
5. Когда все потоки завершены, программа освобождает ресурсы и завершает выполнение.
*/

#include <Windows.h>
#include <iostream>

using namespace std;

int arraySize = 0;
int* arrayData = nullptr; // Указатель на массив, с которым работают потоки

CRITICAL_SECTION criticalSection; // Синхронизация доступа к разделяемым ресурсам

// Массивы, содержащие объекты событий и дескрипторы потоков.
HANDLE* threadHandles;
HANDLE* startEvents;
HANDLE* stopEvents;
HANDLE* exitEvents;

DWORD WINAPI markerFunction(LPVOID param) {
    int threadId = (int)param;

    WaitForSingleObject(startEvents[threadId], INFINITE);

    int markedElements = 0;
    srand(threadId + 1);

    while (true) {
        EnterCriticalSection(&criticalSection);
        int randomIndex = rand() % arraySize;
        if (arrayData[randomIndex] == 0) {
            Sleep(5);
            arrayData[randomIndex] = threadId + 1;
            markedElements++;
            Sleep(5);
            LeaveCriticalSection(&criticalSection);
        }
        else {
            cout << "ID потока: " << threadId + 1 << "\n";
            cout << "Количество отмеченных элементов: " << markedElements << "\n";
            cout << "Индекс элемента, который нельзя отметить: " << randomIndex << "\n";

            LeaveCriticalSection(&criticalSection);

            SetEvent(stopEvents[threadId]);
            ResetEvent(startEvents[threadId]);

            HANDLE events[] = { startEvents[threadId], exitEvents[threadId] };

            if (WaitForMultipleObjects(2, events, FALSE, INFINITE) == WAIT_OBJECT_0 + 1) {
                EnterCriticalSection(&criticalSection);
                for (int i = 0; i < arraySize; i++) {
                    if (arrayData[i] == threadId + 1) {
                        arrayData[i] = 0;
                    }
                }
                LeaveCriticalSection(&criticalSection);
                ExitThread(0);
            }
            else {
                ResetEvent(stopEvents[threadId]);
                continue;
            }
        }
    }
}

int main() {
    setlocale(LC_ALL, "Russian");

    cout << "Введите размер массива: ";
    cin >> arraySize;
    arrayData = new int[arraySize] {};

    cout << "Введите количество потоков: ";
    int numThreads = 0;
    cin >> numThreads;

    InitializeCriticalSection(&criticalSection);
    threadHandles = new HANDLE[numThreads];
    startEvents = new HANDLE[numThreads];
    stopEvents = new HANDLE[numThreads];
    exitEvents = new HANDLE[numThreads];

    for (int i = 0; i < numThreads; i++) {
        startEvents[i] = CreateEvent(NULL, TRUE, FALSE, NULL);
        stopEvents[i] = CreateEvent(NULL, TRUE, FALSE, NULL);
        exitEvents[i] = CreateEvent(NULL, TRUE, FALSE, NULL);
        threadHandles[i] = CreateThread(NULL, 0, markerFunction, (LPVOID)i, 0, NULL);
    }

    for (int i = 0; i < numThreads; i++) {
        SetEvent(startEvents[i]);
    }

    int threadsCompleted = 0;
    bool* exitedThreads = new bool[numThreads] {};

    while (threadsCompleted < numThreads) {
        WaitForMultipleObjects(numThreads, stopEvents, TRUE, INFINITE);

        EnterCriticalSection(&criticalSection);
        cout << "Массив: ";
        for (int i = 0; i < arraySize; i++) {
            cout << arrayData[i] << " ";
        }
        cout << "\n";
        LeaveCriticalSection(&criticalSection);

        int selectedThreadId;
        cout << "Введите ID потока для остановки: ";
        cin >> selectedThreadId;
        selectedThreadId--;

        if (!exitedThreads[selectedThreadId]) {
            threadsCompleted++;
            exitedThreads[selectedThreadId] = true;

            SetEvent(exitEvents[selectedThreadId]);
            WaitForSingleObject(threadHandles[selectedThreadId], INFINITE);

            CloseHandle(threadHandles[selectedThreadId]);
            CloseHandle(exitEvents[selectedThreadId]);
            CloseHandle(startEvents[selectedThreadId]);
        }

        EnterCriticalSection(&criticalSection);
        cout << "Текущий массив: ";
        for (int i = 0; i < arraySize; i++) {
            cout << arrayData[i] << " ";
        }
        cout << "\n";
        LeaveCriticalSection(&criticalSection);

        for (int i = 0; i < numThreads; i++) {
            if (!exitedThreads[i]) {
                ResetEvent(stopEvents[i]);
                SetEvent(startEvents[i]);
            }
        }
    }

    delete[] arrayData;
    DeleteCriticalSection(&criticalSection);
    delete[] threadHandles;
    delete[] startEvents;
    delete[] stopEvents;
    delete[] exitEvents;

    return 0;
}
