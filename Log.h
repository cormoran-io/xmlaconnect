/*
	ODBO provider for XMLA data stores
    Copyright (C) 2014-2015  ARquery LTD
	http://www.arquery.com

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

	@description
					
*/

#ifndef LOG_HPP
#define LOG_HPP

#include <iostream>
#include <sstream>
#include <iostream>
#include <fstream>
#include <time.h>

class Logger {
private:
	Logger()
	{
		mFile.open("Log_XMLA_Connect.txt");
	}

public:
	
	static Logger& instance() {
		static Logger instance;
		return instance;
	}

	void log(const char* aMsg ) {
		mFile<<currentDateTime()<<":  "<<aMsg<<std::endl;
	}

	const std::string currentDateTime() {
		time_t     now = time(0);
		struct tm  tstruct;
		char       buf[80];
		localtime_s(&tstruct,&now);

		strftime(buf, sizeof(buf), "%Y-%m-%d.%X", &tstruct);
		return buf;
	}

	~Logger()
	{
		mFile.close();
	}
private:
	std::ofstream mFile;
};

#define LOG(msg)\
{\
	std::stringstream log;\
	log << msg;\
	Logger::instance().log(log.str().c_str() );\
}


#endif