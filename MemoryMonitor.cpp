#include"Public.h"

#define DIV 1024
#define WIDTH 7

void WalkVM();
void ShowInfo();

int main()
{
	cout << "---------------------------------------------" << endl;
	cout << "Windows�ڴ涯̬����ϵͳ" << endl;
	cout << "---------------------------------------------" << endl;
	cout << "input 1 to show the system info" << endl;
	cout << "input 2 to show specific process info" << endl;
	cout << "input c to clear the console" << endl;
	cout << "input q to quit the program" << endl;

	char key;
	cin >> key;
	while(key != 'q')
	{
		switch (key)
		{
		case '1':
			ShowInfo();
			break;
		case '2':
			WalkVM();
			break;
		case 'c':
			system("cls");
			break;
		default:
			cout << "input error! try to input again" << endl;
		}
		cin >> key;
	}

	system("pause");
	return 0;
}

void ShowInfo()
{
	SYSTEM_INFO si;						//ϵͳ��Ϣ
	ZeroMemory(&si, sizeof(si));
	GetSystemInfo(&si);

	MEMORYSTATUSEX statex;				//�ڴ���Ϣ
	statex.dwLength = sizeof(statex);
	GlobalMemoryStatusEx(&statex);

	PERFORMACE_INFORMATION pi;			//ϵͳ������Ϣ
	pi.cb = sizeof(pi);
	GetPerformanceInfo(&pi, sizeof(pi));

	cout << "---------------------------------------------" << endl;
	cout << "�����ڴ�ʹ���ʣ� " << statex.dwMemoryLoad << "%" << endl;
	cout << "��ҳ��СΪ�� " << si.dwPageSize / DIV << "KB" << endl;
	cout << "�û����������ڴ��������� " << statex.ullTotalPhys / DIV / DIV << "MB" << endl;
	cout << "�����ڴ�������� " << statex.ullAvailPhys / DIV / DIV << "KB" << endl;
	cout << "ϵͳ��ǰ�����ڴ��������� " << pi.PhysicalTotal << "page" << endl;
	cout << "ϵͳ��ǰ�����ڴ�������� " << pi.PhysicalAvailable << "page" << endl;
	cout << "���̿���˽�е�ַ�ռ��С�� " << ((size_t)si.lpMaximumApplicationAddress -
										  (size_t)si.lpMinimumApplicationAddress) / DIV / DIV / DIV << "GB" << endl;
	cout << endl;

	HANDLE Snapshot;
	Snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	PROCESSENTRY32 processListStr;
	processListStr.dwSize = sizeof(processListStr);

	BOOL return_value = Process32First(Snapshot, &processListStr);

	while (return_value)
	{
		cout << "����ID��	" << processListStr.th32ProcessID << endl;
		cout << "�������ƣ�	" << processListStr.szExeFile << endl;

		HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, processListStr.th32ProcessID);
		PROCESS_MEMORY_COUNTERS pmc;
		ZeroMemory(&pmc, sizeof(pmc));

		if (GetProcessMemoryInfo(hProcess, &pmc, sizeof(pmc)) > 0)
		{
			cout << "��������С��	" << pmc.WorkingSetSize / DIV << "KB" << endl;
		}

		cout << "---------------------------------------------" << endl;

		return_value = Process32Next(Snapshot, &processListStr);
	}

	cout << "---------------------------------------------" << endl;
}

void WalkVM()
{

	cout << "���������ַ�ռ�͹�������Ϣ��ѯ" << endl;
	cout << "������Ҫ��ѯ�Ľ���ID�� " << endl;
	size_t pID;
	cin >> pID;

	SYSTEM_INFO si;						//ϵͳ��Ϣ
	ZeroMemory(&si, sizeof(si));
	GetSystemInfo(&si);


	HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pID);

	MEMORY_BASIC_INFORMATION mbi;			//���������ڴ�ռ�Ļ�����Ϣ�ṹ
	ZeroMemory(&mbi, sizeof(mbi));

	LPCVOID pBlock = si.lpMinimumApplicationAddress;			//ÿ�����̿��õ�ַ�ռ����С�ڴ��ַ
	while (pBlock < si.lpMaximumApplicationAddress)
	{
		if (VirtualQueryEx(hProcess, pBlock, &mbi, sizeof(mbi)) == sizeof(mbi))
		{
			LPCVOID pEnd = (PBYTE)pBlock + mbi.RegionSize;			//pBlockΪ���������ͷָ�룬pEndΪβָ��
			TCHAR szSIZE[MAX_PATH];

			StrFormatByteSize(mbi.RegionSize, szSIZE, MAX_PATH);	//�������СתΪ�ַ�����ʽ
			
			//��ʾ���ַ�Ϳ��С
			cout << hex << setw(8) << pBlock << "-" << hex << setw(8) << pEnd << "(" << szSIZE << ")";

			//��ʾ���״̬
			switch (mbi.State)
			{
				case MEM_COMMIT:
					cout << "Committed��";	//�ύ
					break;
				case MEM_FREE:
					cout << "Free��";			//����
					break;
				case MEM_RESERVE:
					cout << "Reserved��";		//Ԥ��
					break;
				default:
					break;
			}

			//��ʾ������
			switch (mbi.Type)
			{
				case MEM_IMAGE:				//EXE, DLL
					cout << "Image";		
					break;
				case MEM_MAPPED:			//�ڴ�ӳ���ļ�
					cout << "Mapped";
					break;
				case MEM_PRIVATE:			//˽������
					cout << "Private";
					break;
				default:
					break;
			}

			//���������Ƿ����ļ�

			TCHAR szFILENAME[MAX_PATH];
			if (GetModuleFileName((HMODULE)pBlock, szFILENAME, MAX_PATH) > 0) 
			{
				//ȥ������·������·������
				PathStripPath(szFILENAME);
				cout << "	  Module�� " << szFILENAME;
			}

			cout << endl << endl;
			pBlock = pEnd;
		}
	}
}