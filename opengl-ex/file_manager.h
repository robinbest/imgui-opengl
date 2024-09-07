#pragma once

#include <fstream>
#include <string>
#include <sstream>
#include <iostream>

class FileManager
{
public:
	FileManager();
	~FileManager();
	static std::string read(const std::string& filename);

	//! input is the first argument in main()
	static void init_exe_path(const std::string& argument0);
	static std::string get_exe_path() { return s_exe_path; }

private:
	static std::string s_exe_path;  //! main exe path
};

