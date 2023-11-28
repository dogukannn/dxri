#include <iostream>
#include <sstream>

#define LOG_LEVEL_INFO 0
#define LOG_LEVEL_WARNING 1 
#define LOG_LEVEL_ERROR 2

#define PRINTED_LOG_LEVEL LOG_LEVEL_INFO

class ll
{
public:
	std::wstringstream log;
	uint32_t loglevel;
	ll(uint32_t log_level)
	{
		loglevel = log_level;
		switch(log_level)
		{
		case LOG_LEVEL_INFO:
			log << "[DXRI_INFO] : ";
			break;
		case LOG_LEVEL_WARNING:
			log << "[DXRI_WARNING] : ";
			break;
		case LOG_LEVEL_ERROR:
			log << "[DXRI_ERROR] : ";
			break;
		default:
			log << "[DXRI_INFO] : ";
			break;
		}
	}
	~ll()
	{
		if(loglevel >= PRINTED_LOG_LEVEL)
			std::wcout << log.str() << std::endl;
	}
};
inline ll& operator<<(ll& log, std::wstring msg)
{
	log.log << msg;
	return log;
}
inline ll& operator<<(ll& log, std::string msg)
{
	log.log << msg.c_str();
	return log;
}

#define le() ll(LOG_LEVEL_ERROR)
#define lw() ll(LOG_LEVEL_WARNING)
#define li() ll(LOG_LEVEL_INFO)

