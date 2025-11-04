#pragma once

#include <functional>

struct llist_node {
    void* data;
    llist_node* prev;
    llist_node* next;
};

class llist {
private:
    int m_size;
    llist_node* m_head;
    llist_node* m_tail;

public:
    llist()
        : m_size { 0 }
        , m_head { nullptr }
        , m_tail { nullptr }
    {
    }

    ~llist() noexcept;

    int size() { return m_size; }

    void* head();

    llist_node* head_node() { return m_head; }

    void* tail();
    void insert(void* data);
    void append(void* data);
    void* find_element(std::function<bool(void*)>);
    void* delete_element(std::function<bool(void*)>);
    void for_each_node(std::function<void(llist_node*)>);
};
