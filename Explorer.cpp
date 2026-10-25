#include "Explorer.h"
#include <cctype>

const string CLR_RESET = "\033[0m";
const string CLR_HEADER = "\033[1;37;44m";
const string CLR_FOLDER = "\033[1;36m";
const string CLR_SELECT = "\033[7m";
const string CLR_SUCCESS = "\033[1;32m";
const string CLR_ERROR = "\033[1;31m";
const string CLR_INFO = "\033[1;33m";
const string CLR_HINT = "\033[2;37m";

int compareIgnoreCase(const char *a, const char *b)
{
    int i = 0;
    while (a[i] != '\0' && b[i] != '\0')
    {
        int c1 = tolower((unsigned char)a[i]);
        int c2 = tolower((unsigned char)b[i]);
        if (c1 != c2)
        {
            return c1 - c2;
        }
        i++;
    }
    return tolower((unsigned char)a[i]) - tolower((unsigned char)b[i]);
}

bool compareByName(const Item *a, const Item *b)
{
    int r = compareIgnoreCase(a->getName(), b->getName());
    if (r == 0)
    {
        r = strcmp(a->getName(), b->getName());
    }
    return r > 0;
}

bool compareBySize(const Item *a, const Item *b)
{
    return a->getTotalSize() < b->getTotalSize();
}

bool compareByDate(const Item *a, const Item *b)
{
    return a->getDate() > b->getDate();
}

bool compareByType(const Item *a, const Item *b)
{
    if (a->getIsFolder() != b->getIsFolder())
    {
        return !a->getIsFolder();
    }
    return compareByName(a, b);
}

Explorer::Explorer()
{
    root = new Item("/", 1, "");
    currentFolder = root;

    trashFolder = new Item(".Trash", 1, "");
    *root += trashFolder;

    clipboardItem = nullptr;
    isCutOperation = false;
    selectedItem = nullptr;

    useColors = true;
    currentSortLabel = "None";
    lastMessage = "Welcome! Type an item number or a command letter.";
    lastMessageType = MSG_INFO;
}

Explorer::~Explorer()
{
    delete root;
}

void Explorer::setMessage(const string &msg, int type)
{
    lastMessage = msg;
    lastMessageType = type;
}

void Explorer::printLastMessage() const
{
    if (lastMessageType == MSG_SUCCESS)
    {
        cout << "  [Success] " << lastMessage << "\n";
    }
    else if (lastMessageType == MSG_ERROR)
    {
        cout << "  [Error] " << lastMessage << "\n";
    }
    else if (lastMessageType == MSG_INFO)
    {
        cout << "  [Info] " << lastMessage << "\n";
    }
}

string Explorer::color(const string &ansiCode) const
{
    if (!useColors)
        return "";
    return ansiCode;
}

void Explorer::printPadded(const string &text, int width) const
{
    int len = text.length();
    if (len > width)
    {
        if (width >= 2)
        {
            for (int i = 0; i < width - 2; i++)
            {
                cout << text[i];
            }
            cout << "..";
        }
        else
        {
            for (int i = 0; i < width; i++)
            {
                cout << text[i];
            }
        }
    }
    else
    {
        cout << text;
        for (int i = len; i < width; i++)
        {
            cout << " ";
        }
    }
}

void Explorer::printLine(const string &left, const string &fill, const string &right, int width) const
{
    cout << left;
    for (int i = 0; i < width; i++)
    {
        cout << fill;
    }
    cout << right << "\n";
}

bool Explorer::isValidName(const string &name) const
{
    if (name.length() == 0 || name.length() >= 50)
    {
        return false;
    }
    if (name == "." || name == ".." || name == ".Trash")
    {
        return false;
    }
    if (strchr(name.c_str(), '/') != nullptr)
    {
        return false;
    }
    return true;
}

Item *Explorer::resolvePath(const string &path) const
{
    if (path.length() == 0)
        return nullptr;
    if (path == "/")
        return root;
    if (path == ".")
        return currentFolder;
    if (path == "..")
        return currentFolder->getParent();

    Item *node = (path[0] == '/') ? root : currentFolder;
    int i = (path[0] == '/') ? 1 : 0;
    int len = path.length();

    while (i < len)
    {
        string part = "";
        while (i < len && path[i] != '/')
        {
            part += path[i];
            i++;
        }
        i++;

        if (part.length() == 0 || part == ".")
            continue;
        if (part == ".Trash")
            return nullptr;
        if (part == "..")
        {
            node = node->getParent();
        }
        else
        {
            node = node->findChild(part.c_str());
        }

        if (node == nullptr)
            return nullptr;
        if (!node->getIsFolder() && i < len)
            return nullptr;
    }
    return node;
}

bool Explorer::isSelfOrDescendant(const Item *ancestor, const Item *node) const
{
    for (const Item *p = node; p != nullptr; p = p->getParent())
    {
        if (p == ancestor)
            return true;
    }
    return false;
}

string Explorer::intToString(int num) const
{
    if (num <= 0)
        return "0";
    string result = "";
    while (num > 0)
    {
        char digit = '0' + (num % 10);
        result = digit + result;
        num = num / 10;
    }
    return result;
}

bool Explorer::isAllDigits(const string &text) const
{
    if (text.length() == 0)
        return false;
    for (int i = 0; i < (int)text.length(); i++)
    {
        if (text[i] < '0' || text[i] > '9')
        {
            return false;
        }
    }
    return true;
}

string Explorer::getUniqueName(const Item *folder, const string &baseName) const
{
    if (folder->findChild(baseName.c_str()) == nullptr)
    {
        return baseName;
    }
    string candidate = baseName + "_copy";
    int suffix = 2;
    while (folder->findChild(candidate.c_str()) != nullptr)
    {
        candidate = baseName + "_copy" + intToString(suffix);
        suffix++;
    }
    if (candidate.length() >= 50)
    {
        candidate = candidate.substr(0, 49);
    }
    return candidate;
}

bool Explorer::isClipboardAffected(const Item *target) const
{
    if (clipboardItem == nullptr || target == nullptr)
        return false;
    return isSelfOrDescendant(target, clipboardItem);
}

string Explorer::getCurrentPath() const
{
    if (currentFolder == root)
        return "/";
    string path = "";
    const Item *temp = currentFolder;
    while (temp != nullptr && temp != root)
    {
        path = string("/") + temp->getName() + path;
        temp = temp->getParent();
    }
    return path;
}

string Explorer::getBreadcrumbPath() const
{
    if (currentFolder == root)
        return "/";
    string path = "";
    const Item *temp = currentFolder;
    while (temp != nullptr && temp != root)
    {
        path = string("  >  ") + temp->getName() + path;
        temp = temp->getParent();
    }
    return string("/") + path;
}

string Explorer::getPreviewText(const Item *item) const
{
    if (item == nullptr)
        return "";
    if (item->getIsFolder())
    {
        return string("(Folder containing ") + intToString(item->children.getSize()) + " item(s))";
    }
    if (strlen(item->content) == 0)
    {
        return "(Empty file)";
    }
    string preview = "";
    int len = strlen(item->content);
    int limit = (len < 60) ? len : 60;
    for (int i = 0; i < limit; i++)
    {
        char c = item->content[i];
        if (c == '\n' || c == '\r' || c == '\t')
        {
            preview += ' ';
        }
        else
        {
            preview += c;
        }
    }
    if (len > 60)
    {
        preview += "..";
    }
    return preview;
}

int Explorer::getVisibleCount() const
{
    int count = 0;
    for (int i = 0; i < currentFolder->getChildCount(); i++)
    {
        Item *child = (*currentFolder)[i];
        if (strcmp(child->getName(), ".Trash") == 0)
            continue;
        count++;
    }
    return count;
}

Item *Explorer::getVisibleItem(int oneBasedIndex) const
{
    if (oneBasedIndex <= 0)
        return nullptr;
    int current = 0;
    for (int i = 0; i < currentFolder->getChildCount(); i++)
    {
        Item *child = (*currentFolder)[i];
        if (strcmp(child->getName(), ".Trash") == 0)
            continue;
        current++;
        if (current == oneBasedIndex)
        {
            return child;
        }
    }
    return nullptr;
}

void Explorer::waitForEnter() const
{
    cout << "\n  " << color(CLR_HINT) << "Press Enter to return to dashboard..." << color(CLR_RESET);
    string dummy;
    getline(cin, dummy);
}

bool Explorer::createFolder(const string &folderName)
{
    if (!isValidName(folderName))
    {
        setMessage("Invalid folder name (1-49 chars, no '/', '.', '..', or '.Trash')!", MSG_ERROR);
        return false;
    }
    if (currentFolder->findChild(folderName.c_str()) != nullptr)
    {
        setMessage(string("Item '") + folderName + "' already exists in this folder!", MSG_ERROR);
        return false;
    }
    Item *newFolder = new Item(folderName.c_str(), 1);
    *currentFolder += newFolder;
    if (newFolder->getParent() != currentFolder)
    {
        delete newFolder;
        setMessage("Failed to create folder!", MSG_ERROR);
        return false;
    }
    setMessage(string("Folder '") + folderName + "' created.", MSG_SUCCESS);
    return true;
}

bool Explorer::createFile(const string &fileName)
{
    return createFile(fileName, "");
}

bool Explorer::createFile(const string &fileName, const string &initialContent)
{
    if (!isValidName(fileName))
    {
        setMessage("Invalid file name (1-49 chars, no '/', '.', '..', or '.Trash')!", MSG_ERROR);
        return false;
    }
    if (initialContent.length() >= 200)
    {
        setMessage("Content is too long (must be under 200 characters)!", MSG_ERROR);
        return false;
    }
    if (currentFolder->findChild(fileName.c_str()) != nullptr)
    {
        setMessage(string("Item '") + fileName + "' already exists in this folder!", MSG_ERROR);
        return false;
    }
    Item *newFile = new Item(fileName.c_str(), 0, initialContent.c_str());
    *currentFolder += newFile;
    if (newFile->getParent() != currentFolder)
    {
        delete newFile;
        setMessage("Failed to create file!", MSG_ERROR);
        return false;
    }
    setMessage(string("File '") + fileName + "' (" + intToString(newFile->getTotalSize()) + " B) created.", MSG_SUCCESS);
    return true;
}

bool Explorer::openFolder(const string &path)
{
    if (path.length() == 0)
    {
        setMessage("Path cannot be empty!", MSG_ERROR);
        return false;
    }
    if (path == "..")
    {
        return goBack();
    }
    Item *target = resolvePath(path);
    if (target == nullptr)
    {
        setMessage(string("Folder '") + path + "' not found.", MSG_ERROR);
        return false;
    }
    if (!target->getIsFolder())
    {
        setMessage(string("'") + path + "' is a file, not a folder!", MSG_ERROR);
        return false;
    }
    currentFolder = target;
    selectedItem = nullptr;
    if (currentFolder == root)
    {
        setMessage("Opened root directory '/'.", MSG_INFO);
    }
    else
    {
        setMessage(string("Opened folder '") + currentFolder->getName() + "'.", MSG_INFO);
    }
    return true;
}

bool Explorer::goBack()
{
    if (currentFolder == root)
    {
        setMessage("Already at root directory '/'.", MSG_INFO);
        return false;
    }
    currentFolder = currentFolder->getParent();
    selectedItem = nullptr;
    setMessage(string("Moved back to '") + getCurrentPath() + "'.", MSG_INFO);
    return true;
}

void Explorer::listCurrentFolder() const
{
    listCurrentFolder(true);
}

void Explorer::listCurrentFolder(bool showDetails) const
{
    string blue = color("\033[1;34m");
    string white = color("\033[0;37m");
    string reset = color(CLR_RESET);

    int visibleCount = 0;
    int totalSize = 0;
    for (int i = 0; i < currentFolder->getChildCount(); i++)
    {
        Item *child = (*currentFolder)[i];
        if (strcmp(child->getName(), ".Trash") == 0)
            continue;
        visibleCount++;
        totalSize += child->getTotalSize();
    }

    cout << "\n--- Contents of " << getCurrentPath()
         << " (Items: " << visibleCount << " | Total Size: " << totalSize << " B) ---\n";

    if (visibleCount == 0)
    {
        cout << "  (Empty Folder)\n";
        return;
    }

    int index = 1;

    for (int i = 0; i < currentFolder->getChildCount(); i++)
    {
        Item *child = (*currentFolder)[i];
        if (strcmp(child->getName(), ".Trash") == 0)
            continue;
        if (child->getIsFolder())
        {
            cout << "  " << index++ << ". " << blue;
            if (showDetails)
            {
                cout << *child;
            }
            else
            {
                cout << "[FOLDER] " << child->getName();
            }
            cout << reset << "\n";
        }
    }

    for (int i = 0; i < currentFolder->getChildCount(); i++)
    {
        Item *child = (*currentFolder)[i];
        if (!child->getIsFolder())
        {
            cout << "  " << index++ << ". " << white;
            if (showDetails)
            {
                cout << *child;
            }
            else
            {
                cout << "[FILE]   " << child->getName();
            }
            cout << reset << "\n";
        }
    }
}

void Explorer::printTreeRecursive(const Item *node, string prefix, bool isLast) const
{
    cout << prefix << (isLast ? "└── " : "├── ")
         << node->getName() << (node->getIsFolder() ? "/" : "")
         << " (" << node->getTotalSize() << " B, v" << node->version << ")\n";

    string nextPrefix = prefix + (isLast ? "    " : "│   ");
    for (int i = 0; i < node->getChildCount(); i++)
    {
        bool lastChild = (i == node->getChildCount() - 1);
        printTreeRecursive((*node)[i], nextPrefix, lastChild);
    }
}

void Explorer::showFullTree() const
{
    int rootSize = root->getTotalSize() - trashFolder->getTotalSize();
    cout << "\n========== COMPLETE DIRECTORY TREE ==========\n";
    cout << "/ (Total Size: " << rootSize << " B)\n";

    int lastVisible = -1;
    for (int i = 0; i < root->getChildCount(); i++)
    {
        if (strcmp((*root)[i]->getName(), ".Trash") != 0)
        {
            lastVisible = i;
        }
    }

    for (int i = 0; i < root->getChildCount(); i++)
    {
        if (strcmp((*root)[i]->getName(), ".Trash") == 0)
            continue;
        printTreeRecursive((*root)[i], "", i == lastVisible);
    }
    cout << "=============================================\n";
}

void Explorer::searchRecursive(const Item *node, const string &target, string currentPath, int &foundCount) const
{
    for (int i = 0; i < node->getChildCount(); i++)
    {
        Item *child = (*node)[i];
        if (strcmp(child->getName(), ".Trash") == 0)
            continue;

        string childPath = (currentPath == "/") ? (string("/") + child->getName()) : (currentPath + "/" + child->getName());
        if (child->getName() == target || strstr(child->getName(), target.c_str()) != nullptr)
        {
            cout << "  Found: " << childPath << " -> " << *child << "\n";
            foundCount++;
        }
        if (child->getIsFolder())
        {
            searchRecursive(child, target, childPath, foundCount);
        }
    }
}

void Explorer::searchItem(const string &name)
{
    if (name.length() == 0)
    {
        setMessage("Search term cannot be empty!", MSG_ERROR);
        cout << "  [Error] " << lastMessage << "\n";
        return;
    }
    cout << "\nSearching for '" << name << "' across file system...\n";
    int count = 0;
    searchRecursive(root, name, "/", count);
    if (count == 0)
    {
        cout << "  No matching items found.\n";
        setMessage(string("No items found matching '") + name + "'.", MSG_INFO);
    }
    else
    {
        setMessage(string("Found ") + intToString(count) + " match(es) for '" + name + "'.", MSG_SUCCESS);
    }
}

bool Explorer::renameItem(const string &oldName, const string &newName)
{
    if (oldName == ".Trash")
    {
        setMessage("Cannot rename '.Trash'!", MSG_ERROR);
        return false;
    }
    Item *target = currentFolder->findChild(oldName.c_str());
    if (!target)
    {
        setMessage(string("'") + oldName + "' not found.", MSG_ERROR);
        return false;
    }
    if (!isValidName(newName))
    {
        setMessage("Invalid new name (1-49 chars, no '/', '.', '..', or '.Trash')!", MSG_ERROR);
        return false;
    }
    if (oldName == newName)
    {
        setMessage("New name is identical to the current name.", MSG_INFO);
        return false;
    }
    if (currentFolder->findChild(newName.c_str()) != nullptr)
    {
        setMessage(string("'") + newName + "' already exists!", MSG_ERROR);
        return false;
    }
    target->setName(newName.c_str());
    ++(*target);
    setMessage(string("Renamed to '") + newName + "' (v" + intToString(target->version) + ").", MSG_SUCCESS);
    return true;
}

bool Explorer::copyItem(const string &sourceName, const string &destFolderPath)
{
    if (sourceName == ".Trash" || destFolderPath == ".Trash")
    {
        setMessage("Cannot use '.Trash' in copy operation!", MSG_ERROR);
        return false;
    }
    Item *src = currentFolder->findChild(sourceName.c_str());
    if (!src)
    {
        setMessage(string("Source '") + sourceName + "' not found.", MSG_ERROR);
        return false;
    }
    Item *dest = resolvePath(destFolderPath);
    if (!dest || !dest->getIsFolder())
    {
        setMessage(string("Destination folder '") + destFolderPath + "' is invalid.", MSG_ERROR);
        return false;
    }
    if (isSelfOrDescendant(src, dest))
    {
        setMessage(string("Cannot copy '") + sourceName + "' into itself or its own subfolder!", MSG_ERROR);
        return false;
    }
    if (dest->findChild(src->getName()) != nullptr)
    {
        setMessage(string("'") + dest->getName() + "' already contains '" + src->getName() + "'!", MSG_ERROR);
        return false;
    }

    Item *deepClone = new Item(*src);
    if (!dest->addChild(deepClone))
    {
        delete deepClone;
        setMessage("Duplicate name or invalid destination folder!", MSG_ERROR);
        return false;
    }
    setMessage(string("Deep copied '") + sourceName + "' into '" + dest->getName() + "'.", MSG_SUCCESS);
    return true;
}

bool Explorer::moveItem(const string &sourceName, const string &destFolderPath)
{
    if (sourceName == ".Trash" || destFolderPath == ".Trash")
    {
        setMessage("Cannot use '.Trash' in move operation!", MSG_ERROR);
        return false;
    }
    Item *src = currentFolder->findChild(sourceName.c_str());
    if (!src)
    {
        setMessage(string("Source item '") + sourceName + "' not found.", MSG_ERROR);
        return false;
    }
    Item *dest = resolvePath(destFolderPath);
    if (!dest || !dest->getIsFolder())
    {
        setMessage(string("Destination folder '") + destFolderPath + "' not found.", MSG_ERROR);
        return false;
    }
    if (dest == currentFolder)
    {
        setMessage(string("'") + sourceName + "' is already in that folder.", MSG_INFO);
        return false;
    }
    if (isSelfOrDescendant(src, dest))
    {
        setMessage(string("Cannot move '") + sourceName + "' into itself or its own subfolder!", MSG_ERROR);
        return false;
    }
    if (dest->findChild(src->getName()) != nullptr)
    {
        setMessage("Item with same name already exists in destination!", MSG_ERROR);
        return false;
    }
    Item *movingItem = currentFolder->removeChild(sourceName.c_str());
    if (!movingItem)
    {
        setMessage(string("Source item '") + sourceName + "' not found.", MSG_ERROR);
        return false;
    }
    if (!dest->addChild(movingItem))
    {
        currentFolder->addChild(movingItem);
        setMessage(string("Move failed! Restored '") + sourceName + "' to original folder.", MSG_ERROR);
        return false;
    }
    if (selectedItem == movingItem)
    {
        selectedItem = nullptr;
    }
    setMessage(string("Moved '") + sourceName + "' into '" + dest->getName() + "'.", MSG_SUCCESS);
    return true;
}

bool Explorer::deleteItem(const string &name)
{
    if (name == ".Trash")
    {
        setMessage("Cannot delete '.Trash'!", MSG_ERROR);
        return false;
    }
    Item *target = currentFolder->findChild(name.c_str());
    if (target && isClipboardAffected(target))
    {
        clipboardItem = nullptr;
        isCutOperation = false;
    }
    if (target && selectedItem == target)
    {
        selectedItem = nullptr;
    }

    int beforeNodes = Item::getTotalItems();
    if (currentFolder->deleteChild(name.c_str()))
    {
        int afterNodes = Item::getTotalItems();
        setMessage(string("Deleted '") + name + "'. Memory freed for " + intToString(beforeNodes - afterNodes) + " node(s).", MSG_SUCCESS);
        return true;
    }
    setMessage(string("Item '") + name + "' not found.", MSG_ERROR);
    return false;
}

bool Explorer::copyToClipboard(const string &name)
{
    if (name == ".Trash")
    {
        setMessage("Cannot copy '.Trash'!", MSG_ERROR);
        return false;
    }
    Item *target = currentFolder->findChild(name.c_str());
    if (!target)
    {
        setMessage(string("Item '") + name + "' not found.", MSG_ERROR);
        return false;
    }
    clipboardItem = target;
    isCutOperation = false;
    setMessage(string("Copied '") + name + "' to clipboard.", MSG_SUCCESS);
    return true;
}

bool Explorer::cutToClipboard(const string &name)
{
    if (name == ".Trash")
    {
        setMessage("Cannot cut '.Trash'!", MSG_ERROR);
        return false;
    }
    Item *target = currentFolder->findChild(name.c_str());
    if (!target)
    {
        setMessage(string("Item '") + name + "' not found.", MSG_ERROR);
        return false;
    }
    clipboardItem = target;
    isCutOperation = true;
    setMessage(string("Cut '") + name + "' to clipboard.", MSG_SUCCESS);
    return true;
}

bool Explorer::pasteClipboard()
{
    if (clipboardItem == nullptr)
    {
        setMessage("Clipboard is empty!", MSG_ERROR);
        return false;
    }

    if (!isCutOperation)
    {
        if (isSelfOrDescendant(clipboardItem, currentFolder))
        {
            setMessage("Cannot copy a folder into itself or its own subfolder!", MSG_ERROR);
            return false;
        }
        string newName = getUniqueName(currentFolder, clipboardItem->getName());
        Item *deepClone = new Item(*clipboardItem);
        deepClone->setName(newName.c_str());
        if (!currentFolder->addChild(deepClone))
        {
            delete deepClone;
            setMessage("Paste failed!", MSG_ERROR);
            return false;
        }
        setMessage(string("Pasted '") + newName + "' into " + getCurrentPath() + ".", MSG_SUCCESS);
        return true;
    }
    else
    {
        if (isSelfOrDescendant(clipboardItem, currentFolder))
        {
            setMessage("Cannot paste a folder into itself or its subfolder!", MSG_ERROR);
            return false;
        }
        Item *oldParent = clipboardItem->getParent();
        if (oldParent == nullptr)
        {
            setMessage("Cannot move root item!", MSG_ERROR);
            return false;
        }

        string origName = clipboardItem->getName();
        string newName = (oldParent == currentFolder) ? origName : getUniqueName(currentFolder, origName);

        oldParent->removeChild(origName.c_str());
        clipboardItem->setName(newName.c_str());

        if (!currentFolder->addChild(clipboardItem))
        {
            clipboardItem->setName(origName.c_str());
            oldParent->addChild(clipboardItem);
            setMessage("Paste failed! Restored item to original folder.", MSG_ERROR);
            return false;
        }

        setMessage(string("Moved '") + newName + "' into " + getCurrentPath() + ".", MSG_SUCCESS);
        clipboardItem = nullptr;
        isCutOperation = false;
        return true;
    }
}

bool Explorer::moveToTrash(const string &name)
{
    if (name == ".Trash")
    {
        setMessage("Cannot delete '.Trash' folder!", MSG_ERROR);
        return false;
    }
    Item *target = currentFolder->findChild(name.c_str());
    if (!target)
    {
        setMessage(string("Item '") + name + "' not found.", MSG_ERROR);
        return false;
    }

    if (isClipboardAffected(target))
    {
        clipboardItem = nullptr;
        isCutOperation = false;
    }
    if (selectedItem == target)
    {
        selectedItem = nullptr;
    }

    Item *removed = currentFolder->removeChild(name.c_str());
    string trashName = getUniqueName(trashFolder, removed->getName());
    removed->setName(trashName.c_str());
    trashFolder->addChild(removed);

    setMessage(string("Moved '") + name + "' to Trash.", MSG_SUCCESS);
    return true;
}

void Explorer::listTrash() const
{
    cout << "\n--- Contents of Trash (/.Trash) ---\n";
    if (trashFolder->getChildCount() == 0)
    {
        cout << "  (Trash is empty)\n";
        return;
    }
    for (int i = 0; i < trashFolder->getChildCount(); i++)
    {
        cout << "  " << i + 1 << ". " << *((*trashFolder)[i]) << "\n";
    }
}

bool Explorer::restoreFromTrash(const string &name)
{
    Item *target = trashFolder->findChild(name.c_str());
    if (!target)
    {
        setMessage(string("Item '") + name + "' not found in Trash.", MSG_ERROR);
        return false;
    }
    Item *removed = trashFolder->removeChild(name.c_str());
    string restoredName = getUniqueName(currentFolder, removed->getName());
    removed->setName(restoredName.c_str());
    currentFolder->addChild(removed);

    setMessage(string("Restored '") + restoredName + "' to " + getCurrentPath() + ".", MSG_SUCCESS);
    return true;
}

void Explorer::emptyTrash()
{
    if (trashFolder->getChildCount() == 0)
    {
        setMessage("Trash is already empty.", MSG_INFO);
        printLastMessage();
        return;
    }
    while (trashFolder->getChildCount() > 0)
    {
        Item *child = (*trashFolder)[0];
        if (isClipboardAffected(child))
        {
            clipboardItem = nullptr;
            isCutOperation = false;
        }
        string childName = child->getName();
        int beforeNodes = Item::getTotalItems();
        trashFolder->deleteChild(childName.c_str());
        int afterNodes = Item::getTotalItems();
        setMessage(string("Deleted '") + childName + "'. Memory freed for " + intToString(beforeNodes - afterNodes) + " node(s).", MSG_SUCCESS);
        printLastMessage();
    }
}

void Explorer::editFileContent(const string &fileName)
{
    Item *target = currentFolder->findChild(fileName.c_str());
    if (!target)
    {
        setMessage("File not found.", MSG_ERROR);
        return;
    }
    if (target->getIsFolder())
    {
        setMessage(string("'") + fileName + "' is a folder, not a file!", MSG_ERROR);
        return;
    }
    cin >> *target;
    setMessage(string("Updated file '") + target->getName() + "' (v" + intToString(target->version) + ").", MSG_SUCCESS);
}

void Explorer::readFileContent(const string &fileName) const
{
    Item *target = currentFolder->findChild(fileName.c_str());
    if (!target)
    {
        cout << "  [Error] File '" << fileName << "' not found.\n";
        return;
    }
    if (target->getIsFolder())
    {
        cout << "  [Error] '" << fileName << "' is a folder, not a file!\n";
        return;
    }
    cout << "  --- " << target->getName() << " (" << target->getTotalSize()
         << " B, v" << target->version << ") ---\n";
    cout << "  " << target->content << "\n";
}

void Explorer::showItemInfo(const string &name) const
{
    if (name == ".Trash")
    {
        cout << "  [Error] Item '" << name << "' not found.\n";
        return;
    }
    Item *target = currentFolder->findChild(name.c_str());
    if (!target)
    {
        cout << "  [Error] Item '" << name << "' not found.\n";
        return;
    }
    string curPath = getCurrentPath();
    string fullPath = (curPath == "/") ? (string("/") + target->getName()) : (curPath + "/" + target->getName());

    cout << "\n--- Item Details ---\n";
    cout << "  Name:               " << target->getName() << "\n";
    cout << "  Type:               " << (target->getIsFolder() ? "Folder" : "File") << "\n";
    cout << "  Full Path:          " << fullPath << "\n";
    cout << "  Total Size:         " << target->getTotalSize() << " B\n";
    cout << "  Number of Children: " << target->children.getSize() << "\n";
    cout << "  Created Date:       " << target->createdDate << "\n";
    cout << "  Version:            v" << target->version << "\n";
    if (!target->getIsFolder())
    {
        cout << "  Content:            " << target->content << "\n";
    }
}

void Explorer::showHelp() const
{
    cout << "\n--- Available Shell Commands ---\n";
    cout << "  ls                         : List current folder (folders in blue, files in white)\n";
    cout << "  ls -l                      : List with size, created date, and version\n";
    cout << "  cd <path>                  : Change directory (e.g. /College/OOP, ../Backup, /)\n";
    cout << "  pwd                        : Print current directory path\n";
    cout << "  mkdir <name>               : Create a new folder\n";
    cout << "  touch <name>               : Create a new file (prompts for content)\n";
    cout << "  cat <name>                 : View file content\n";
    cout << "  edit <name>                : Edit file content\n";
    cout << "  copy <name>                : Copy item to clipboard\n";
    cout << "  cut <name>                 : Cut item to clipboard\n";
    cout << "  paste                      : Paste clipboard into current folder\n";
    cout << "  cp <name> <path>           : Deep copy item into folder path (or '..')\n";
    cout << "  mv <name> <path>           : Move item into folder path (or '..')\n";
    cout << "  rename <old> <new>         : Rename an item in current folder\n";
    cout << "  rm <name>                  : Move item to Trash (.Trash)\n";
    cout << "  trash                      : List items inside Trash\n";
    cout << "  restore <name>             : Restore item from Trash to current folder\n";
    cout << "  emptytrash                 : Permanently delete all items in Trash\n";
    cout << "  find <name>                : Search exact or partial name across tree\n";
    cout << "  tree                       : Display complete directory tree\n";
    cout << "  sort <name|size|date|type> : Sort current folder contents\n";
    cout << "  info <name>                : Show full metadata for an item\n";
    cout << "  clear                      : Clear the terminal screen\n";
    cout << "  demo                       : Run automated professor demo\n";
    cout << "  menu                       : Switch to classic numbered menu\n";
    cout << "  help                       : Show this help message\n";
    cout << "  exit                       : Exit shell\n";
}

void Explorer::sortCurrentFolder(bool (*cmp)(const Item *, const Item *), bool descending)
{
    int n = currentFolder->children.getSize();
    for (int i = 0; i < n - 1; i++)
    {
        for (int j = 0; j < n - i - 1; j++)
        {
            Item *left = currentFolder->children[j];
            Item *right = currentFolder->children[j + 1];
            bool outOfOrder = descending ? cmp(right, left) : cmp(left, right);
            if (outOfOrder)
            {
                currentFolder->children.swap(j, j + 1);
            }
        }
    }
    if (cmp == compareByName)
        currentSortLabel = descending ? "Name (Z-A)" : "Name";
    else if (cmp == compareBySize)
        currentSortLabel = descending ? "Size (Low)" : "Size";
    else if (cmp == compareByDate)
        currentSortLabel = descending ? "Date (New)" : "Date";
    else if (cmp == compareByType)
        currentSortLabel = descending ? "Type (Rev)" : "Type";
    setMessage(string("Folder contents sorted by ") + currentSortLabel + " using Function Pointer!", MSG_SUCCESS);
}

void Explorer::runAutomatedDemo()
{
    int passed = 0;
    int total = 0;

    cout << "\n=========================================================\n";
    cout << "   AUTOMATED FULL SYSTEM & EDGE-CASE TEST SUITE\n";
    cout << "=========================================================\n";

    currentFolder = root;
    selectedItem = nullptr;
    clipboardItem = nullptr;
    isCutOperation = false;

    while (root->getChildCount() > 1)
    {
        for (int i = 0; i < root->getChildCount(); i++)
        {
            if (strcmp((*root)[i]->getName(), ".Trash") != 0)
            {
                root->deleteChild((*root)[i]->getName());
                break;
            }
        }
    }
    emptyTrash();

    int startNodes = Item::getTotalItems();
    cout << "Initial Active Nodes in RAM (root + .Trash): " << startNodes << "\n";

    cout << "\n--- [TEST 1] Name & Content Validation Edge Cases ---\n";
    total++;
    if (!createFolder(""))
    {
        passed++;
        cout << "  [PASS] Rejected empty folder name\n";
    }
    else
        cout << "  [FAIL] Allowed empty name\n";
    total++;
    if (!createFolder("."))
    {
        passed++;
        cout << "  [PASS] Rejected '.' folder name\n";
    }
    else
        cout << "  [FAIL] Allowed '.'\n";
    total++;
    if (!createFolder(".."))
    {
        passed++;
        cout << "  [PASS] Rejected '..' folder name\n";
    }
    else
        cout << "  [FAIL] Allowed '..'\n";
    total++;
    if (!createFolder(".Trash"))
    {
        passed++;
        cout << "  [PASS] Rejected reserved '.Trash' name\n";
    }
    else
        cout << "  [FAIL] Allowed '.Trash'\n";
    total++;
    if (!createFolder("bad/name"))
    {
        passed++;
        cout << "  [PASS] Rejected name containing '/'\n";
    }
    else
        cout << "  [FAIL] Allowed '/'\n";

    string longName(55, 'a');
    total++;
    if (!createFolder(longName))
    {
        passed++;
        cout << "  [PASS] Rejected name >= 50 characters\n";
    }
    else
        cout << "  [FAIL] Allowed long name\n";

    string longContent(210, 'x');
    total++;
    if (!createFile("big.txt", longContent))
    {
        passed++;
        cout << "  [PASS] Rejected content >= 200 characters\n";
    }
    else
        cout << "  [FAIL] Allowed long content\n";

    cout << "\n--- [TEST 2] Folder/File Creation & Duplicate Rejection ---\n";
    total++;
    if (createFolder("College"))
    {
        passed++;
        printLastMessage();
    }
    total++;
    if (createFolder("Personal"))
    {
        passed++;
        printLastMessage();
    }
    total++;
    if (createFolder("Backup"))
    {
        passed++;
        printLastMessage();
    }
    total++;
    if (!createFolder("College"))
    {
        passed++;
        cout << "  [PASS] Rejected duplicate folder 'College'\n";
    }
    else
        cout << "  [FAIL] Allowed duplicate folder\n";

    cout << "\n--- [TEST 3] Multi-Step Path Navigation & Invalid Paths ---\n";
    total++;
    if (openFolder("College"))
    {
        passed++;
    }
    total++;
    if (createFolder("OOP"))
    {
        passed++;
    }
    total++;
    if (openFolder("/College/OOP"))
    {
        passed++;
        cout << "  [PASS] Navigated to absolute path /College/OOP\n";
    }
    total++;
    if (createFile("zebra_notes.txt", "Polymorphism and Inheritance come after midsem."))
    {
        passed++;
    }
    total++;
    if (createFile("Alpha_code.cpp", "#include <iostream>\nint main() { return 0; }"))
    {
        passed++;
    }
    total++;
    if (createFile("beta_empty.txt"))
    {
        passed++;
    }
    total++;
    if (!createFile("zebra_notes.txt", "dup"))
    {
        passed++;
        cout << "  [PASS] Rejected duplicate file in same folder\n";
    }

    total++;
    if (!openFolder("/College/NonExistent/OOP") && getCurrentPath() == "/College/OOP")
    {
        passed++;
        cout << "  [PASS] Invalid path rejected without changing currentFolder\n";
    }
    else
        cout << "  [FAIL] Invalid path changed currentFolder\n";

    total++;
    if (!openFolder("zebra_notes.txt"))
    {
        passed++;
        cout << "  [PASS] Rejected cd into a file\n";
    }
    total++;
    if (!openFolder("/.Trash"))
    {
        passed++;
        cout << "  [PASS] Rejected cd into hidden .Trash\n";
    }
    total++;
    if (openFolder("../../Backup") && getCurrentPath() == "/Backup")
    {
        passed++;
        cout << "  [PASS] Relative path '../../Backup' resolved to /Backup\n";
    }
    else
        cout << "  [FAIL] Relative path failed\n";

    openFolder("/");
    total++;
    if (!goBack())
    {
        passed++;
        cout << "  [PASS] Blocked going above root '/'\n";
    }

    cout << "\n--- [TEST 4] Rename, Version Increment, and Friend Info ---\n";
    openFolder("/College/OOP");
    total++;
    if (!renameItem("beta_empty.txt", "bad/name"))
    {
        passed++;
        cout << "  [PASS] Rejected invalid new name on rename\n";
    }
    total++;
    if (!renameItem("beta_empty.txt", "beta_empty.txt"))
    {
        passed++;
        cout << "  [PASS] Rejected identical name on rename\n";
    }
    total++;
    if (!renameItem("beta_empty.txt", "zebra_notes.txt"))
    {
        passed++;
        cout << "  [PASS] Rejected renaming to existing file name\n";
    }
    total++;
    if (renameItem("beta_empty.txt", "midsem_report.tex"))
    {
        passed++;
        printLastMessage();
    }

    Item *report = currentFolder->findChild("midsem_report.tex");
    if (report)
    {
        report->setContent("Complete OOPS Report with Screenshots and Annexures.");
        report->createdDate.setDate(10, 14, 2026);
        (*report)++;
    }
    readFileContent("midsem_report.tex");
    showItemInfo("midsem_report.tex");

    cout << "\n--- [TEST 5] Function-Pointer Sorting (Name, Size, Date, Type, Reverse) ---\n";
    createFolder("LabWork");
    sortCurrentFolder(compareByName);
    total++;
    if (strcmp((*currentFolder)[0]->getName(), "Alpha_code.cpp") == 0)
    {
        passed++;
        cout << "  [PASS] Case-insensitive Name sort placed 'Alpha_code.cpp' first\n";
    }
    else
        cout << "  [FAIL] Name sort order wrong\n";

    sortCurrentFolder(compareBySize);
    total++;
    if ((*currentFolder)[0]->getTotalSize() >= (*currentFolder)[1]->getTotalSize())
    {
        passed++;
        cout << "  [PASS] Size sort placed largest item first\n";
    }
    else
        cout << "  [FAIL] Size sort order wrong\n";

    sortCurrentFolder(compareByDate);
    total++;
    if (!((*currentFolder)[0]->getDate() > (*currentFolder)[1]->getDate()))
    {
        passed++;
        cout << "  [PASS] Date sort placed oldest item first\n";
    }
    else
        cout << "  [FAIL] Date sort order wrong\n";

    sortCurrentFolder(compareByType);
    total++;
    if ((*currentFolder)[0]->getIsFolder())
    {
        passed++;
        cout << "  [PASS] Type sort placed Folder before Files\n";
    }
    else
        cout << "  [FAIL] Type sort order wrong\n";

    sortCurrentFolder(compareByName, true);
    total++;
    if (strcmp((*currentFolder)[0]->getName(), "zebra_notes.txt") == 0)
    {
        passed++;
        cout << "  [PASS] Reverse Name sort placed 'zebra_notes.txt' first\n";
    }
    else
        cout << "  [FAIL] Reverse sort failed\n";

    cout << "\n--- [TEST 6] Cyclic Move/Copy Protection & Deep Copy Independence ---\n";
    openFolder("/");
    total++;
    if (!moveItem("College", "College"))
    {
        passed++;
        cout << "  [PASS] Blocked moving folder into itself\n";
    }
    total++;
    if (!moveItem("College", "College/OOP"))
    {
        passed++;
        cout << "  [PASS] Blocked moving folder into its own subfolder\n";
    }
    total++;
    if (!copyItem("College", "College/OOP"))
    {
        passed++;
        cout << "  [PASS] Blocked copying folder into its own subfolder\n";
    }

    int beforeCopyNodes = Item::getTotalItems();
    total++;
    if (copyItem("College", "Backup"))
    {
        passed++;
        printLastMessage();
    }
    int afterCopyNodes = Item::getTotalItems();
    total++;
    if (afterCopyNodes > beforeCopyNodes)
    {
        passed++;
        cout << "  [PASS] Deep copy allocated " << (afterCopyNodes - beforeCopyNodes) << " new independent nodes\n";
    }
    else
        cout << "  [FAIL] Deep copy node count did not increase\n";

    total++;
    if (deleteItem("College") && resolvePath("/Backup/College/OOP") != nullptr)
    {
        passed++;
        cout << "  [PASS] Deleted original 'College'; deep copy in '/Backup/College/OOP' is intact!\n";
    }
    else
        cout << "  [FAIL] Deep copy was not independent\n";

    total++;
    if (moveItem("Backup", "Personal") && resolvePath("/Personal/Backup/College/OOP") != nullptr)
    {
        passed++;
        cout << "  [PASS] Moved 'Backup' tree into 'Personal'\n";
    }
    else
        cout << "  [FAIL] Move failed\n";
    openFolder("/Personal");
    moveItem("Backup", "..");
    openFolder("/");

    cout << "\n--- [TEST 7] Clipboard (Copy, Cut, Auto-Rename, Delete-Clear) ---\n";
    total++;
    if (!pasteClipboard())
    {
        passed++;
        cout << "  [PASS] Blocked paste when clipboard is empty\n";
    }

    openFolder("/Backup/College/OOP");
    total++;
    if (copyToClipboard("Alpha_code.cpp"))
    {
        passed++;
    }
    total++;
    if (pasteClipboard() && currentFolder->findChild("Alpha_code.cpp_copy") != nullptr)
    {
        passed++;
        cout << "  [PASS] Pasted duplicate as 'Alpha_code.cpp_copy'\n";
    }
    else
        cout << "  [FAIL] Auto-rename on paste 1 failed\n";

    total++;
    if (pasteClipboard() && currentFolder->findChild("Alpha_code.cpp_copy2") != nullptr)
    {
        passed++;
        cout << "  [PASS] Pasted second duplicate as 'Alpha_code.cpp_copy2'\n";
    }
    else
        cout << "  [FAIL] Auto-rename on paste 2 failed\n";

    total++;
    if (copyToClipboard("Alpha_code.cpp_copy2") && deleteItem("Alpha_code.cpp_copy2") && !pasteClipboard())
    {
        passed++;
        cout << "  [PASS] Clipboard automatically cleared when copied item was deleted\n";
    }
    else
        cout << "  [FAIL] Dangling pointer remained in clipboard after delete\n";

    total++;
    if (cutToClipboard("Alpha_code.cpp_copy"))
    {
        passed++;
    }
    openFolder("/Personal");
    total++;
    if (pasteClipboard() && currentFolder->findChild("Alpha_code.cpp_copy") != nullptr)
    {
        passed++;
        cout << "  [PASS] Cut-and-paste moved file into /Personal\n";
    }
    else
        cout << "  [FAIL] Cut-and-paste failed\n";

    cout << "\n--- [TEST 8] Trash, Restore & EmptyTrash ---\n";
    total++;
    if (moveToTrash("Alpha_code.cpp_copy") && currentFolder->findChild("Alpha_code.cpp_copy") == nullptr)
    {
        passed++;
        cout << "  [PASS] Moved file to hidden .Trash\n";
    }
    else
        cout << "  [FAIL] moveToTrash failed\n";

    listTrash();

    total++;
    if (restoreFromTrash("Alpha_code.cpp_copy") && currentFolder->findChild("Alpha_code.cpp_copy") != nullptr)
    {
        passed++;
        cout << "  [PASS] Restored file from .Trash back to /Personal\n";
    }
    else
        cout << "  [FAIL] restoreFromTrash failed\n";

    moveToTrash("Alpha_code.cpp_copy");
    emptyTrash();
    total++;
    if (trashFolder->getChildCount() == 0)
    {
        passed++;
        cout << "  [PASS] emptyTrash permanently deleted trashed items and freed RAM\n";
    }
    else
        cout << "  [FAIL] Trash not empty after emptyTrash\n";

    cout << "\n--- [TEST 9] Exact & Partial Substring Search ---\n";
    searchItem("zebra");
    total++;
    if (lastMessageType == MSG_SUCCESS)
    {
        passed++;
        cout << "  [PASS] Partial search found 'zebra_notes.txt'\n";
    }
    searchItem("non_existent_file_xyz");
    total++;
    if (lastMessageType == MSG_INFO)
    {
        passed++;
        cout << "  [PASS] Search cleanly handled no matches\n";
    }

    openFolder("/");
    showFullTree();

    cout << "\n=========================================================\n";
    cout << "   TEST SUMMARY: " << passed << " / " << total << " CHECKS PASSED!\n";
    cout << "   Active Nodes Currently in RAM: " << Item::getTotalItems() << "\n";
    cout << "=========================================================\n";

    setMessage(string("Automated Test Suite complete: ") + intToString(passed) + "/" + intToString(total) + " passed!", MSG_SUCCESS);
}

void Explorer::renderDashboard() const
{
    cout << "\033[2J\033[H";

    printLine("  ╔", "═", "╗", 73);
    string leftTitle = "  VIRTUAL FILE EXPLORER";
    string rightRam = string("Items in RAM: ") + intToString(Item::getTotalItems()) + "  ";
    cout << "  ║" << color(CLR_HEADER);
    cout << leftTitle;
    int midSpaces = 73 - (int)leftTitle.length() - (int)rightRam.length();
    for (int i = 0; i < midSpaces; i++)
        cout << " ";
    cout << rightRam << color(CLR_RESET) << "║\n";
    printLine("  ╚", "═", "╝", 73);

    string locText = string("Location:  ") + getBreadcrumbPath();
    string sortText = string("Sorted by: ") + currentSortLabel;
    cout << "    ";
    printPadded(locText, 73 - (int)sortText.length());
    cout << sortText << "\n\n";

    cout << "  ┌────┬──────────────────────┬────────┬───────────┬──────────────────┬─────┐\n";
    cout << "  │ #  │ Name                 │ Type   │ Size      │ Created          │ Ver │\n";
    cout << "  ├────┼──────────────────────┼────────┼───────────┼──────────────────┼─────┤\n";

    int visibleCount = getVisibleCount();
    if (visibleCount == 0)
    {
        cout << "  │ " << color(CLR_HINT);
        printPadded("(Empty folder - press N for new folder or F for new file)", 71);
        cout << color(CLR_RESET) << " │\n";
    }
    else
    {
        int idx = 1;
        for (int i = 0; i < currentFolder->getChildCount(); i++)
        {
            Item *child = (*currentFolder)[i];
            if (strcmp(child->getName(), ".Trash") == 0)
                continue;

            string rowColor = "";
            if (child == selectedItem)
            {
                rowColor = color(CLR_SELECT);
            }
            else if (child->getIsFolder())
            {
                rowColor = color(CLR_FOLDER);
            }

            string numStr = intToString(idx++);
            string tag = child->getIsFolder() ? "[D] " : "[F] ";
            string nameStr = tag + child->getName();
            string typeStr = child->getIsFolder() ? "Folder" : "File";
            string sizeStr = intToString(child->getTotalSize()) + " B";
            string verStr = string("v") + intToString(child->version);

            cout << "  │" << rowColor << " ";
            printPadded(numStr, 2);
            cout << " │ ";
            printPadded(nameStr, 20);
            cout << " │ ";
            printPadded(typeStr, 6);
            cout << " │ ";
            printPadded(sizeStr, 9);
            cout << " │ ";
            cout << child->createdDate;
            cout << " │ ";
            printPadded(verStr, 3);
            cout << " " << color(CLR_RESET) << "│\n";
        }
    }
    cout << "  └────┴──────────────────────┴────────┴───────────┴──────────────────┴─────┘\n\n";

    if (selectedItem != nullptr)
    {
        cout << "    Selected: " << selectedItem->getName()
             << "  (" << (selectedItem->getIsFolder() ? "Folder" : "File")
             << ", " << selectedItem->getTotalSize() << " B, v" << selectedItem->version << ")\n";
        cout << "    Preview : " << getPreviewText(selectedItem) << "\n\n";
    }
    else
    {
        cout << "    Selected: " << color(CLR_HINT)
             << "None (Type # to open folder / select file, or *# to select folder)"
             << color(CLR_RESET) << "\n";
        cout << "    Preview : " << color(CLR_HINT) << "-" << color(CLR_RESET) << "\n\n";
    }

    if (lastMessageType != MSG_NONE && lastMessage.length() > 0)
    {
        string msgClr = color(CLR_INFO);
        string icon = "! ";
        if (lastMessageType == MSG_SUCCESS)
        {
            msgClr = color(CLR_SUCCESS);
            icon = "✔ ";
        }
        else if (lastMessageType == MSG_ERROR)
        {
            msgClr = color(CLR_ERROR);
            icon = "✖ ";
        }
        cout << "    " << msgClr << icon << lastMessage << color(CLR_RESET) << "\n\n";
    }

    cout << "    ACTIONS\n";
    cout << "    [1-9..] Select / open item    [B] Back        [N] New folder   [F] New file\n";
    cout << "    [E] Edit file    [R] Rename   [D] Delete      [C] Copy   [X] Move   [V] Paste\n";
    cout << "    [S] Sort         [/text] Search               [T] Tree   [I] Info   [K] Colors on/off\n";
    cout << "    [H] Help         [Q] Quit     " << color(CLR_HINT) << "(or type 'terminal' / 'demo')" << color(CLR_RESET) << "\n\n";
}

void Explorer::handleDashboardCopyOrMove(bool isMove)
{
    if (selectedItem == nullptr)
    {
        setMessage("Select an item first by typing its number", MSG_INFO);
        return;
    }

    if (isMove)
    {
        cutToClipboard(selectedItem->getName());
    }
    else
    {
        copyToClipboard(selectedItem->getName());
    }

    cout << "\n    ┌── " << (isMove ? "MOVE" : "COPY") << " DESTINATION ───────────────────────────────────────────┐\n";
    cout << "    │  0. Keep in Clipboard (to Paste later with [V])                     │\n";
    cout << "    │  1. .. (parent folder)                                              │\n";

    int folderOption = 2;
    for (int i = 0; i < currentFolder->getChildCount(); i++)
    {
        Item *child = (*currentFolder)[i];
        if (strcmp(child->getName(), ".Trash") == 0)
            continue;
        if (child->getIsFolder() && child != selectedItem)
        {
            cout << "    │  " << folderOption << ". ";
            printPadded(child->getName(), 64);
            cout << "│\n";
            folderOption++;
        }
    }
    cout << "    └─────────────────────────────────────────────────────────────────────┘\n";
    cout << "    Destination number > ";

    string input;
    if (!getline(cin >> ws, input))
        return;
    if (!isAllDigits(input))
    {
        setMessage("Invalid destination number!", MSG_ERROR);
        return;
    }

    int choice = 0;
    for (int i = 0; i < (int)input.length(); i++)
    {
        choice = choice * 10 + (input[i] - '0');
    }

    if (choice == 0)
    {
        return;
    }

    string destName = "";
    if (choice == 1)
    {
        destName = "..";
    }
    else
    {
        int curr = 2;
        for (int i = 0; i < currentFolder->getChildCount(); i++)
        {
            Item *child = (*currentFolder)[i];
            if (strcmp(child->getName(), ".Trash") == 0)
                continue;
            if (child->getIsFolder() && child != selectedItem)
            {
                if (curr == choice)
                {
                    destName = child->getName();
                    break;
                }
                curr++;
            }
        }
    }

    if (destName == "")
    {
        setMessage("Destination number out of range!", MSG_ERROR);
        return;
    }

    if (isMove)
    {
        moveItem(selectedItem->getName(), destName);
    }
    else
    {
        copyItem(selectedItem->getName(), destName);
    }
}

void Explorer::handleDashboardSort()
{
    cout << "\n    ┌── SORT FOLDER ──────────────────────────────────────────────────────┐\n";
    cout << "    │  1. Name (A-Z)   2. Size (High-Low)   3. Date (Old-New)   4. Type   │\n";
    cout << "    └─────────────────────────────────────────────────────────────────────┘\n";
    cout << "    Sort option (1-4) > ";

    string input;
    if (!getline(cin >> ws, input))
        return;

    if (input == "1")
    {
        sortCurrentFolder(compareByName);
    }
    else if (input == "2")
    {
        sortCurrentFolder(compareBySize);
    }
    else if (input == "3")
    {
        sortCurrentFolder(compareByDate);
    }
    else if (input == "4")
    {
        sortCurrentFolder(compareByType);
    }
    else
    {
        setMessage("Invalid sort option! Choose 1, 2, 3, or 4.", MSG_ERROR);
    }
}

void Explorer::showDashboardHelp() const
{
    cout << "\033[2J\033[H";
    cout << "\n========== DASHBOARD HELP & SHORTCUTS ==========\n";
    cout << "  <number>     : Open folder immediately, or select a file for preview\n";
    cout << "  *<number>    : Select a folder (without opening it) for R/D/C/X/I\n";
    cout << "  B            : Go back to parent folder\n";
    cout << "  N            : Create a new folder in current location\n";
    cout << "  F            : Create a new file with text content\n";
    cout << "  E            : Edit content of the currently selected file\n";
    cout << "  R            : Rename the currently selected item\n";
    cout << "  D            : Delete the currently selected item (with Y/N prompt)\n";
    cout << "  C            : Copy selected item to a destination folder or clipboard\n";
    cout << "  X            : Move/Cut selected item to a destination folder or clipboard\n";
    cout << "  V            : Paste item from clipboard into current folder\n";
    cout << "  S            : Sort current folder by Name, Size, Date, or Type\n";
    cout << "  /text        : Search for 'text' across all folders and files\n";
    cout << "  T            : View complete directory tree\n";
    cout << "  I            : View full metadata for the selected item\n";
    cout << "  K            : Toggle ANSI terminal colors on/off\n";
    cout << "  terminal     : Switch to classic command-line shell\n";
    cout << "  demo         : Run automated professor viva demonstration\n";
    cout << "  Q            : Quit and free all dynamically allocated memory\n";
    cout << "================================================\n";
    waitForEnter();
}

void Explorer::startDashboard()
{
    string line;
    while (true)
    {
        renderDashboard();
        cout << "    Command > ";

        if (!getline(cin >> ws, line))
        {
            break;
        }
        if (line.length() == 0)
        {
            continue;
        }

        if (line == "terminal")
        {
            startShell();
            setMessage("Returned from terminal shell to dashboard.", MSG_INFO);
            continue;
        }
        if (line == "demo")
        {
            cout << "\033[2J\033[H";
            runAutomatedDemo();
            waitForEnter();
            continue;
        }
        if (line == "menu")
        {
            startMenu();
            continue;
        }

        if (line.length() > 1 && line[0] == '*' && isAllDigits(line.substr(1)))
        {
            int num = 0;
            for (int i = 1; i < (int)line.length(); i++)
            {
                num = num * 10 + (line[i] - '0');
            }
            Item *target = getVisibleItem(num);
            if (target == nullptr)
            {
                setMessage("Item number out of range!", MSG_ERROR);
            }
            else
            {
                selectedItem = target;
                setMessage(string("Selected '") + target->getName() + "'.", MSG_INFO);
            }
            continue;
        }

        if (isAllDigits(line))
        {
            int num = 0;
            for (int i = 0; i < (int)line.length(); i++)
            {
                num = num * 10 + (line[i] - '0');
            }
            Item *target = getVisibleItem(num);
            if (target == nullptr)
            {
                setMessage("Item number out of range!", MSG_ERROR);
            }
            else if (target->getIsFolder())
            {
                currentFolder = target;
                selectedItem = nullptr;
                setMessage(string("Opened folder '") + target->getName() + "'.", MSG_INFO);
            }
            else
            {
                selectedItem = target;
                setMessage(string("Selected file '") + target->getName() + "'.", MSG_INFO);
            }
            continue;
        }

        if (line[0] == '/')
        {
            string query = line.substr(1);
            if (query.length() == 0)
            {
                cout << "\n    ┌── SEARCH ───────────────────────────────────────────────────────────┐\n";
                cout << "    Search text > ";
                if (!getline(cin >> ws, query))
                    break;
            }
            cout << "\033[2J\033[H";
            searchItem(query);
            waitForEnter();
            continue;
        }

        char cmd = tolower((unsigned char)line[0]);
        if (cmd == 'q')
        {
            cout << "\n  [Info] Exiting Virtual File Explorer. Freeing all memory...\n";
            break;
        }
        else if (cmd == 'b')
        {
            goBack();
        }
        else if (cmd == 'n')
        {
            cout << "\n    ┌── NEW FOLDER ───────────────────────────────────────────────────────┐\n";
            cout << "    New folder name > ";
            string folderName;
            if (!getline(cin >> ws, folderName))
                break;
            createFolder(folderName);
        }
        else if (cmd == 'f')
        {
            cout << "\n    ┌── NEW FILE ─────────────────────────────────────────────────────────┐\n";
            cout << "    New file name > ";
            string fileName, content;
            if (!getline(cin >> ws, fileName))
                break;
            cout << "    Initial text content (press Enter for empty) > ";
            if (!getline(cin, content))
                break;
            createFile(fileName, content);
        }
        else if (cmd == 'e')
        {
            if (selectedItem == nullptr)
            {
                setMessage("Select an item first by typing its number", MSG_INFO);
            }
            else if (selectedItem->getIsFolder())
            {
                setMessage(string("'") + selectedItem->getName() + "' is a folder, not a file!", MSG_ERROR);
            }
            else
            {
                cout << "\n    ┌── EDIT FILE: " << selectedItem->getName() << "\n    ";
                editFileContent(selectedItem->getName());
            }
        }
        else if (cmd == 'r')
        {
            if (selectedItem == nullptr)
            {
                setMessage("Select an item first by typing its number", MSG_INFO);
            }
            else
            {
                cout << "\n    ┌── RENAME ITEM ──────────────────────────────────────────────────────┐\n";
                cout << "    New name for '" << selectedItem->getName() << "' > ";
                string newName;
                if (!getline(cin >> ws, newName))
                    break;
                renameItem(selectedItem->getName(), newName);
            }
        }
        else if (cmd == 'd')
        {
            if (selectedItem == nullptr)
            {
                setMessage("Select an item first by typing its number", MSG_INFO);
            }
            else
            {
                string targetName = selectedItem->getName();
                cout << "\n    ┌── CONFIRM DELETE ───────────────────────────────────────────────────┐\n";
                cout << "    Delete '" << targetName << "'? (Y/N) > ";
                string confirm;
                if (!getline(cin >> ws, confirm))
                    break;
                if (confirm.length() > 0 && tolower((unsigned char)confirm[0]) == 'y')
                {
                    deleteItem(targetName);
                }
                else
                {
                    setMessage("Delete cancelled.", MSG_INFO);
                }
            }
        }
        else if (cmd == 'c')
        {
            handleDashboardCopyOrMove(false);
        }
        else if (cmd == 'x')
        {
            handleDashboardCopyOrMove(true);
        }
        else if (cmd == 'v')
        {
            pasteClipboard();
        }
        else if (cmd == 's')
        {
            handleDashboardSort();
        }
        else if (cmd == 't')
        {
            cout << "\033[2J\033[H";
            showFullTree();
            waitForEnter();
        }
        else if (cmd == 'i')
        {
            if (selectedItem == nullptr)
            {
                setMessage("Select an item first by typing its number", MSG_INFO);
            }
            else
            {
                cout << "\033[2J\033[H";
                showItemInfo(selectedItem->getName());
                waitForEnter();
            }
        }
        else if (cmd == 'k')
        {
            useColors = !useColors;
            setMessage(useColors ? "Colors turned ON." : "Colors turned OFF.", MSG_INFO);
        }
        else if (cmd == 'h')
        {
            showDashboardHelp();
        }
        else
        {
            setMessage(string("Unknown command '") + line + "'. Press H for help.", MSG_ERROR);
        }

        if (cin.eof())
            break;
    }
}

void Explorer::startMenu()
{
    int choice = 0;
    while (true)
    {
        cout << "\n=========================================================\n";
        cout << " VIRTUAL FILE EXPLORER | Path: " << getCurrentPath()
             << " | Active Nodes in RAM: " << Item::getTotalItems() << "\n";
        cout << "=========================================================\n";
        cout << " 1. Create Folder (mkdir)      7. Copy Item (Deep Copy)\n";
        cout << " 2. Create File (touch)        8. Move Item (mv)\n";
        cout << " 3. Open Folder (cd <path>)    9. Delete Item (rm)\n";
        cout << " 4. Go Back (cd ..)           10. Sort Folder (Fn Pointer)\n";
        cout << " 5. List Folder (ls)          11. RUN AUTOMATED DEMO\n";
        cout << " 6. Show Full Tree & Search   12. Edit/Rename File\n";
        cout << " 0. Exit Menu\n";
        cout << "---------------------------------------------------------\n";
        cout << "Enter choice: ";

        if (!(cin >> choice))
        {
            if (cin.eof())
                break;
            cin.clear();
            cin.ignore(1000, '\n');
            cout << "  [Error] Invalid input! Please enter a number.\n";
            continue;
        }

        if (choice == 0)
            break;

        string name1, name2;
        switch (choice)
        {
        case 1:
            cout << "Enter new folder name: ";
            cin >> name1;
            createFolder(name1);
            printLastMessage();
            break;
        case 2:
            cout << "Enter new file name: ";
            cin >> name1;
            cout << "Enter initial text content (press Enter for empty file): ";
            cin.ignore(1000, '\n');
            getline(cin, name2);
            createFile(name1, name2);
            printLastMessage();
            break;
        case 3:
            cout << "Enter folder path to open: ";
            cin >> name1;
            if (!openFolder(name1))
                printLastMessage();
            break;
        case 4:
            if (!goBack())
                printLastMessage();
            break;
        case 5:
            listCurrentFolder();
            break;
        case 6:
            showFullTree();
            cout << "Enter item name to search (or 'skip'): ";
            cin >> name1;
            if (name1 != "skip")
                searchItem(name1);
            break;
        case 7:
            cout << "Enter item name to copy: ";
            cin >> name1;
            cout << "Enter destination folder path (or '..' for parent): ";
            cin >> name2;
            copyItem(name1, name2);
            printLastMessage();
            break;
        case 8:
            cout << "Enter item name to move: ";
            cin >> name1;
            cout << "Enter destination folder path (or '..' for parent): ";
            cin >> name2;
            moveItem(name1, name2);
            printLastMessage();
            break;
        case 9:
            cout << "Enter item name to delete: ";
            cin >> name1;
            deleteItem(name1);
            printLastMessage();
            break;
        case 10:
        {
            int sChoice = 0;
            char rev = 'n';
            cout << "Sort by: 1. Name (A-Z) | 2. Size (High-Low) | 3. Date (Old-New) | 4. Type (Folders first): ";
            if (!(cin >> sChoice))
            {
                cin.clear();
                cin.ignore(1000, '\n');
                cout << "  [Error] Invalid sort choice!\n";
                break;
            }
            bool (*cmp)(const Item *, const Item *) = nullptr;
            if (sChoice == 1)
                cmp = compareByName;
            else if (sChoice == 2)
                cmp = compareBySize;
            else if (sChoice == 3)
                cmp = compareByDate;
            else if (sChoice == 4)
                cmp = compareByType;
            if (cmp == nullptr)
            {
                cout << "  [Error] Invalid sort choice!\n";
                break;
            }
            cout << "Reverse the order? (y/n): ";
            cin >> rev;
            sortCurrentFolder(cmp, rev == 'y' || rev == 'Y');
            printLastMessage();
            listCurrentFolder();
            break;
        }
        case 11:
            runAutomatedDemo();
            break;
        case 12:
        {
            int sub = 0;
            cout << "1. Edit File Content (>> operator) | 2. Rename Item: ";
            if (!(cin >> sub))
            {
                cin.clear();
                cin.ignore(1000, '\n');
                cout << "  [Error] Invalid option!\n";
                break;
            }
            if (sub == 1)
            {
                cout << "Enter file name: ";
                cin >> name1;
                editFileContent(name1);
                printLastMessage();
            }
            else if (sub == 2)
            {
                cout << "Enter current name and new name: ";
                cin >> name1 >> name2;
                renameItem(name1, name2);
                printLastMessage();
            }
            else
            {
                cout << "  [Error] Invalid option!\n";
            }
            break;
        }
        default:
            cout << "  [Error] Invalid choice!\n";
        }

        if (cin.eof())
            break;
    }
}

void Explorer::startShell()
{
    cout << "=========================================================\n";
    cout << " VIRTUAL FILE EXPLORER SHELL (Type 'help' for commands)\n";
    cout << "=========================================================\n";

    string cmd, arg1, arg2;
    while (true)
    {
        cout << "\nuser@virtual-mac " << getCurrentPath() << " % ";
        if (!(cin >> cmd))
        {
            break;
        }

        if (cmd == "exit")
        {
            cout << "  [Info] Exiting shell...\n";
            break;
        }
        else if (cmd == "ls")
        {
            while (cin.peek() == ' ' || cin.peek() == '\t')
            {
                cin.get();
            }
            if (cin.peek() == '\n' || cin.peek() == EOF)
            {
                listCurrentFolder(false);
            }
            else
            {
                cin >> arg1;
                if (arg1 == "-l")
                {
                    listCurrentFolder(true);
                }
                else
                {
                    cout << "  [Error] Unknown option '" << arg1 << "'. Use 'ls' or 'ls -l'.\n";
                }
            }
        }
        else if (cmd == "cd")
        {
            if (!(cin >> arg1))
                break;
            if (!openFolder(arg1))
                printLastMessage();
        }
        else if (cmd == "pwd")
        {
            cout << "  " << getCurrentPath() << "\n";
        }
        else if (cmd == "mkdir")
        {
            if (!(cin >> arg1))
                break;
            createFolder(arg1);
            printLastMessage();
        }
        else if (cmd == "touch")
        {
            if (!(cin >> arg1))
                break;
            cout << "Enter initial text content (press Enter for empty): ";
            cin.ignore(1000, '\n');
            getline(cin, arg2);
            createFile(arg1, arg2);
            printLastMessage();
        }
        else if (cmd == "cat")
        {
            if (!(cin >> arg1))
                break;
            readFileContent(arg1);
        }
        else if (cmd == "edit")
        {
            if (!(cin >> arg1))
                break;
            editFileContent(arg1);
            printLastMessage();
        }
        else if (cmd == "copy")
        {
            if (!(cin >> arg1))
                break;
            copyToClipboard(arg1);
            printLastMessage();
        }
        else if (cmd == "cut")
        {
            if (!(cin >> arg1))
                break;
            cutToClipboard(arg1);
            printLastMessage();
        }
        else if (cmd == "paste")
        {
            pasteClipboard();
            printLastMessage();
        }
        else if (cmd == "rm")
        {
            if (!(cin >> arg1))
                break;
            moveToTrash(arg1);
            printLastMessage();
        }
        else if (cmd == "trash")
        {
            listTrash();
        }
        else if (cmd == "restore")
        {
            if (!(cin >> arg1))
                break;
            restoreFromTrash(arg1);
            printLastMessage();
        }
        else if (cmd == "emptytrash")
        {
            emptyTrash();
        }
        else if (cmd == "cp")
        {
            if (!(cin >> arg1 >> arg2))
                break;
            copyItem(arg1, arg2);
            printLastMessage();
        }
        else if (cmd == "mv")
        {
            if (!(cin >> arg1 >> arg2))
                break;
            moveItem(arg1, arg2);
            printLastMessage();
        }
        else if (cmd == "rename")
        {
            if (!(cin >> arg1 >> arg2))
                break;
            renameItem(arg1, arg2);
            printLastMessage();
        }
        else if (cmd == "find")
        {
            if (!(cin >> arg1))
                break;
            searchItem(arg1);
        }
        else if (cmd == "tree")
        {
            showFullTree();
        }
        else if (cmd == "sort")
        {
            if (!(cin >> arg1))
                break;
            if (arg1 == "name")
            {
                sortCurrentFolder(compareByName);
                printLastMessage();
            }
            else if (arg1 == "size")
            {
                sortCurrentFolder(compareBySize);
                printLastMessage();
            }
            else if (arg1 == "date")
            {
                sortCurrentFolder(compareByDate);
                printLastMessage();
            }
            else if (arg1 == "type")
            {
                sortCurrentFolder(compareByType);
                printLastMessage();
            }
            else
            {
                cout << "  [Error] Invalid sort option '" << arg1 << "'. Use 'name', 'size', 'date', or 'type'.\n";
            }
        }
        else if (cmd == "info")
        {
            if (!(cin >> arg1))
                break;
            showItemInfo(arg1);
        }
        else if (cmd == "clear")
        {
            for (int i = 0; i < 30; i++)
            {
                cout << "\n";
            }
        }
        else if (cmd == "help")
        {
            showHelp();
        }
        else if (cmd == "demo")
        {
            runAutomatedDemo();
        }
        else if (cmd == "menu")
        {
            startMenu();
        }
        else
        {
            cout << "  [Error] Unknown command '" << cmd << "'. Type 'help' to see valid commands.\n";
            cin.ignore(1000, '\n');
        }

        if (cin.eof())
            break;
    }
}

int main()
{
    Explorer app;
    app.startDashboard();
    return 0;
}