
#ifndef DYNAMICARRAY_CPP
#define DYNAMICARRAY_CPP

#include "DynamicArray.h"

#include <iostream>
using namespace std;

template <typename T>
int DynamicArray<T>::arrayCount = 0;

template <typename T>
DynamicArray<T>::DynamicArray(int size)
{
    if (size > 0)
    {
        capacity = size;
    }
    else
    {
        capacity = 4;
    }
    count = 0;
    aptr = new T[capacity];
    arrayCount = arrayCount + 1;
}

template <typename T>
DynamicArray<T>::DynamicArray(const DynamicArray<T> &temp)
{
    capacity = temp.capacity;
    count = temp.count;
    aptr = new T[capacity];

    for (int i = 0; i < count; i++)
    {
        aptr[i] = temp.aptr[i];
    }

    arrayCount = arrayCount + 1;
}

template <typename T>
DynamicArray<T>::~DynamicArray()
{
    delete[] aptr;
    arrayCount = arrayCount - 1;
}

template <typename T>
const DynamicArray<T> &DynamicArray<T>::operator=(const DynamicArray<T> &temp)
{
    if (this != &temp)
    {
        delete[] aptr;

        capacity = temp.capacity;
        count = temp.count;
        aptr = new T[capacity];

        for (int i = 0; i < count; i++)
        {
            aptr[i] = temp.aptr[i];
        }
    }
    return *this;
}

template <typename T>
bool DynamicArray<T>::operator==(const DynamicArray<T> &temp) const
{
    if (count != temp.count)
    {
        return false;
    }

    for (int i = 0; i < count; i++)
    {
        if (aptr[i] != temp.aptr[i])
        {
            return false;
        }
    }

    return true;
}

template <typename T>
bool DynamicArray<T>::operator!=(const DynamicArray<T> &temp) const
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

template <typename T>
T &DynamicArray<T>::operator[](int index)
{
    if (index < 0 || index >= count)
    {
        cout << "Error: Index out of range" << endl;
        return aptr[0];
    }
    return aptr[index];
}

template <typename T>
const T &DynamicArray<T>::operator[](int index) const
{
    if (index < 0 || index >= count)
    {
        cout << "Error: Index out of range" << endl;
        return aptr[0];
    }
    return aptr[index];
}

template <typename T>
void DynamicArray<T>::resize(int newsize)
{
    T *temp = new T[newsize];

    for (int i = 0; i < count; i++)
    {
        temp[i] = aptr[i];
    }

    delete[] aptr;
    aptr = temp;
    capacity = newsize;
}

template <typename T>
void DynamicArray<T>::pushBack(const T &item)
{
    if (count == capacity)
    {
        resize(capacity * 2);
    }

    aptr[count] = item;
    count = count + 1;
}

template <typename T>
bool DynamicArray<T>::removeAt(int index)
{
    if (index < 0 || index >= count)
    {
        return false;
    }

    for (int i = index; i < count - 1; i++)
    {
        aptr[i] = aptr[i + 1];
    }

    count = count - 1;
    return true;
}

template <typename T>
void DynamicArray<T>::swap(int i, int j)
{
    if (i >= 0 && i < count)
    {
        if (j >= 0 && j < count)
        {
            T temp = aptr[i];
            aptr[i] = aptr[j];
            aptr[j] = temp;
        }
    }
}

template <typename T>
int DynamicArray<T>::getSize() const
{
    return count;
}

template <typename T>
int DynamicArray<T>::getArrayCount()
{
    return arrayCount;
}

#endif