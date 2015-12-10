#include <iostream>
#include <windows.h>
#include <process.h>
#include <vector>
#include <chrono>

CRITICAL_SECTION cs;
const UINT threadNum = 10;
std::vector<HANDLE> hStartThread;
std::vector<HANDLE> hFinishThread;

//�X���b�h�̏������e
UINT _stdcall work(void * index)
{
	auto threadIndex = reinterpret_cast<int>(index);

	while (1)
	{

		WaitForSingleObject(hStartThread[threadIndex], INFINITE);

		EnterCriticalSection(&cs);

		std::cout << threadIndex;

		LeaveCriticalSection(&cs);

		SetEvent(hFinishThread[threadIndex]);
	}

	return 0;
}

//���Ԃ��X�V���Čo�ߕb����Ԃ�
long long UpdateTime(std::chrono::time_point<std::chrono::system_clock>* nowTime, std::chrono::time_point<std::chrono::system_clock>* oldTime)
{
	*nowTime = std::chrono::system_clock::now();
	return std::chrono::duration_cast<std::chrono::seconds>(*nowTime - *oldTime).count();
}

int main()
{
	InitializeCriticalSection(&cs);
	
	std::vector<HANDLE> hThread;

	hThread.resize(threadNum);
	hStartThread.resize(threadNum);
	hFinishThread.resize(threadNum);

	int i = 1;//�����X���b�h���m�F�p�̕ϐ�
	for (int i = 0; i < threadNum; ++i)
	{
		hThread[i] = reinterpret_cast<HANDLE>(_beginthreadex
			(
				nullptr,
				0,
				work,
				reinterpret_cast<void*>(i),
				0,
				nullptr
			));
		if (!hThread[i])
		{
			std::cout << "failed at creating thread";
			return -1;
		}

		hStartThread[i] = CreateEvent(nullptr, false, false, nullptr);
		hFinishThread[i] = CreateEvent(nullptr, false, false, nullptr);
	}
	auto oldTime = std::chrono::system_clock::now();
	auto nowTime = oldTime;

	auto sec = UpdateTime(&nowTime,&oldTime);

	while (sec < 13)
	{
		sec = UpdateTime(&nowTime, &oldTime);
		if (sec > 3)
		{
			for (auto& handle : hStartThread)
			{
				SetEvent(handle);//�C�x���g�n���h�����V�O�i����Ԃɂ���(work�֐�����WaitForSingleObject�ł̑ҋ@��Ԃ�����)
			}
			WaitForMultipleObjects(threadNum, hFinishThread.data(), true, INFINITE);
		}
	}

	for (int i = 0; i < threadNum;++i)
	{
		CloseHandle(hStartThread[i]);
		CloseHandle(hFinishThread[i]);
		CloseHandle(hThread[i]);
	}
	return 0;
}