#include <iostream>
#include <cstring>
#include <ctime>
#include "Item.h"

using namespace std;

int Item::totalItems = 0;

Item::Item(const char *n, int folder, const char *txt)
{
    if (n == NULL) n = "unnamed";
    strncpy(name, n, 49);
    name[49] = '\0';

    isFolder = folder;

    if (isFolder == 1 || txt == NULL) {
        strcpy(content, "");
        size = 0;
    } else {
        strncpy(content, txt, 199);
        content[199] = '\0';
        size = strlen(content);
    }

    time_t now = time(NULL);
    tm *t = localtime(&now);
    createdDate.setDateTime(t->tm_mon + 1, t->tm_mday, t->tm_year + 1900, t->tm_hour, t->tm_min);

    version = 1;
    parent = NULL;
    totalItems++;
}

Item::Item(const Item &other) : createdDate(other.createdDate)
{
    strcpy(name, other.name);
    isFolder = other.isFolder;
    strcpy(content, other.content);
    size = other.size;
    version = other.version;
    parent = NULL;

    for (int i = 0; i < other.children.getSize(); i++)
    {
        Item *copyChild = new Item(*(other.children[i]));
        copyChild->parent = this;
        children.pushBack(copyChild);
    }
    totalItems++;
}

Item::~Item()
{
    for (int i = 0; i < children.getSize(); i++)
    {
        delete children[i];
    }
    totalItems--;
}

Item &Item::setName(const char *newName)
{
    if (newName != NULL) {
        if (parent != NULL) {
            Item *same = parent->findChild(newName);
            if (same != NULL && same != this) {
                cout << "Error: Name already exists in folder.\n";
                return *this;
            }
        }
        strncpy(name, newName, 49);
        name[49] = '\0';
    }
    return *this;
}

Item &Item::setContent(const char *newText)
{
    if (isFolder == 0 && newText != NULL) {
        strncpy(content, newText, 199);
        content[199] = '\0';
        size = strlen(content);
    }
    return *this;
}

const char *Item::getName() const { return name; }
int Item::getIsFolder() const { return isFolder; }
int Item::getChildCount() const { return children.getSize(); }
DateTime Item::getDate() const { return createdDate; }
Item *Item::getParent() const { return parent; }

int Item::getTotalSize() const
{
    if (isFolder == 0) return size;

    int total = 0;
    for (int i = 0; i < children.getSize(); i++)
    {
        total += children[i]->getTotalSize();
    }
    return total;
}

int Item::addChild(Item *child)
{
    if (child == NULL || isFolder == 0) return 0;

    if (child == this) {
        cout << "Error: Cannot add folder inside itself!\n";
        return 0;
    }
    if (child->parent != NULL) {
        cout << "Error: Item already belongs to another folder!\n";
        return 0;
    }
    for (Item *p = parent; p != NULL; p = p->parent) {
        if (p == child) {
            cout << "Error: Cannot add folder inside its own sub-folder!\n";
            return 0;
        }
    }
    if (findChild(child->name) != NULL) {
        cout << "Error: Name already exists in folder.\n";
        return 0;
    }

    child->parent = this;
    children.pushBack(child);
    return 1;
}

Item *Item::findChild(const char *childName) const
{
    if (childName == NULL) return NULL;
    for (int i = 0; i < children.getSize(); i++)
    {
        if (strcmp(children[i]->getName(), childName) == 0)
        {
            return children[i];
        }
    }
    return NULL;
}

Item *Item::removeChild(const char *childName)
{
    if (childName == NULL) return NULL;
    for (int i = 0; i < children.getSize(); i++)
    {
        if (strcmp(children[i]->getName(), childName) == 0)
        {
            Item *removed = children[i];
            children.removeAt(i);
            removed->parent = NULL;
            return removed;
        }
    }
    return NULL;
}

int Item::deleteChild(const char *childName)
{
    Item *target = removeChild(childName);
    if (target != NULL)
    {
        delete target;
        return 1;
    }
    return 0;
}

const Item &Item::operator=(const Item &other)
{
    if (this != &other)
    {
        for (int i = children.getSize() - 1; i >= 0; i--) {
            delete children[i];
            children.removeAt(i);
        }

        strcpy(name, other.name);
        isFolder = other.isFolder;
        strcpy(content, other.content);
        size = other.size;
        version = other.version;
        createdDate = other.createdDate;

        for (int i = 0; i < other.children.getSize(); i++)
        {
            Item *copyChild = new Item(*(other.children[i]));
            copyChild->parent = this;
            children.pushBack(copyChild);
        }
    }
    return *this;
}

bool Item::operator==(const Item &other) const
{
    return (strcmp(name, other.name) == 0 && isFolder == other.isFolder && getTotalSize() == other.getTotalSize());
}

Item *Item::operator[](int index) const
{
    if (index >= 0 && index < children.getSize()) {
        return children[index];
    }
    return NULL;
}

Item &Item::operator+=(Item *child)
{
    addChild(child);
    return *this;
}

Item Item::operator+(const Item &other) const
{
    Item result("merged.txt", 0, "");
    char combined[405];

    strcpy(combined, content);
    strcat(combined, "_");
    strcat(combined, other.content);

    result.setContent(combined);
    return result;
}

Item &Item::operator++()
{
    ++version;
    return *this;
}

Item Item::operator++(int)
{
    Item temp(*this);
    version++;
    return temp;
}

int Item::getTotalItems()
{
    return totalItems;
}

ostream &operator<<(ostream &out, const Item &it)
{
    if (it.isFolder == 1) out << "[FOLDER] ";
    else out << "[FILE]   ";

    out << it.name << " | Size: " << it.getTotalSize() << " B | v" << it.version << " | Date: ";
    out << it.createdDate;
    return out;
}

istream &operator>>(istream &in, Item &it)
{
    if (it.isFolder == 1) return in;
    cout << "Enter content: ";
    in >> ws;
    in.getline(it.content, 200);
    if (in.fail()) {
        in.clear();
        in.ignore(1000, '\n');
    }
    it.size = strlen(it.content);
    it.version++;
    return in;
}
