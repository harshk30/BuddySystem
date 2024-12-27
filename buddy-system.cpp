#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <iomanip>

using namespace std;

const int MAX_POWERS = 11;
const vector<int> powers = {1, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024};

class Block {
public:
    string pname;
    int memory;
    int frag;
    int free;
    
    Block(const string& name = "free", int mem = 0, int f = 0, bool isFree = true)
        : pname(name), memory(mem), frag(f), free(isFree) {}
};

class MemoryManager {
private:
    vector<Block> blocks;

    bool isPerfectPower(int value) {
        return find(powers.begin(), powers.end(), value) != powers.end();
    }

    int findNearestGreaterPower(int amount) {
        auto it = upper_bound(powers.begin(), powers.end(), amount);
        return it != powers.end() ? *it : 0;
    }

    int findNearestLowerPower(int amount) {
        auto it = lower_bound(powers.begin(), powers.end(), amount);
        if (it != powers.begin() && it != powers.end()) {
            --it;
            return *it;
        }
        return 0;
    }

    void addBlock(const string& pname, int blockSize, int amount) {
        blocks.emplace_back(pname, amount, blockSize - amount, false);
    }

    void addFreeBlock(int blockSize) {
        blocks.emplace_back("free", blockSize, 0, true);
    }

    int findSmallestFreeBlock(int blockSize) {
        int foundIndex = -1;
        for (size_t i = 0; i < blocks.size(); i++) {
            if (blocks[i].free && blocks[i].memory >= blockSize) {
                if (foundIndex == -1 || blocks[i].memory < blocks[foundIndex].memory) {
                    foundIndex = i;
                }
            }
        }
        return foundIndex;
    }

    int findExactFreeBlock(int blockSize) {
        for (size_t i = 0; i < blocks.size(); i++) {
            if (blocks[i].free && blocks[i].memory == blockSize) {
                return i;
            }
        }
        return -1;
    }

    int findExactFreeBlockButNotTheGivenBlock(int blockSize, int index) {
        for (size_t i = 0; i < blocks.size(); i++) {
            if (blocks[i].free && blocks[i].memory == blockSize && i != static_cast<size_t>(index)) {
                return i;
            }
        }
        return -1;
    }

    void removeBlock(int index) {
        blocks.erase(blocks.begin() + index);
    }

    void mergeEqualSizeFreeBlocks() {
        int other;
        do {
            other = -1;
            for (size_t i = 0; i < blocks.size(); i++) {
                if (blocks[i].free) {
                    other = findExactFreeBlockButNotTheGivenBlock(blocks[i].memory, i);
                    if (other > 0) {
                        blocks[i].memory += blocks[other].memory;
                        break;
                    }
                }
            }
            if (other > 0) {
                removeBlock(other);
            }
        } while (other > 0);
    }

    int findBlockWithProcessName(const string& pname) {
        for (size_t i = 0; i < blocks.size(); i++) {
            if (blocks[i].pname == pname) {
                return i;
            }
        }
        return -1;
    }

    void freeBlock(int index) {
        blocks[index].pname = "free";
        blocks[index].memory += blocks[index].frag;
        blocks[index].frag = 0;
        blocks[index].free = true;
    }

public:
    MemoryManager() {
        // Initialize with first free block
        blocks.emplace_back("free", 1024, 0, true);
    }

    void printBlocks() const {
        cout << "************* Memory *************\n";
        for (size_t i = 0; i < blocks.size(); i++) {
            cout << "Block-" << i + 1 << ": " << left << setw(4) 
                 << blocks[i].pname << " - " << blocks[i].memory + blocks[i].frag << "M";
            if (!blocks[i].free) {
                cout << " (Internal Frag = " << blocks[i].frag << "M)";
            }
            cout << "\n";
        }
        cout << "**********************************\n";
    }

    void requestMemory(const string& pname, int amount) {
        int blockSize = findNearestGreaterPower(amount);
        if (blockSize > 0) {
            int foundIndex = findSmallestFreeBlock(blockSize);

            if (foundIndex == -1) {
                cout << "No enough space for the request!\n";
                return;
            }

            if (blockSize == blocks[foundIndex].memory) {
                blocks[foundIndex].pname = pname;
                blocks[foundIndex].memory = amount;
                blocks[foundIndex].frag = blockSize - amount;
                blocks[foundIndex].free = false;
            } else {
                addBlock(pname, blockSize, amount);
                blocks[foundIndex].memory -= blockSize;

                int remainingBlockSize = blocks[foundIndex].memory;
                if (!isPerfectPower(remainingBlockSize)) {
                    int nearBlockSize = findNearestLowerPower(remainingBlockSize);
                    blocks[foundIndex].memory = nearBlockSize;
                    remainingBlockSize -= nearBlockSize;

                    while (!isPerfectPower(remainingBlockSize)) {
                        nearBlockSize = findNearestLowerPower(remainingBlockSize);
                        addFreeBlock(nearBlockSize);
                        remainingBlockSize -= nearBlockSize;
                    }

                    addFreeBlock(remainingBlockSize);
                }
            }
        } else {
            cout << "No enough space for the request!\n";
        }
    }

    void releaseMemory(const string& pname) {
        int ind = findBlockWithProcessName(pname);
        if (ind == -1) {
            cout << "Invalid process name\n";
        } else {
            freeBlock(ind);
            mergeEqualSizeFreeBlocks();
        }
    }
};

int main() {
    MemoryManager manager;
    manager.printBlocks();

    while (true) {
        int choice;
        cout << "> Enter 0(EXIT), 1(request), 2(release): ";
        cin >> choice;

        if (choice == 0) {
            cout << "!!! THE END !!!\n";
            break;
        }

        if (choice == 1) {
            string pname;
            int amount;
            cout << "Enter process name and requested space(M): ";
            cin >> pname >> amount;
            manager.requestMemory(pname, amount);
            manager.printBlocks();
        }
        else if (choice == 2) {
            string pname;
            cout << "Enter process name: ";
            cin >> pname;
            manager.releaseMemory(pname);
            manager.printBlocks();
        }
        else {
            cout << "Invalid choice\n";
        }
    }

    return 0;
}
