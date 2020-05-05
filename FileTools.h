#ifndef FILETOOLS
#define FILETOOLS
#include <string>
#include <fstream>
#include<streambuf>
#include<sstream>

class FileTool
{
public:
	static std::string GetShortFilename(const std::string& str)
	{
		std::string shortName = str;
#ifdef _WIN32
		size_t index = short_name.rfind('\\');
#else
		size_t index = shortName.rfind('/');
#endif
		if (index == std::string::npos)
		{
			index = 0;
		}
		else
		{
			++index;
		}
		shortName = shortName.substr(index);
		return shortName;
	}

	static  std::string GetFileContent(const std::string& filePath)
	{
		std::ifstream ifile(filePath);
		if (!ifile.is_open())
		{
			return "";
		}
		return std::string((std::istreambuf_iterator<char>(ifile)),
			std::istreambuf_iterator<char>());
	}
};
#endif // !FILETOOLS
