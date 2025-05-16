#include "bplustree.hpp"
#include <filesystem>
#include <vector>
#include <cstring>

// Assuming PAGE_SIZE and KEY_SIZE are defined in bplustree.hpp
// Compute HEADER_SIZE as sum of fixed-size fields
static constexpr size_t HEADER_SIZE = sizeof(bool)   // isLeaf
+ sizeof(int)      // keyCount
+ sizeof(long)     // parentPage
+ sizeof(long);    // nextLeafPage

BPlusTree::BPlusTree(const std::string& filename)
    : filePath(filename), pageCount(0) {
    // Ensure file exists
    std::fstream f(filePath, std::ios::in | std::ios::out | std::ios::binary);
    if (!f) {
        std::ofstream create(filePath, std::ios::binary);
        create.close();
    }
    // Determine current page count (round up)
    std::ifstream in(filePath, std::ios::binary | std::ios::ate);
    long size = in.tellg();
    if (size <= 0) {
        pageCount = 0;
    }
    else {
        pageCount = (size + PAGE_SIZE - 1) / PAGE_SIZE;
    }
}

BPlusTree::~BPlusTree() {
    // Nothing to do
}

long BPlusTree::allocateNode() {
    long newPage = pageCount;
    // Initialize an empty node
    Node empty{};
    empty.selfPage = newPage;
    empty.isLeaf = true;
    empty.keyCount = 0;
    empty.parentPage = -1;
    empty.nextLeafPage = -1;
    std::memset(empty.keys, 0, sizeof(empty.keys));
    std::memset(empty.children, 0, sizeof(empty.children));

    writeNode(empty);
    ++pageCount;
    return newPage;
}

void BPlusTree::writeNode(const Node& node) {
    std::fstream f(filePath, std::ios::in | std::ios::out | std::ios::binary);
    f.seekp(node.selfPage * PAGE_SIZE);

    // Prepare a full PAGE_SIZE buffer
    std::vector<char> page(PAGE_SIZE, 0);
    size_t offset = 0;

    // Copy header fields
    std::memcpy(page.data() + offset, &node.isLeaf, sizeof(node.isLeaf));       offset += sizeof(node.isLeaf);
    std::memcpy(page.data() + offset, &node.keyCount, sizeof(node.keyCount));     offset += sizeof(node.keyCount);
    std::memcpy(page.data() + offset, &node.parentPage, sizeof(node.parentPage));   offset += sizeof(node.parentPage);
    std::memcpy(page.data() + offset, &node.nextLeafPage, sizeof(node.nextLeafPage)); offset += sizeof(node.nextLeafPage);

    // Copy keys array
    std::memcpy(page.data() + offset, node.keys, sizeof(node.keys));
    offset += sizeof(node.keys);

    // Copy children array
    std::memcpy(page.data() + offset, node.children, sizeof(node.children));

    // Write full page to disk
    f.write(page.data(), PAGE_SIZE);
    f.close();
}

BPlusTree::Node BPlusTree::readNode(long page) {
    Node node{};
    std::ifstream f(filePath, std::ios::binary);
    f.seekg(page * PAGE_SIZE);

    // Read header
    f.read(reinterpret_cast<char*>(&node.isLeaf), sizeof(node.isLeaf));
    f.read(reinterpret_cast<char*>(&node.keyCount), sizeof(node.keyCount));
    f.read(reinterpret_cast<char*>(&node.parentPage), sizeof(node.parentPage));
    f.read(reinterpret_cast<char*>(&node.nextLeafPage), sizeof(node.nextLeafPage));

    // Read arrays
    f.read(reinterpret_cast<char*>(node.keys), sizeof(node.keys));
    f.read(reinterpret_cast<char*>(node.children), sizeof(node.children));

    node.selfPage = page;
    f.close();
    return node;
}

void BPlusTree::insert(const std::string& key, long recordOffset) {
    if (pageCount == 0) {
        allocateNode();  // create root
    }
    Node root = readNode(0);
    splitAndInsert(root, key, recordOffset);
}

bool BPlusTree::search(const std::string& key, long& recordOffset) {
    if (pageCount == 0) {
        return false;
    }
    Node node = readNode(0);
    // descend to leaf
    while (!node.isLeaf) {
        int i = 0;
        while (i < node.keyCount && key > node.keys[i]) ++i;
        node = readNode(node.children[i]);
    }
    // search leaf
    for (int i = 0; i < node.keyCount; ++i) {
        if (key == node.keys[i]) {
            recordOffset = node.children[i];
            return true;
        }
    }
    return false;
}

void BPlusTree::splitAndInsert(Node& node, const std::string& key, long recordOffset) {
    if (node.isLeaf) {
        // find insert position
        int i = 0;
        while (i < node.keyCount && key > node.keys[i]) ++i;

        // shift keys and pointers to make room
        for (int j = node.keyCount; j > i; --j) {
            std::memcpy(node.keys[j], node.keys[j - 1], KEY_SIZE);
            node.children[j] = node.children[j - 1];
        }

        // copy new key safely
        std::size_t copyLen = std::min<std::size_t>(key.size(), KEY_SIZE - 1);
        std::memcpy(node.keys[i], key.data(), copyLen);
        node.keys[i][copyLen] = '\0';

        // insert pointer
        node.children[i] = recordOffset;
        node.keyCount++;

        writeNode(node);
        // TODO: handle split when node.keyCount > ORDER
    }
    else {
        // descend to correct child
        int i = 0;
        while (i < node.keyCount && key > node.keys[i]) ++i;
        Node child = readNode(node.children[i]);
        splitAndInsert(child, key, recordOffset);
    }
}

bool BPlusTree::findRecordAtIndex(int index, long& recordOffset) {
    if (pageCount == 0) return false;

    Node node = readNode(0);
    // Traverse to the leftmost leaf
    while (!node.isLeaf) {
        node = readNode(node.children[0]);
    }

    // Walk leaf nodes until index is found
    int count = 0;
    while (true) {
        for (int i = 0; i < node.keyCount; ++i) {
            if (count == index) {
                recordOffset = node.children[i];
                return true;
            }
            ++count;
        }
        if (node.nextLeafPage == -1) break;
        node = readNode(node.nextLeafPage);
    }

    return false; // Index out of bounds
}
