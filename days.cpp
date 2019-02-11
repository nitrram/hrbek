#include <string>

using namespace std;


extern int get_day_of_week(const string &hypothesis)
{
	if(hypothesis.find("Pondělí") != string::npos)
	{
		return 0;
	}
	else if(hypothesis.find("Úterý") != string::npos)
	{
		return 1;
	}
	else if(hypothesis.find("Středa") != string::npos)
	{
		return 2;
	}
	else if(hypothesis.find("Čtvrtek") != string::npos)
	{
		return 3;
	}
	else if(hypothesis.find("Pátek") != string::npos)
	{
		return 4;
	}
	
	return  -1;
}

extern std::string smooth_text(const std::string &hypothesis)
{
	return hypothesis;
}
