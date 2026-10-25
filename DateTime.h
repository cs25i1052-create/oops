#ifndef DATETIME_H
#define DATETIME_H

#include <iostream>
using namespace std;

class DateTime
{

    friend ostream &operator<<(ostream &, const DateTime &);
    friend istream &operator>>(istream &, DateTime &);

private:
    int month;
    int day;
    int year;
    int hour;
    int minute;
    int checkday(int) const;
    bool isLeapYear(int) const;

public:
    DateTime(int = 1, int = 1, int = 2026, int = 12, int = 0);

    ~DateTime();

    DateTime &setDateTime(int, int, int, int, int);
    DateTime &setDate(int, int, int);
    DateTime &setTime(int, int);

    void display() const;

    bool operator==(const DateTime &) const;
    bool operator!=(const DateTime &) const;
    bool operator>(const DateTime &) const;
    bool operator<(const DateTime &) const;
};

#endif