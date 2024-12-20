// Работа Опришко Станислава 5 группа 2 курс

#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <Windows.h>
int main(int argc, char* argv[]) {
	setlocale(LC_ALL, "ru");
	std::string commonSenderFilePath;
	std::string receiverFilePath = std::string(argv[0]);

	int posOfSlash = receiverFilePath.find_last_of('\\');

	commonSenderFilePath = receiverFilePath.substr(0, posOfSlash) + "\\Sender.exe";

	std::string binaryFileName;
	int notesCount;

	std::cout << "Введите имя бинарного файла\n";
	std::cin >> binaryFileName;
	std::cin.ignore(INT_MAX, '\n');
	std::cout << "Введите кол-во записей в файле\n";
	std::cin >> notesCount;

	std::fstream messageBuffer;
	messageBuffer.open(binaryFileName, std::ios::binary | std::ios::out | std::ios::in | std::ios::trunc);

	if (!messageBuffer.is_open()) {
		std::cerr << "File wasn't opened\n";
		return -1;
	}

	HANDLE hMutex;
	std::wstring mutexName(L"MainMutex");
	hMutex = CreateMutex(NULL, FALSE, mutexName.c_str());

	HANDLE hSemaphore;
	std::wstring semaphoreName(L"MainSemaphore");
	hSemaphore = CreateSemaphore(NULL, 0, notesCount, semaphoreName.c_str());

	if (hMutex == NULL || hSemaphore == NULL) {
		std::cerr << "Мьютекс или семафора не созданы";
		return -1;
	}
	commonSenderFilePath += " ";
	commonSenderFilePath += binaryFileName;
	commonSenderFilePath += " ";
	commonSenderFilePath += std::string(mutexName.begin(), mutexName.end());
	commonSenderFilePath += " ";
	commonSenderFilePath += std::string(semaphoreName.begin(), semaphoreName.end());

	int senderCount;
	std::cout << "Введите кол-во процессов Sender\n";
	std::cin >> senderCount;

	std::vector<HANDLE> eventVector(senderCount);
	std::vector<STARTUPINFO> siSender(senderCount);
	std::vector<PROCESS_INFORMATION> piSender(senderCount);
	std::vector<HANDLE> processHandles(senderCount);

	for (int i = 0; i < senderCount; i++)
	{
		std::wstring eventName = L"ReadyToWork" + std::to_wstring(i);
		std::wstring senderFilePath = std::wstring(commonSenderFilePath.begin(), commonSenderFilePath.end()) + L" "
			+ eventName + L" " + std::to_wstring(i);

		ZeroMemory(&siSender.at(i), sizeof(STARTUPINFO));
		if (!CreateProcess(NULL,
			const_cast<wchar_t*>(senderFilePath.c_str()),
			NULL,
			NULL,
			FALSE,
			CREATE_NEW_CONSOLE,
			NULL,
			NULL,
			&siSender.at(i),
			&piSender.at(i)
		)) {
			std::cerr << "Process wasn't created\n";
			for (int j = 0; j < i; j++) {
				CloseHandle(piSender.at(j).hProcess);
				CloseHandle(piSender.at(j).hThread);
			}
			return GetLastError();
		}
		else {
			processHandles.at(i) = piSender.at(i).hProcess;
			eventVector.at(i) = CreateEvent(NULL, TRUE, FALSE, eventName.c_str());
		}
	}
	WaitForMultipleObjects(senderCount, eventVector.data(), TRUE, INFINITE);

	bool noteCAnBeReaded = true;
	while (noteCAnBeReaded) {

		int section;
		std::cout << "Работает Receiver " << "\nВведите 1, чтобы считать запись из файл, введите 0, чтобы завершить работу процесса\n";
		std::cin >> section;
		while (section != 1 && section != 0) {
			std::cout << "Число должно быть либо 0 либо 1, введите снова\n";
			std::cin >> section;
		}


		switch (section) {
		case 1: {
			if (WaitForSingleObject(hMutex, INFINITE) == WAIT_OBJECT_0) {
				if (WaitForSingleObject(hSemaphore, 0) == WAIT_OBJECT_0) {
					char* size = new char[2];
					messageBuffer.read(size, 2);
					int sizeOfMessage = atoi(size);
					sizeOfMessage -= 10;
					char* message = new char[sizeOfMessage];
					messageBuffer.read(message, sizeOfMessage);
					message[sizeOfMessage] = '\0';
					std::cout << "Сообщение:\n" << std::string(message) << "\n";
					ReleaseMutex(hMutex);
				}
				else {
					std::cout << "Файл пуст\n";
					ReleaseMutex(hMutex);
				}
			}
		}break;
		case 0: {
			noteCAnBeReaded = false;
			for (int i = 0; i < senderCount; i++) {
				TerminateProcess(piSender.at(i).hProcess, 0);
				CloseHandle(piSender.at(i).hProcess);
				CloseHandle(piSender.at(i).hThread);
			}
			CloseHandle(hSemaphore);
			CloseHandle(hMutex);
			for (auto& el : eventVector) {
				CloseHandle(el);
			}
			messageBuffer.close();
			return 0;

		}

		}

	}

	WaitForMultipleObjects(senderCount, processHandles.data(), TRUE, INFINITE);
	Sleep(1000);
	for (int i = 0; i < senderCount; i++) {
		CloseHandle(piSender.at(i).hProcess);
		CloseHandle(piSender.at(i).hThread);
	}
	CloseHandle(hSemaphore);
	CloseHandle(hMutex);
	for (auto& el : eventVector) {
		CloseHandle(el);
	}
	messageBuffer.close();
	return 0;
}