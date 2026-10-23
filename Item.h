#ifndef ITEM_H
#define ITEM_H

#include <iostream>
#include <cstring>
#include "DateTime.h"
#include "DynamicArray.h"

using namespace std;

class Item
{
    friend ostream &operator<<(ostream &out, const Item &it);
    friend istream &operator>>(istream &in, Item &it);
    friend class Explorer;

private:
    char name[50];
    int isFolder;
    char content[200];
    int size;
    int version;

    DateTime createdDate;
    Item *parent;
    DynamicArray<Item *> children;

    static int totalItems;

public:
    Item(const char *n = "root", int folder = 1, const char *txt = "");
    Item(const Item &other);
    ~Item();

    Item &setName(const char *newName);
    Item &setContent(const char *newText);

    const char *getName() const;
    int getIsFolder() const;
    int getTotalSize() const;
    int getChildCount() const;
    DateTime getDate() const;
    Item *getParent() const;

    int addChild(Item *child);
    Item *findChild(const char *childName) const;
    Item *removeChild(const char *childName);
    int deleteChild(const char *childName);

    const Item &operator=(const Item &other);
    bool operator==(const Item &other) const;

    Item *operator[](int index) const;
    Item &operator+=(Item *child);
    Item operator+(const Item &other) const;

    Item &operator++();
    Item operator++(int);

    static int getTotalItems();
};

#endif
