#pragma once

#include <string>
#include <cstring>
#include <fstream>
#include<iostream>

/// A disk‐based B+-tree with fixed 4KB pages.
/// Keys are fixed‐length strings (max 40 bytes), values are 8‐byte offsets.
class BPlusTree {
public:
    static constexpr int PAGE_SIZE = 4096;
    static constexpr int KEY_SIZE = 40;
    static constexpr int PTR_SIZE = sizeof(long);
    static constexpr int HEADER_SIZE = sizeof(bool)   // isLeaf
        + sizeof(int)    // keyCount
        + sizeof(long)   // parentPage
        + sizeof(long);  // nextLeafPage

    // Max entries per node: floor((PAGE_SIZE - HEADER_SIZE) / (KEY_SIZE + PTR_SIZE))
    static constexpr int ORDER = (PAGE_SIZE - HEADER_SIZE) / (KEY_SIZE + PTR_SIZE);

    struct Node {
        bool    isLeaf;
        int     keyCount;
        long    parentPage;
        long    nextLeafPage;
        char    keys[ORDER][KEY_SIZE];
        long    children[ORDER + 1];
        long    selfPage;

        Node(bool leaf = true)
            : isLeaf(leaf), keyCount(0), parentPage(-1), nextLeafPage(-1), selfPage(-1) {
            std::memset(keys, 0, sizeof(keys));
            for (int i = 0; i < ORDER + 1; ++i) children[i] = -1;
        }
    };

    explicit BPlusTree(const std::string& filename);
    ~BPlusTree();

    /// Insert key→recordOffset mapping
    void insert(const std::string& key, long recordOffset);

    /// Search for key; if found, set recordOffset and return true
    bool search(const std::string& key, long& recordOffset);

private:
    std::string filePath;
    long        pageCount;

    long  allocateNode();
    void  writeNode(const Node& node);
    Node  readNode(long page);
    void  splitAndInsert(Node& node, const std::string& key, long recordOffset);
    bool findRecordAtIndex(int index, long& recordOffset);
};
