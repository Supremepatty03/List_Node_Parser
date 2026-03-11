#pragma once
#include <string>
#include <vector>
#include <memory>

using namespace std;

struct ListNode {
	ListNode* prev = nullptr;

	ListNode* next = nullptr;
	ListNode* rand = nullptr;

	string data;
};

struct ParseResult {
    vector<ListNode> nodes;
    vector<int32_t> rand_indices;

    ListNode* head () const { return nodes.empty () ? nullptr : const_cast< ListNode* >( &nodes.front () ); }
};

class IListParser {
public:
    virtual ~IListParser () = default;

    virtual ParseResult parse ( istream& in ) = 0;

    virtual ParseResult parseFile ( const string& filename ) = 0;
};
