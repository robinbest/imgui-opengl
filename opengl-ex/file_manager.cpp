#include <filesystem>
#include "file_manager.h"

namespace fs = std::filesystem;
std::string FileManager::s_exe_path;

FileManager::FileManager()
{
}

FileManager::~FileManager()
{
}

std::string FileManager::read(const std::string& filename)
{
    //std::cout << "Loading " << filename << std::endl;
    std::ifstream file;
    file.exceptions (std::ifstream::failbit | std::ifstream::badbit);
	std::stringstream file_stream;
	try {
		file.open(filename.c_str());
    	file_stream << file.rdbuf();
		file.close();
    }
    catch (std::ifstream::failure e) {
        std::cout << "Error reading Shader File!" << std::endl;
    }
	return file_stream.str();
}

void FileManager::init_exe_path(const std::string& argument0)
{
    s_exe_path = fs::path(argument0).parent_path().string();
}
