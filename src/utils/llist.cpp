#include "llist.h"
#include <cassert>
#include <functional>

llist::~llist() noexcept
{
    llist_node* curr_node { m_head };

    while (curr_node) {
        if (curr_node->prev) {
            delete curr_node->prev;
        }
        curr_node = curr_node->next;
    }

    if (m_tail) {
        delete m_tail;
    }
}

void* llist::tail()
{
    {
        if (!m_tail) {
            return nullptr;
        } else {
            return m_tail->data;
        }
    }
}

void* llist::head()
{
    {
        if (!m_head) {
            return nullptr;
        } else {
            return m_head->data;
        }
    }
}

// insert a new node at the head
void llist::insert(void* data)
{
    llist_node* n { new llist_node { .data = data, .prev = nullptr, .next = m_head } };
    if (!m_head) {
        m_head = n;
    } else {
        (*m_head).prev = n;
        if (m_size == 1) {
            m_tail = m_head;
        }
        m_head = n;
    }

    ++m_size;
}

// append a new node to the end
void llist::append(void* data)
{
    llist_node* n { new llist_node { .data = data, .prev = m_tail, .next = nullptr } };
    if (!m_head) {
        m_head = n;
    } else if (!m_tail) {
        assert(m_size == 1);
        m_tail = n;
        m_tail->prev = m_head;
    } else {
        (*m_tail).next = n;
        m_tail = n;
    }

    ++m_size;
}

void* llist::find_element(std::function<bool(void*)> cmpfn)
{
    auto curr_node { m_head };

    while (curr_node) {
        if (cmpfn(curr_node->data)) {
            return curr_node->data;
        }
        curr_node = curr_node->next;
    }
    return nullptr;
}

void* llist::delete_element(std::function<bool(void*)> cmpfn)
{
    auto curr_node { m_head };

    while (curr_node) {
        if (cmpfn(curr_node->data)) {
            if (curr_node->prev) {
                curr_node->prev->next = curr_node->next;
            }
            if (curr_node->next) {
                curr_node->next->prev = curr_node->prev;
            }
            auto rdata { curr_node->data };
            delete curr_node;
            if (curr_node == m_head) {
                m_head = nullptr;
            }
            if (curr_node == m_tail) {
                m_tail = nullptr;
            }
            --m_size;
            return rdata;
        }
        curr_node = curr_node->next;
    }
    return nullptr;
}

void llist::for_each_node(std::function<void(llist_node* n)> fn)
{
    auto curr_node { m_head };
    while (curr_node) {
        fn(curr_node);
        curr_node = curr_node->next;
    }
}
