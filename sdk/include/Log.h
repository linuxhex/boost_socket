#ifndef _LOG_kyosho_20110902_
#define _LOG_kyosho_20110902_

#include <fstream>
#include <string>
#include <sstream>
#include <ctime>
#include <map>

using namespace std;


class cLog
{
public:

	~cLog();

	bool Open(string sFileName);
	void Close();

	bool CommonLogInit(); //打开默认的log 文件
	bool LaserLogInit();
	bool NewWithTime(string str);

	void Enable();
	void Disable();

	string GetTimeStr();

	template <typename T> void LogOut(const T& value)
	{
		if (m_bEnabled)
		{
			m_tOLogFile << value;
		}
	}

	template <typename T> void LogOutLn(const T& value)
	{
		if (m_bEnabled)
		{
			m_tOLogFile << GetTimeStr() <<" "<<value << endl;
		}
	}

	void LogOutLn()
	{
		if (m_bEnabled)
		{
			m_tOLogFile << endl;
		}
	}

	template <typename T> cLog& operator<<(const T& value)
	{
		if (m_bEnabled)
		{
			m_tOLogFile << GetTimeStr() <<" "<<value << endl;
		}
		return (*this);
	}

	cLog& operator<<(ostream& (*_Pfn)(ostream&))
	{
		if (m_bEnabled)
		{
			(*_Pfn)(m_tOLogFile);
		}
		return (*this);
	}

static cLog* GetInstance(string str_name);
static bool Close(string str_name);
private:
	cLog();
	typedef map<string,cLog*>  mlog;
static mlog name_log_map;
	string log_name_;

	template<typename T> string ValueToStr(T value)
	{
		ostringstream ost;
		ost << value;
		return ost.str();
	}
private:
	ofstream m_tOLogFile;

	bool m_bEnabled;
};

#define LOGS(str) (*cLog::GetInstance(str))

#endif //_LOG_kyosho_20110902_
