#ifndef DYNAMICARRAY_H
#define DYNAMICARRAY_H

#include <iostream>
using namespace std;

template <typename T>

class DynamicArray
{
private:
    T *aptr;
    int capacity;
    int count;
    static int arrayCount;
    void resize(int);

public:
    DynamicArray(int = 4);
    DynamicArray(const DynamicArray<T> &);
    ~DynamicArray();

    const DynamicArray<T> &operator=(const DynamicArray<T> &);
    bool operator==(const DynamicArray<T> &) const;
    bool operator!=(const DynamicArray<T> &) const;
    T &operator[](int);
    const T &operator[](int) const;

    void pushBack(const T &);
    bool removeAt(int);
    void swap(int, int);

    int getSize() const;
    static int getArrayCount();
};
#include "DynamicArray.cpp"
#endif