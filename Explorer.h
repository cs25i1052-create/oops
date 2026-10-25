#ifndef EXPLORER_H
#define EXPLORER_H

#include "Item.h"

bool compareByName(const Item *a, const Item *b);
bool compareBySize(const Item *a, const Item *b);
bool compareByDate(const Item *a, const Item *b);
bool compareByType(const Item *a, const Item *b);

class Explorer
{
private:
    Item *root;
    Item *currentFolder;
    Item *trashFolder;

    Item *clipboardItem;
    bool isCutOperation;
    Item *selectedItem;

    bool useColors;
    string currentSortLabel;
    string lastMessage;
    int lastMessageType;

    static const int MSG_NONE = 0;
    static const int MSG_SUCCESS = 1;
    static const int MSG_ERROR = 2;
    static const int MSG_INFO = 3;

    bool isValidName(const string &name) const;
    Item *resolvePath(const string &path) const;
    bool isSelfOrDescendant(const Item *ancestor, const Item *node) const;
    string intToString(int num) const;
    bool isAllDigits(const string &text) const;
    string getUniqueName(const Item *folder, const string &baseName) const;
    bool isClipboardAffected(const Item *target) const;

    void setMessage(const string &msg, int type);
    void printLastMessage() const;
    string color(const string &ansiCode) const;
    void printPadded(const string &text, int width) const;
    void printLine(const string &left, const string &fill, const string &right, int width) const;
    string getBreadcrumbPath() const;
    string getPreviewText(const Item *item) const;
    int getVisibleCount() const;
    Item *getVisibleItem(int oneBasedIndex) const;
    void waitForEnter() const;

    void renderDashboard() const;
    void handleDashboardCopyOrMove(bool isMove);
    void handleDashboardSort();
    void showDashboardHelp() const;

    void printTreeRecursive(const Item *node, string prefix, bool isLast) const;
    void searchRecursive(const Item *node, const string &target, string currentPath, int &foundCount) const;

public:
    Explorer();
    ~Explorer();

    string getCurrentPath() const;

    bool createFolder(const string &folderName);
    bool createFile(const string &fileName);
    bool createFile(const string &fileName, const string &initialContent);

    bool openFolder(const string &path);
    bool goBack();
    void listCurrentFolder() const;
    void listCurrentFolder(bool showDetails) const;
    void showFullTree() const;
    void searchItem(const string &name);
    bool renameItem(const string &oldName, const string &newName);
    bool copyItem(const string &sourceName, const string &destFolderPath);
    bool moveItem(const string &sourceName, const string &destFolderPath);
    bool deleteItem(const string &name);

    bool copyToClipboard(const string &name);
    bool cutToClipboard(const string &name);
    bool pasteClipboard();

    bool moveToTrash(const string &name);
    void listTrash() const;
    bool restoreFromTrash(const string &name);
    void emptyTrash();

    void editFileContent(const string &fileName);
    void readFileContent(const string &fileName) const;
    void showItemInfo(const string &name) const;
    void showHelp() const;

    void sortCurrentFolder(bool (*cmp)(const Item *, const Item *), bool descending = false);

    void runAutomatedDemo();
    void startMenu();
    void startShell();
    void startDashboard();
};

#endif