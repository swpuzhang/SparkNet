#ifndef LOG_H
#define LOG_H
#include <string>
#include <memory>
#include <fstream>
#include <sstream>
#include <string.h>
#include <errno.h>
#include "spdlog/spdlog.h"
#include "spdlog/sinks/rotating_file_sink.h"
#include "spdlog/async.h"
#include "spdlog/common.h"
#include "Json.h"
#include "FileDir.h"
#include "date/date.h"
#include "Singleton.h"
#include "FileTools.h"

using namespace date;
using namespace std::chrono;

class CFunLogHelper;

struct LogConfig
{
	size_t _level;
	size_t _maxFileSize;
	size_t _maxFileNum;
	std::string _logDir;
	LogConfig(void) :
		_level(spdlog::level::trace),
		_maxFileSize(1024 * 1024 * 10),
		_maxFileNum(100),
		_logDir("./log")
	{
	}
};

constexpr int PATH_MAX_LEN = 260;
std::string GetPorcName(void)
{
	char link[PATH_MAX_LEN] = { 0 };
	char buf[PATH_MAX_LEN] = { 0 };
	sprintf(link, "/proc/%d/exe", ::getpid());
	if (readlink(link, buf, PATH_MAX_LEN - 1) < 0)
	{
		printf("readlink error=%d\n", errno);
		return "default";
	}

	std::string strPathName = buf;
	auto posBegin = strPathName.find_last_of('/');
	strPathName = strPathName.substr(posBegin + 1);
	return strPathName;
}

class CLog
{
public:
	
	int init(const std::string& configPath = "")
	{
		_procName = GetPorcName();
		_configFilePath = configPath;
		if (_configFilePath.empty())
		{
			_configFilePath = "./config.json";
		}
		loadLogConfig();
		return 0;
	}
	void safeClose() { spdlog::shutdown(); }
	spdlog::logger& getLogger() { return  *_logger; }
	size_t GetCurrentLogLevel() const { return _logConfig._level; }
private:
	int loadLogConfig(void)
	{
		std::ifstream ifile(_configFilePath);
		if (!ifile.is_open())
		{
			setLogAtt();
			return 0;
		}
		Json jv;
		ifile >> jv;

		if (!jv.is_object() || !jv["log"].is_object() || !jv["log"]["default"].is_object())
		{
			setLogAtt();
			return 0;
		}

		_logConfig._level = std::stoul(jv["log"]["default"]["level"].get<std::string>());
		_logConfig._maxFileNum = std::stoul(jv["log"]["default"]["filenum"].get<std::string>());
		_logConfig._maxFileSize = std::stoul(jv["log"]["default"]["filesize"].get<std::string>());
		_logConfig._logDir = jv["log"]["default"]["dir"].get<std::string>();

		if (!jv["log"][_procName].is_object())
		{
			setLogAtt();
			return 0;
		}


		_logConfig._level = std::stoul(jv["log"][_procName]["level"].get<std::string>());
		_logConfig._maxFileNum = std::stoul(jv["log"][_procName]["filenum"].get<std::string>());
		_logConfig._maxFileSize = std::stoul(jv["log"][_procName]["filesize"].get<std::string>());
		_logConfig._logDir = jv["log"][_procName]["dir"].get<std::string>();

		std::string cfgTag = _procName;
		
		cfgTag += "_" + std::to_string(::getpid());
		if (!jv["log"][cfgTag].is_object())
		{
			setLogAtt();
			return 0;
		}

		_logConfig._level = std::stoul(jv["log"][cfgTag]["level"].get<std::string>());
		_logConfig._maxFileNum = std::stoul(jv["log"][cfgTag]["filenum"].get<std::string>());
		_logConfig._maxFileSize = std::stoul(jv["log"][cfgTag]["filesize"].get<std::string>());
		_logConfig._logDir = jv["log"][cfgTag]["dir"].get<std::string>();


		setLogAtt();
		return 0;
	}
	bool setLogAtt()
	{
		std::string strWorkDir = GetPwd();
		std::string strLogDir = strWorkDir + "/log";
		_logConfig._logDir = strLogDir + "/" + _procName;
		CreateDirectory(_logConfig._logDir);
		spdlog::init_thread_pool(100000, 1);
		spdlog::flush_every(std::chrono::seconds(5));
		std::string strFileName;
		std::stringstream strstr;
		strstr << floor<days>(system_clock::now());
		strFileName = _logConfig._logDir + "/" + _procName + "_" + strstr.str() +
			"_p" + std::to_string(::getpid()) + ".log";
		_logger = spdlog::rotating_logger_mt<spdlog::async_factory>("logger", strFileName,
			_logConfig._maxFileSize * 1024 * 1024, _logConfig._maxFileNum);
		_logger->set_level(spdlog::level::level_enum(_logConfig._level));
		return true;
	}
	//std::string get_short_filename(const std::string& str);
	std::string _configFilePath;
	std::string _procName;
	LogConfig _logConfig;
	std::shared_ptr<spdlog::logger> _logger;
};

static auto logger = MagicSingleton<CLog>::GetInstance();

class CFunLogHelper
{
public:
	CFunLogHelper(int line, const std::string& fileName, const std::string& funcName)
		: _line(line), _filename(fileName), _funName(funcName), _enterTime(steady_clock::now())
	{

		logger->getLogger().log(spdlog::level::trace,
			("->" + std::string("[{}:{}:{}]")).c_str(), FileTool::GetShortFilename(_filename), funcName, line);
	}
	~CFunLogHelper()
	{
		long long useMilli = (steady_clock::now() - _enterTime).count();
		logger->getLogger().log(spdlog::level::trace,
			("<-[" + std::to_string(useMilli) + "ms]" + std::string("[{}:{}:{}]")).c_str(),
			FileTool::GetShortFilename(_filename), _funName, _line);

	}
private:
	int _line;
	std::string _filename;
	std::string _funName;
	steady_clock::time_point _enterTime;
};

class LogStream
{
public:
	LogStream(size_t level) : _level(level) {}
	template<typename T>
	LogStream& operator << (const T& value)
	{
		if (logger->GetCurrentLogLevel() <= _level)
		{
			_strstrm << value;
		}
		return *this;
	}
	std::string GetString()
	{
		return _strstrm.str();
	}
private:
	std::ostringstream _strstrm;
	size_t _level;
};

class LogOpearator
{
public:
	LogOpearator(int level, int linenum, const std::string& filename, const std::string functionname, bool logSysErr = false, bool isabort = false) 
		: _logstrm(level),
		_level(level), 
		_linenum(linenum),
		_filename(filename), 
		_functionname(functionname),
		_isAbort(isabort){
		if (logSysErr)
		{
			_logSysErr = " sys error:";
			_logSysErr += strerror(errno);
		}
	}
	LogStream& stream() { return _logstrm; }
	~LogOpearator()
	{
		if (logger->GetCurrentLogLevel() <= _level)
		{
			std::string str = _logstrm.GetString();
			if (!_logSysErr.empty())
			{
				str += _logSysErr;
			}
			
			if (!str.empty())
			{
				logger->getLogger().log(spdlog::level::level_enum(_level),
					(str + std::string("[{}:{}:{}]")).c_str(), FileTool::GetShortFilename(_filename), _functionname, _linenum);
			}
			if (_isAbort)
			{
				logger->getLogger().flush();
				abort();
			}
		}
	}
private:
	LogStream _logstrm;
	size_t _level;
	int _linenum;
	std::string _filename;
	std::string _functionname;
	std::string _logSysErr;
	bool _isAbort;
};

#define TRACE_LOG \
	LogOpearator(spdlog::level::trace, __LINE__, __FILE__, __FUNCTION__).stream()

#define DEBUG_LOG \
	LogOpearator(spdlog::level::debug, __LINE__, __FILE__, __FUNCTION__).stream()

#define INFO_LOG \
	LogOpearator(spdlog::level::info, __LINE__, __FILE__, __FUNCTION__).stream()

#define WARN_LOG \
	LogOpearator(spdlog::level::warn, __LINE__, __FILE__, __FUNCTION__).stream()

#define ERROR_LOG \
	LogOpearator(spdlog::level::err, __LINE__, __FILE__, __FUNCTION__).stream()

#define CRITICAL_LOG  \
	LogOpearator(spdlog::level::critical, __LINE__, __FILE__, __FUNCTION__, false, true).stream()

#define SYS_ERROR_LOG \
	LogOpearator(spdlog::level::critical, __LINE__, __FILE__, __FUNCTION__, true).stream()

#define SYS_CRITICAL_LOG \
	LogOpearator(spdlog::level::critical, __LINE__, __FILE__, __FUNCTION__, true, true).stream()
	

#endif // !LOG_H
