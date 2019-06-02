#ifndef AKINA_BPLUSTREE_HPP
#define AKINA_BPLUSTREE_HPP

#include "BasicStruct.hpp"
#include "vector.hpp"
#include "bptree.hpp"
#include <fstream>
#include <functional>
#include <assert.h>


template <class Key,class Data,size_t Nodesize,class Cmp=std::less<Key>>
    class BPlusTree
{
private:
	bptree<Key,Data,Nodesize,Cmp> Bt;
public:
		BPlusTree(const char* FName,const char* MMFName):Bt(FName,MMFName){}
		~BPlusTree()=default;

		int Find(const Key &key,Data &data)
		{
			if (Bt.count(key)==0) return 0;
			data=Bt.find(key);
			return 1;
		}
		int Erase(const Key &key)
		{
			if (Bt.count(key)==0) return 0;
			Bt.remove(key);return 1;
		}

		int Insert(const Key &key,const Data &data)
		{
			if (Bt.count(key)>0) return 0;
			Bt.insert(key,data);return 1;
		}
		int Modify(const Key &key,const Data &data)
		{
			if (Bt.count(key)==0) return 0;
			Bt.set(key,data);return 1;
		}
		int AskArr(const Key &key,std::function<bool(const Key &,const Key &)>CMP,sjtu::vector<Key> &vect1,sjtu::vector<Data> &vect2)
		{
			Bt.search(key,CMP,vect1,vect2);
			return 1;
		}

		void Clear()
		{
			Bt.init();
		}
};
#endif