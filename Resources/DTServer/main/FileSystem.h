#pragma once
#include <iostream>
#include <string>

class FileSystem
{
public:
	bool checkFileCriteria(const std::wstring file_path);
	std::string chooseLog();
	FileSystem();


};