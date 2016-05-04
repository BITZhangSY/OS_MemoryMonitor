#include"Public.h"

#define DIV 1024
#define WIDTH 7

void WalkVM();
void ShowInfo();

int main()
{
	cout << "---------------------------------------------" << endl;
	cout << "Windows内存动态监视系统" << endl;
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
	SYSTEM_INFO si;						//系统信息
	ZeroMemory(&si, sizeof(si));
	GetSystemInfo(&si);

	MEMORYSTATUSEX statex;				//内存信息
	statex.dwLength = sizeof(statex);
	GlobalMemoryStatusEx(&statex);

	PERFORMACE_INFORMATION pi;			//系统性能信息
	pi.cb = sizeof(pi);
	GetPerformanceInfo(&pi, sizeof(pi));

	cout << "---------------------------------------------" << endl;
	cout << "物理内存使用率： " << statex.dwMemoryLoad << "%" << endl;
	cout << "分页大小为： " << si.dwPageSize / DIV << "KB" << endl;
	cout << "用户可用物理内存总容量： " << statex.ullTotalPhys / DIV / DIV << "MB" << endl;
	cout << "物理内存空闲量： " << statex.ullAvailPhys / DIV / DIV << "KB" << endl;
	cout << "系统当前物理内存总容量： " << pi.PhysicalTotal << "page" << endl;
	cout << "系统当前物理内存空闲量： " << pi.PhysicalAvailable << "page" << endl;
	cout << "进程可用私有地址空间大小： " << ((size_t)si.lpMaximumApplicationAddress -
										  (size_t)si.lpMinimumApplicationAddress) / DIV / DIV / DIV << "GB" << endl;
	cout << endl;

	HANDLE Snapshot;
	Snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	PROCESSENTRY32 processListStr;
	processListStr.dwSize = sizeof(processListStr);

	BOOL return_value = Process32First(Snapshot, &processListStr);

	while (return_value)
	{
		cout << "进程ID：	" << processListStr.th32ProcessID << endl;
		cout << "进程名称：	" << processListStr.szExeFile << endl;

		HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, processListStr.th32ProcessID);
		PROCESS_MEMORY_COUNTERS pmc;
		ZeroMemory(&pmc, sizeof(pmc));

		if (GetProcessMemoryInfo(hProcess, &pmc, sizeof(pmc)) > 0)
		{
			cout << "工作集大小：	" << pmc.WorkingSetSize / DIV << "KB" << endl;
		}

		cout << "---------------------------------------------" << endl;

		return_value = Process32Next(Snapshot, &processListStr);
	}

	cout << "---------------------------------------------" << endl;
}

void WalkVM()
{

	cout << "进程虚拟地址空间和工作集信息查询" << endl;
	cout << "请输入要查询的进程ID： " << endl;
	size_t pID;
	cin >> pID;

	SYSTEM_INFO si;						//系统信息
	ZeroMemory(&si, sizeof(si));
	GetSystemInfo(&si);


	HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pID);

	MEMORY_BASIC_INFORMATION mbi;			//进程虚拟内存空间的基本信息结构
	ZeroMemory(&mbi, sizeof(mbi));

	LPCVOID pBlock = si.lpMinimumApplicationAddress;			//每个进程可用地址空间的最小内存地址
	while (pBlock < si.lpMaximumApplicationAddress)
	{
		if (VirtualQueryEx(hProcess, pBlock, &mbi, sizeof(mbi)) == sizeof(mbi))
		{
			LPCVOID pEnd = (PBYTE)pBlock + mbi.RegionSize;			//pBlock为该虚存区的头指针，pEnd为尾指针
			TCHAR szSIZE[MAX_PATH];

			StrFormatByteSize(mbi.RegionSize, szSIZE, MAX_PATH);	//将区域大小转为字符串形式
			
			//显示块地址和块大小
			cout << hex << setw(8) << pBlock << "-" << hex << setw(8) << pEnd << "(" << szSIZE << ")";

			//显示块的状态
			switch (mbi.State)
			{
				case MEM_COMMIT:
					cout << "Committed，";	//提交
					break;
				case MEM_FREE:
					cout << "Free，";			//空闲
					break;
				case MEM_RESERVE:
					cout << "Reserved，";		//预留
					break;
				default:
					break;
			}

			//显示块类型
			switch (mbi.Type)
			{
				case MEM_IMAGE:				//EXE, DLL
					cout << "Image";		
					break;
				case MEM_MAPPED:			//内存映射文件
					cout << "Mapped";
					break;
				case MEM_PRIVATE:			//私有区域
					cout << "Private";
					break;
				default:
					break;
			}

			//检测该区域是否有文件

			TCHAR szFILENAME[MAX_PATH];
			if (GetModuleFileName((HMODULE)pBlock, szFILENAME, MAX_PATH) > 0) 
			{
				//去除完整路径名的路径部分
				PathStripPath(szFILENAME);
				cout << "	  Module： " << szFILENAME;
			}

			cout << endl << endl;
			pBlock = pEnd;
		}
	}
}