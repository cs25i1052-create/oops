#include "DateTime.h"
#include <iostream>

using namespace std;

DateTime::DateTime(int m, int d, int y, int hr, int min)
{
    setDateTime(m, d, y, hr, min);
}

DateTime::~DateTime()
{
}

bool DateTime::isLeapYear(int y) const
{
    if (y % 400 == 0)
    {
        return true;
    }

    else if (y % 4 == 0 && y % 100 != 0)
    {
        return true;
    }

    else
    {
        return false;
    }
}

int DateTime::checkday(int Day) const
{
    static const int days[13] = {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

    if (Day > 0 && Day <= days[month])
    {
        return Day;
    }

    if (month == 2 && Day == 29 && isLeapYear(year))
    {
        return Day;
    }

    cout << "Invalid Day Value" << endl;
    return 1;
}

DateTime &DateTime::setDateTime(int m, int d, int y, int hr, int min)
{
    setDate(m, d, y);
    setTime(hr, min);
    return *this;
}

DateTime &DateTime::setDate(int m, int d, int y)
{
    if (m > 0 && m <= 12)
    {
        month = m;
    }

    else
    {
        month = 1;
    }

    if (y >= 1970 && y <= 2100)
    {
        year = y;
    }

    else
    {
        year = 2026;
    }

    day = checkday(d);
    return *this;
}

DateTime &DateTime::setTime(int hr, int min)
{
    if (hr >= 0 && hr < 24)
    {
        hour = hr;
    }

    else
    {
        hour = 12;
    }

    if (min >= 0 && min < 60)
    {
        minute = min;
    }

    else
    {
        minute = 0;
    }

    return *this;
}

void DateTime::display() const
{
    int h;

    if (hour == 0 || hour == 12)
    {
        h = 12;
    }
    else
    {
        h = hour % 12;
    }

    if (day < 10)
    {
        cout << "0";
    }
    cout << day << "/";

    if (month < 10)
    {
        cout << "0";
    }
    cout << month << "/" << year << " ";

    if (h < 10)
    {
        cout << "0";
    }
    cout << h << ":";

    if (minute < 10)
    {
        cout << "0";
    }
    cout << minute;

    if (hour < 12)
    {
        cout << " AM";
    }

    else
    {
        cout << " PM";
    }
}

bool DateTime::operator==(const DateTime &temp) const
{
    if (year == temp.year && month == temp.month && day == temp.day && hour == temp.hour && minute == temp.minute)
    {
        return true;
    }
    else
    {
        return false;
    }
}

bool DateTime::operator!=(const DateTime &temp) const
{
    if (*this == temp)
    {
        return false;
    }
    else
    {
        return true;
    }
}

bool DateTime::operator<(const DateTime &temp) const
{
    if (year < temp.year)
    {
        return true;
    }
    else if (year > temp.year)
    {
        return false;
    }

    if (month < temp.month)
    {
        return true;
    }
    else if (month > temp.month)
    {
        return false;
    }

    if (day < temp.day)
    {
        return true;
    }
    else if (day > temp.day)
    {
        return false;
    }

    if (hour < temp.hour)
    {
        return true;
    }
    else if (hour > temp.hour)
    {
        return false;
    }

    if (minute < temp.minute)
    {
        return true;
    }
    else
    {
        return false;
    }
}

bool DateTime::operator>(const DateTime &temp) const
{
    if (*this == temp)
    {
        return false;
    }

    if (*this < temp)
    {
        return false;
    }

    return true;
}

ostream &operator<<(ostream &out, const DateTime &dt)
{
    if (dt.day < 10)
    {
        out << "0";
    }
    out << dt.day << "/";

    if (dt.month < 10)
    {
        out << "0";
    }
    out << dt.month << "/" << dt.year << " ";

    if (dt.hour < 10)
    {
        out << "0";
    }
    out << dt.hour << ":";

    if (dt.minute < 10)
    {
        out << "0";
    }
    out << dt.minute;

    return out;
}

istream &operator>>(istream &in, DateTime &dt)
{
    int m, d, y, hr, min;

    cout << "Enter Month Day Year Hour Minute (e.g., 10 23 2026 14 30): ";
    in >> m >> d >> y >> hr >> min;
    dt.setDateTime(m, d, y, hr, min);

    return in;
}
