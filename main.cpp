/* Task 0: Mini-Hekaton
 
 This file contains an incomplete implementation of a storage backend
 for a relation with three attributes (a, b, and c). Each row is
 dynamically allocated on the heap. For attribute a the relation has a
 hash index which stores pointers to the rows. Rows with the same hash
 value are linked using the next field and can only be accessed through
 the hash table (to scan all rows one has to iterate through the hash
 table). This storage format is used in Hekaton, the main-memory engine
 of Microsoft SQL Server 2014.
 
 Complete the constructor, destructor, insert, lookup, and remove
 functions (see TODO). You need a C++11 compiler. In total the code
 required is less than 50 lines. The main function contains test code. */

#include <cassert>
#include <iostream>
#include <cstring>
#include <vector>
#include <algorithm>
#include <chrono>

using namespace std;
using namespace std::chrono;

struct Row {
    /// Attribute a
    uint64_t a;
    /// Attribute b
    uint64_t b;
    /// Attribute c
    uint64_t c;
    /// The next pointer for linking rows that have the same hash value in the hash table
    Row* next;
};

struct Relation {
    /// Number of rows in relation
    uint64_t size;
    /// Size of the hash table, must be a power of two
    uint64_t sizeIndex;
    /// Hash table
    Row** index;

    // Construct a relation
    Relation(uint64_t sizeIndex) : size(0), sizeIndex(sizeIndex) {
        // Check that sizeIndex is a power of two
        assert((sizeIndex&(sizeIndex-1))==0);
        // TODO memory allocation
        index = new Row*[sizeIndex];
    }

    // Destroy relation (free all memory)
    ~Relation() {
        // TODO free all memory
        for (int i = 0; i < sizeIndex; ++i) {
            // remove all elements until linked list is empty
            while (index[i] != nullptr) {
                auto rowAtIndex = index[i];
                remove(rowAtIndex);
            }

            // just good practice I believe, setting it to null
            index[i] = nullptr;
        }

        delete[](index);
        index = nullptr;
    }

    // Insert a new row
    void insert(uint64_t a,uint64_t b,uint64_t c) {
        // TODO inserting a new row
        auto new_row = new Row;
        new_row->c = c;
        new_row->b = b;
        new_row->a = a;
        new_row->next = nullptr;

        // place where the elem should be inserted
        auto index_a = hash(a);
        ++size;

        if (index[index_a] == nullptr) {
            index[index_a] = new_row;
        } else {
            auto curr_row = index[index_a];

            while (curr_row->next != nullptr) {
                curr_row = curr_row->next;
            }

            curr_row->next = new_row;
        }
    }

    /// Find a row using the index
    Row* lookup(uint64_t a) {
        // TODO looks up the a elem

        auto index_a = hash(a);
        auto curr_row = index[index_a];

        while(curr_row != nullptr) {
            if (curr_row->a == a)
                return curr_row;
            curr_row = curr_row->next;
        }

        // not found
        return nullptr;
    }

    // Remove a row
    void remove(Row* row) {
        // TODO removing a row
        auto index_a = hash(row->a);
        if (index[index_a] == nullptr)
            return;

        auto curr_row = index[index_a];
        Row *prev = nullptr;

        // iterate through all of the elements. Return the match if found.
        while (curr_row != nullptr) {
            if (curr_row->a == row->a) {
                // are we at the head of the list ?
                if (prev == nullptr) {
                    index[index_a] = curr_row->next;
                } else {
                    prev->next = curr_row->next;
                }

                delete(curr_row);
                --size;
                return;
            }

            prev = curr_row;
            curr_row = curr_row->next;
        }
    }

    //maybe it should be static ?
    // Computes index into hash table for attribute value a
    uint64_t hash(uint64_t a) {
        return a&(sizeIndex-1);
    }
};

int main() {
    uint64_t n=2500000;
    Relation R(1ull<<20);

    // Random test data
    vector<Row> v;
    for (uint64_t i=0; i<n; i++)
        v.push_back({i,i/3,i/7,nullptr});

    {
        random_shuffle(v.begin(),v.end());
        // Insert test data
        auto start=high_resolution_clock::now();
        for (Row& r : v)
            R.insert(r.a,r.b,r.c);
        cout << "insert " << duration_cast<duration<double>>(high_resolution_clock::now()-start).count() << "s" << endl;
    }

    {
        random_shuffle(v.begin(),v.end());
        // Lookup rows
        auto start=high_resolution_clock::now();
        for (Row& r : v) {
            Row* r2=R.lookup(r.a);
            assert(r2&&(r2->a==r.a));
        }
        cout << "lookup " << duration_cast<duration<double>>(high_resolution_clock::now()-start).count() << "s" << endl;
    }

    {
        auto start=high_resolution_clock::now();
        // Scan all entries and add attribute a
        uint64_t sum=0;
        for (uint64_t i=0; i<R.sizeIndex; i++) {
            Row* r=R.index[i];
            while (r) {
                sum+=r->a;
                r=r->next;
            }
        }
        cout << "scan " << duration_cast<duration<double>>(high_resolution_clock::now()-start).count() << "s" << endl;
        assert(sum==((n*(n-1))/2));
    }

    {
        random_shuffle(v.begin(),v.end());
        // Delete all entries
        auto start=high_resolution_clock::now();
        for (Row& r : v) {
            Row* r2=R.lookup(r.a);
            assert(r2);
            R.remove(r2);
            assert(!R.lookup(r.a));
        }
        cout << "remove " << duration_cast<duration<double>>(high_resolution_clock::now()-start).count() << "s" << endl;
        // Make sure the table is empty
        for (unsigned i=0; i<R.sizeIndex; i++)
            assert(R.index[i]==nullptr);
    }

    return 0;
}
