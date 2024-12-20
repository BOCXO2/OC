#include <iostream>
#include <fstream>
#include <sstream>
#include <Windows.h>

int main(int argc, char* argv[]) {
	setlocale(LC_ALL, "ru");

	HANDLE hMutex = OpenMutexA(SYNCHRONIZE, FALSE, argv[2]);
	HANDLE hSemaphore = OpenSemaphoreA(SEMAPHORE_ALL_ACCESS, FALSE, argv[3]);
	HANDLE hEvent = OpenEventA(EVENT_MODIFY_STATE, FALSE, argv[4]);

	if (hMutex == NULL || hSemaphore == NULL || hEvent == NULL) {
		std::cerr << "Семафора, мьютекс или событие не открыты\n";
		Sleep(2500);
		return GetLastError();
	}

	SetEvent(hEvent);

	std::ofstream messageBuffer;
	messageBuffer.open(argv[1], std::ios::binary | std::ios::out);

	if (!messageBuffer.is_open()) {
		std::cerr << "File " << argv[1] << " wasn't opened\n";
		Sleep(2500);
		return -1;
	}

	bool noteCanBeAdded = true;

	while (noteCanBeAdded) {

		int section;
		std::cout << "Работает Sender номер " << std::string(argv[5]) << "\nВведите 1, чтобы сделать запись в файл, введите 0, чтобы завершить работу процесса\n";
		std::cin >> section;
		while (section != 1 && section != 0) {
			std::cout << "Число должно быть либо 0 либо 1, введите снова\n";
			std::cin >> section;
		}

		switch (section) {
		case 1: {
			if (WaitForSingleObject(hMutex, INFINITE) == WAIT_OBJECT_0) {
				if (ReleaseSemaphore(hSemaphore, 1, NULL)) {
					messageBuffer.close();
					messageBuffer.open(argv[1], std::ios::binary | std::ios::out | std::ios::app);
					if (!messageBuffer.is_open()) {
						std::cerr << "File " << argv[1] << " wasn't opened\n";
						Sleep(2500);
						return -1;
					}

					std::string message;
					std::cout << "Введите сообщение\n";

					while (message.empty())
						std::getline(std::cin, message);

					if (message.size() > 20)
						message = message.substr(0, 20);

					int size = message.size();
					messageBuffer.write(std::to_string(size + 10).c_str(), 2);
					messageBuffer.write(message.c_str(), size);
					messageBuffer.flush();
					;						Sleep(50);
					ReleaseMutex(hMutex);
				}
				else {
					std::cout << "\nФайл заполнен\n";
					ReleaseMutex(hMutex);
				}
			}
		}break;
		case 0: {

			noteCanBeAdded = false;
			CloseHandle(hEvent);
			CloseHandle(hSemaphore);
			CloseHandle(hMutex);

			messageBuffer.close();

			return 0;

		}
		}


	}
	CloseHandle(hEvent);
	CloseHandle(hSemaphore);
	CloseHandle(hMutex);
	messageBuffer.close();
	return 0;
}