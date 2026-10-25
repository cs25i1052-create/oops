#ifndef EXPLORER_H
#define EXPLORER_H

#include "Item.h"

// [Syllabus Mod 2 & 10] Comparison functions for Function Pointers
bool compareByName(const Item* a, const Item* b);
bool compareBySize(const Item* a, const Item* b);
bool compareByDate(const Item* a, const Item* b);

class Explorer {
private:
    Item* root;
    Item* currentFolder;

    // Recursive helpers
    void printTreeRecursive(const Item* node, string prefix, bool isLast) const;
    void searchRecursive(const Item* node, const string& target, string currentPath, int& foundCount) const;

public:
    Explorer();
    ~Explorer();

    string getCurrentPath() const;

    // [Syllabus Mod 3] Function Overloading for creating files
    bool createFolder(const string& folderName);
    bool createFile(const string& fileName);
    bool createFile(const string& fileName, const string& initialContent);

    bool openFolder(const string& folderName);
    bool goBack();
    void listCurrentFolder() const;
    void showFullTree() const;
    void searchItem(const string& name) const;
    bool renameItem(const string& oldName, const string& newName);
    bool copyItem(const string& sourceName, const string& destFolderName);
    bool moveItem(const string& sourceName, const string& destFolderName);
    bool deleteItem(const string& name);
    void editFileContent(const string& fileName);

    // [Syllabus Mod 2 & 10] Bubble Sort accepting a Function Pointer
    void sortCurrentFolder(bool (*cmp)(const Item*, const Item*));

    void runAutomatedDemo();
    void startMenu();
};

#endif