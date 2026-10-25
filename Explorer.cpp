#include "Explorer.h"

// Comparison Functions for Function Pointers
bool compareByName(const Item* a, const Item* b) {
    return a->getName() > b->getName(); // Alphabetical A-Z
}

bool compareBySize(const Item* a, const Item* b) {
    return a->getTotalSize() < b->getTotalSize(); // Largest to Smallest
}

bool compareByDate(const Item* a, const Item* b) {
    return a->getDate().toNumericValue() > b->getDate().toNumericValue(); // Oldest to Newest
}

Explorer::Explorer() {
    root = new Item("/", true, "", nullptr);
    currentFolder = root;
}

Explorer::~Explorer() {
    delete root; // Triggers recursive deletion of the entire file system tree!
}

string Explorer::getCurrentPath() const {
    if (currentFolder == root) return "/";
    string path = "";
    const Item* temp = currentFolder;
    while (temp != nullptr && temp != root) {
        path = "/" + temp->getName() + path;
        temp = temp->getParent();
    }
    return path;
}

bool Explorer::createFolder(const string& folderName) {
    if (currentFolder->findChild(folderName) != nullptr) {
        cout << "  [Error] Item '" << folderName << "' already exists in this folder!\n";
        return false;
    }
    Item* newFolder = new Item(folderName, true);
    *currentFolder += newFolder; // Using overloaded += operator
    cout << "  [Success] Folder '" << folderName << "' created.\n";
    return true;
}

// Overloaded createFile #1 (Empty file)
bool Explorer::createFile(const string& fileName) {
    return createFile(fileName, "");
}

// Overloaded createFile #2 (File with content)
bool Explorer::createFile(const string& fileName, const string& initialContent) {
    if (currentFolder->findChild(fileName) != nullptr) {
        cout << "  [Error] Item '" << fileName << "' already exists in this folder!\n";
        return false;
    }
    Item* newFile = new Item(fileName, false, initialContent);
    *currentFolder += newFile;
    cout << "  [Success] File '" << fileName << "' (" << newFile->getTotalSize() << " B) created.\n";
    return true;
}

bool Explorer::openFolder(const string& folderName) {
    Item* target = currentFolder->findChild(folderName);
    if (target == nullptr) {
        cout << "  [Error] Folder '" << folderName << "' not found.\n";
        return false;
    }
    if (!target->getIsFolder()) {
        cout << "  [Error] '" << folderName << "' is a file, not a folder!\n";
        return false;
    }
    currentFolder = target;
    return true;
}

bool Explorer::goBack() {
    if (currentFolder == root) {
        cout << "  [Info] Already at root directory '/'.\n";
        return false;
    }
    currentFolder = currentFolder->getParent();
    return true;
}

void Explorer::listCurrentFolder() const {
    cout << "\n--- Contents of " << getCurrentPath() << " ---\n";
    if (currentFolder->getChildCount() == 0) {
        cout << "  (Empty Folder)\n";
        return;
    }
    for (int i = 0; i < currentFolder->getChildCount(); i++) {
        cout << "  " << i + 1 << ". " << *((*currentFolder)[i]) << "\n";
    }
}

void Explorer::printTreeRecursive(const Item* node, string prefix, bool isLast) const {
    cout << prefix << (isLast ? "└── " : "├── ")
         << node->getName() << (node->getIsFolder() ? "/" : "")
         << " (" << node->getTotalSize() << " B, v" << node->getVersion() << ")\n";

    string nextPrefix = prefix + (isLast ? "    " : "│   ");
    for (int i = 0; i < node->getChildCount(); i++) {
        bool lastChild = (i == node->getChildCount() - 1);
        printTreeRecursive((*node)[i], nextPrefix, lastChild);
    }
}

void Explorer::showFullTree() const {
    cout << "\n========== COMPLETE DIRECTORY TREE ==========\n";
    cout << "/ (Total Size: " << root->getTotalSize() << " B)\n";
    for (int i = 0; i < root->getChildCount(); i++) {
        printTreeRecursive((*root)[i], "", i == root->getChildCount() - 1);
    }
    cout << "=============================================\n";
}

void Explorer::searchRecursive(const Item* node, const string& target, string currentPath, int& foundCount) const {
    for (int i = 0; i < node->getChildCount(); i++) {
        Item* child = (*node)[i];
        string childPath = (currentPath == "/") ? ("/" + child->getName()) : (currentPath + "/" + child->getName());
        if (child->getName() == target) {
            cout << "  Found: " << childPath << " -> " << *child << "\n";
            foundCount++;
        }
        if (child->getIsFolder()) {
            searchRecursive(child, target, childPath, foundCount);
        }
    }
}

void Explorer::searchItem(const string& name) const {
    cout << "\nSearching for '" << name << "' across file system...\n";
    int count = 0;
    searchRecursive(root, name, "/", count);
    if (count == 0) cout << "  No matching items found.\n";
}

bool Explorer::renameItem(const string& oldName, const string& newName) {
    Item* target = currentFolder->findChild(oldName);
    if (!target) {
        cout << "  [Error] '" << oldName << "' not found.\n";
        return false;
    }
    if (currentFolder->findChild(newName) != nullptr) {
        cout << "  [Error] '" << newName << "' already exists!\n";
        return false;
    }
    target->setName(newName);
    ++(*target); // Prefix increment version on rename
    cout << "  [Success] Renamed to '" << newName << "' (v" << target->getVersion() << ").\n";
    return true;
}

bool Explorer::copyItem(const string& sourceName, const string& destFolderName) {
    Item* src = currentFolder->findChild(sourceName);
    Item* dest = (destFolderName == "..") ? currentFolder->getParent() : currentFolder->findChild(destFolderName);

    if (!src) {
        cout << "  [Error] Source '" << sourceName << "' not found.\n";
        return false;
    }
    if (!dest || !dest->getIsFolder()) {
        cout << "  [Error] Destination folder '" << destFolderName << "' is invalid.\n";
        return false;
    }

    // [Syllabus Mod 7] Calls Recursive Deep Copy Constructor!
    Item* deepClone = new Item(*src);
    if (!dest->addChild(deepClone)) {
        delete deepClone;
        cout << "  [Error] Duplicate name in destination folder!\n";
        return false;
    }
    cout << "  [Success] Deep copied '" << sourceName << "' as '" << deepClone->getName()
         << "' into '" << dest->getName() << "'.\n";
    return true;
}

bool Explorer::moveItem(const string& sourceName, const string& destFolderName) {
    Item* dest = (destFolderName == "..") ? currentFolder->getParent() : currentFolder->findChild(destFolderName);
    if (!dest || !dest->getIsFolder()) {
        cout << "  [Error] Destination folder '" << destFolderName << "' not found.\n";
        return false;
    }
    if (dest->findChild(sourceName) != nullptr) {
        cout << "  [Error] Item with same name already exists in destination!\n";
        return false;
    }
    Item* movingItem = currentFolder->detachChild(sourceName);
    if (!movingItem) {
        cout << "  [Error] Source item '" << sourceName << "' not found.\n";
        return false;
    }
    dest->addChild(movingItem);
    cout << "  [Success] Moved '" << sourceName << "' into '" << dest->getName() << "'.\n";
    return true;
}

bool Explorer::deleteItem(const string& name) {
    int beforeNodes = Item::getActiveItemsCount();
    if (currentFolder->deleteChild(name)) {
        int afterNodes = Item::getActiveItemsCount();
        cout << "  [Success] Deleted '" << name << "'. Memory freed for "
             << (beforeNodes - afterNodes) << " node(s).\n";
        return true;
    }
    cout << "  [Error] Item '" << name << "' not found.\n";
    return false;
}

void Explorer::editFileContent(const string& fileName) {
    Item* target = currentFolder->findChild(fileName);
    if (!target) {
        cout << "  [Error] File not found.\n";
        return;
    }
    cin >> *target; // Uses overloaded >> operator!
    cout << "  [Success] Updated file: " << *target << "\n";
}

// [Syllabus Mod 2 & 10] Bubble Sort using Function Pointers
void Explorer::sortCurrentFolder(bool (*cmp)(const Item*, const Item*)) {
    int n = currentFolder->children.size();
    for (int i = 0; i < n - 1; i++) {
        for (int j = 0; j < n - i - 1; j++) {
            if (cmp(currentFolder->children[j], currentFolder->children[j + 1])) {
                swapElements(currentFolder->children[j], currentFolder->children[j + 1]);
            }
        }
    }
    cout << "  [Success] Folder contents sorted using Function Pointer!\n";
}

// Automated Showcase for Screenshots & Professor Viva
void Explorer::runAutomatedDemo() {
    cout << "\n>>> RUNNING AUTOMATED PROFESSOR DEMO <<<\n";
    createFolder("College");
    createFolder("Personal");
    createFolder("Backup");

    openFolder("College");
    createFolder("OOP");
    openFolder("OOP");

    // Demonstrate Function Overloading & Cascaded Calls (*this)
    createFile("zebra_notes.txt", "Polymorphism and Inheritance come after midsem.");
    createFile("alpha_code.cpp", "#include <iostream>\nint main() { return 0; }");
    createFile("midsem_report.tex");

    Item* report = currentFolder->findChild("midsem_report.tex");
    if (report) {
        // Cascaded Member Function Calls + Conversion Constructor (20261014)
        report->setContent("Complete OOPS Report with Screenshots and Annexures.")
              .setDate(DateTime(20261014));
        (*report)++; // Postfix operator++
    }

    cout << "\n--- 1. Before Sorting in /College/OOP ---";
    listCurrentFolder();

    cout << "\n--- 2. Sorting by Name (A-Z) using Function Pointer ---\n";
    sortCurrentFolder(compareByName);
    listCurrentFolder();

    cout << "\n--- 3. Sorting by Size (Largest First) using Function Pointer ---\n";
    sortCurrentFolder(compareBySize);
    listCurrentFolder();

    goBack(); // Back to /College
    goBack(); // Back to /

    cout << "\n--- 4. Testing Deep Copy of Nested Folder ('College' -> 'Backup') ---\n";
    cout << "  Active Memory Nodes BEFORE Deep Copy: " << Item::getActiveItemsCount() << "\n";
    copyItem("College", "Backup");
    cout << "  Active Memory Nodes AFTER Deep Copy:  " << Item::getActiveItemsCount() << "\n";

    showFullTree();

    cout << "\n--- 5. Deleting Original 'College' Folder to Prove Deep Copy Independence ---\n";
    deleteItem("College");
    showFullTree();
    cout << "  Notice how 'College_copy' inside 'Backup/' is 100% intact!\n";
    cout << ">>> END OF AUTOMATED DEMO <<<\n";
}

void Explorer::startMenu() {
    int choice = 0;
    while (true) {
        cout << "\n=========================================================\n";
        cout << " VIRTUAL FILE EXPLORER | Path: " << getCurrentPath()
             << " | Active Nodes in RAM: " << Item::getActiveItemsCount() << "\n";
        cout << "=========================================================\n";
        cout << " 1. Create Folder (mkdir)      7. Copy Item (Deep Copy)\n";
        cout << " 2. Create File (touch)        8. Move Item (mv)\n";
        cout << " 3. Open Folder (cd)           9. Delete Item (rm)\n";
        cout << " 4. Go Back (cd ..)           10. Sort Folder (Fn Pointer)\n";
        cout << " 5. List Folder (ls)          11. RUN AUTOMATED DEMO\n";
        cout << " 6. Show Full Tree & Search   12. Edit/Rename File\n";
        cout << " 0. Exit & Free All Memory\n";
        cout << "---------------------------------------------------------\n";
        cout << "Enter choice: ";

        if (!(cin >> choice)) {
            cin.clear();
            cin.ignore(1000, '\n');
            cout << "Invalid input! Please enter a number.\n";
            continue;
        }

        if (choice == 0) break;

        string name1, name2;
        switch (choice) {
            case 1:
                cout << "Enter new folder name: ";
                cin >> name1;
                createFolder(name1);
                break;
            case 2:
                cout << "Enter new file name: ";
                cin >> name1;
                cout << "Enter initial text content: ";
                getline(cin >> ws, name2);
                createFile(name1, name2);
                break;
            case 3:
                cout << "Enter folder name to open: ";
                cin >> name1;
                openFolder(name1);
                break;
            case 4:
                goBack();
                break;
            case 5:
                listCurrentFolder();
                break;
            case 6:
                showFullTree();
                cout << "Enter item name to search (or 'skip'): ";
                cin >> name1;
                if (name1 != "skip") searchItem(name1);
                break;
            case 7:
                cout << "Enter item name to copy: ";
                cin >> name1;
                cout << "Enter destination folder name (or '..' for parent): ";
                cin >> name2;
                copyItem(name1, name2);
                break;
            case 8:
                cout << "Enter item name to move: ";
                cin >> name1;
                cout << "Enter destination folder name (or '..' for parent): ";
                cin >> name2;
                moveItem(name1, name2);
                break;
            case 9:
                cout << "Enter item name to delete: ";
                cin >> name1;
                deleteItem(name1);
                break;
            case 10: {
                int sChoice;
                cout << "Sort by: 1. Name (A-Z) | 2. Size (High-Low) | 3. Date (Old-New): ";
                cin >> sChoice;
                if (sChoice == 1) sortCurrentFolder(compareByName);
                else if (sChoice == 2) sortCurrentFolder(compareBySize);
                else if (sChoice == 3) sortCurrentFolder(compareByDate);
                listCurrentFolder();
                break;
            }
            case 11:
                runAutomatedDemo();
                break;
            case 12: {
                int sub;
                cout << "1. Edit File Content (>> operator) | 2. Rename Item: ";
                cin >> sub;
                if (sub == 1) {
                    cout << "Enter file name: ";
                    cin >> name1;
                    editFileContent(name1);
                } else {
                    cout << "Enter current name and new name: ";
                    cin >> name1 >> name2;
                    renameItem(name1, name2);
                }
                break;
            }
            default:
                cout << "Invalid choice!\n";
        }

        // Automatic directory tree display after every user operation
        showFullTree();
    }
}