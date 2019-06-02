//
// Created by ReFraction on 2019/5/17.
//

#ifndef AKINA_EXMEMORY_HPP
#define AKINA_EXMEMORY_HPP
#include <cstdio>
#include <iostream>
#include <fstream>
#include <cstring>
#include <assert.h>

constexpr off_t nulloff_t = 0xdeadbeef;
class ExMemory
{
    struct node{
        node *pre,*nex;
        off_t node_start,node_end;
        node(off_t node_start = nulloff_t,off_t node_end = nulloff_t,node *pre = nullptr,node *nex = nullptr):node_start(node_start),node_end(node_end),pre(pre),nex(nex){}
    };
    node *head,*tail;
    std::string MemoryManageFileName;

public:
    ExMemory() {
        head = new node;
        head->nex = new node(3 * sizeof(off_t) + sizeof(size_t) + 1,nulloff_t,head);
        head->nex->nex = tail = new node;
        tail->pre = head->nex;
    }
    ~ExMemory() {
        clear();
        delete head;
        delete tail->pre;
        delete tail;
    }
    void SetName(const std::string &MMFName)
    {
        MemoryManageFileName = MMFName;
    }
    //clear file info and memory manage info.
    void clear()
    {
        node *p = head->nex,*q;
        while (p != tail){
            q = p->nex;
            delete p;
            p = q;
        }
        head->nex = new node(3 * sizeof(off_t) + sizeof(size_t),nulloff_t,head,tail);
        tail->pre = head->nex;
    }

    off_t malloc(size_t size) {
        node *p = head->nex;
        off_t start;
        while (p != tail->pre && p->node_end - p->node_start + 1 < size) p = p->nex;
        start = p->node_start;
        if (p->node_end - p->node_start + 1 == size) {
            p->pre->nex = p->nex;
            p->nex->pre = p->pre;
            delete p;
        }
        else p->node_start += size;
        return start;
    }
    void free(off_t start,size_t size) {
        node *p = head->nex,*np;
        while (p != tail->pre && p->node_start < start) p = p->nex;
        if (start + size == p->node_start){
            p->node_start = start;
            np = p;
        }
        else{
            np = new node(start,start + size - 1,p->pre,p);
            p->pre->nex = np;
            p->pre = np;
        }
        p = np->pre;
        if (p->node_end + 1 == np->node_start){
            p->node_end = np->node_end;
            p->nex = np->nex;
            np->nex->pre = p;
            delete np;
        }
    }
    //read memory info into memory manager.
    void read_info()
    {
        std::fstream MMFile;
        MMFile.open(MemoryManageFileName,std::ios::in|std::ios::binary);
        assert(MMFile.is_open());
        MMFile.seekg(0);
        off_t f,t;
        clear();
        delete head->nex;
        node *p = head;
        while (true)
        {
            if (!MMFile.read(reinterpret_cast<char*> (&f),sizeof(off_t)))
                break;
            assert(MMFile.read(reinterpret_cast<char*> (&t),sizeof(off_t)));
            p->nex = new node(f,t,p);
            p = p->nex;
        }
        tail->pre = p;
        p->nex = tail;
        MMFile.close();
    }
    //write memory manager info into memory file.
    void write_info()
    {
        std::fstream MMFile;
        MMFile.open(MemoryManageFileName,std::ios::out|std::ios::binary|std::ios::trunc);
        MMFile.seekp(0);
        node *p = head->nex;
        while (p != tail)
        {
            MMFile.write(reinterpret_cast<char*> (&p->node_start),sizeof(off_t));
            MMFile.write(reinterpret_cast<char*> (&p->node_end),sizeof(off_t));
            p = p->nex;
        }
        MMFile.flush();
        MMFile.close();
    }
};
#endif //AKINA_EXMEMORY_HPP
