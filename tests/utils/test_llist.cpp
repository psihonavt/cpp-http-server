#include "utils/llist.h"
#include <cassert>
#include <catch2/catch_test_macros.hpp>
#include <vector>

void assert_llist_content(llist& ll, std::vector<void*>& expected_pointers)
{
    auto curr_node { ll.head_node() };
    for (auto p : expected_pointers) {
        REQUIRE(curr_node->data);
        CHECK(curr_node->data == p);
        curr_node = curr_node->next;
    }
}

TEST_CASE("Linked list insertion", "[llist]")
{
    llist ll {};
    int a { 1 }, b { 2 }, c { 3 };
    std::string d { "abcd" };
    ll.insert(&a);
    ll.insert(&b);
    ll.insert(&c);
    ll.append(&d);
    REQUIRE(ll.size() == 4);

    std::vector<void*> expected { &c, &b, &a, &d };
    assert_llist_content(ll, expected);
}

TEST_CASE("Linked list find and removal", "[llist]")
{
    llist ll {};
    int a { 1 }, b { 2 };
    ll.insert(&a);
    ll.insert(&b);
    REQUIRE(ll.size() == 2);

    REQUIRE(ll.delete_element([&](void* data) -> bool { return data == &a; }) == &a);
    REQUIRE(ll.size() == 1);

    REQUIRE(ll.find_element([&](void* data) -> bool { return data == &b; }) == &b);

    REQUIRE(ll.delete_element([&](void* data) -> bool { return data == &b; }) == &b);
    REQUIRE(ll.size() == 0);
}

TEST_CASE("Linked list foreach", "[llist]")
{
    llist ll {};
    int a { 1 }, b { 2 };
    ll.insert(&a);
    ll.insert(&b);
    REQUIRE(ll.size() == 2);

    std::vector<void*> expected { &b, &a };
    std::vector<void*> seen {};

    auto traverse_fn {
        [&seen](llist_node* node) { seen.push_back(node->data); }
    };

    ll.for_each_node(traverse_fn);

    REQUIRE(seen == expected);
}
