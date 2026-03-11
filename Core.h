#pragma once
#include <string>
#include <vector>
#include <memory>

using namespace std;

struct ListNode {
	ListNode* prev = nullptr;

	ListNode* next = nullptr;
	ListNode* rand = nullptr;

	std::string data;
};

struct ParseResult {
    std::vector<std::unique_ptr<ListNode>> nodes;
    std::vector<int32_t> rand_indices;

    ListNode* head () const { return nodes.empty () ? nullptr : nodes.front ().get (); }
};

class IListParser {
public:
    virtual ~IListParser () = default;

    virtual ParseResult parse ( std::istream& in ) = 0;

    virtual ParseResult parseFile ( const std::string& filename ) = 0;
};
