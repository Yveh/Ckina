#ifndef AKINA_BPLUSTREE_HPP
#define AKINA_BPLUSTREE_HPP
//#include "FileManager.hpp"
#include "ExMemory.hpp"
#include "BasicStruct.hpp"
#include "vector.hpp"
#include <fstream>
#include <functional>
#include <assert.h>


enum NodeType{INTERNAL,LEAF};
template <class Key,class Data,size_t Nodesize,class Cmp=std::less<Key>>
class BPlusTree
{
private:
	std::fstream File;
	std::string FileName;
	ExMemory MemoryManager;

	off_t Root,Head,Tail;
	size_t ElementNumber = 0;

	Cmp Cmp1 = Cmp();// Cmp for single key.

	class Block;
	class Node
	{
		NodeType nodeType;
		Key key0;
		size_t elementNumber;
		off_t pos;
		off_t father;
		off_t pre;
		off_t nex;
		friend BPlusTree;
		friend Block;
	public:
		Node(NodeType nodeType = LEAF,Key key0 = Key(),size_t elementNumber = 0,off_t pos = nulloff_t,off_t father = nulloff_t,off_t pre = nulloff_t,off_t nex = nulloff_t) : nodeType(nodeType),key0(key0),pos(pos),elementNumber(elementNumber),father(father),pre(pre),nex(nex){}
	};
#ifndef DEBUG
	const size_t N = (Nodesize - sizeof(Node)) / (sizeof(off_t) + sizeof(Key)) - 1;
	const size_t M = (Nodesize - sizeof(Block)) / (sizeof(Data) + sizeof(Key)) - 1;
#endif
#ifdef DEBUG
	const size_t N = 5;
	const size_t M = 7;
	#endif

	Node new_node(NodeType nodetype,const Key &key)
	{
		Node *p = new Node(nodetype,key,0,MemoryManager.malloc(Nodesize));
		return *p;
	}
	void delete_Node (Node &p)
	{
		if (p.pre != nulloff_t)
		{
			Node pp = read_node(p.pre);
			pp.nex = p.nex;
		}
		if (p.nex != nulloff_t)
		{
			Node pn = read_node(p.nex);
			pn.pre = p.pre;
		}
		MemoryManager.free(p.pos,Nodesize);
		delete &p;
	}
	class Block
	{
		size_t elementNumber;
		Key key0;
		off_t pos;
		off_t pre;
		off_t nex;
		off_t father;
		friend BPlusTree;
		friend Node;
	public:
		Block(size_t elementNumber = 0,Key key0 = Key(),off_t pos = nulloff_t,off_t pre = nulloff_t,off_t nex = nulloff_t,off_t father = nulloff_t) : elementNumber(elementNumber),key0(key0),pos(pos),pre(pre),nex(nex),father(father){}
	};
	Block new_block(const Key &key)
	{
		Block *b = new Block(0,key,MemoryManager.malloc(Nodesize));
		return *b;
	}
	void delete_block(Block &b)
	{
		if (b.pre != nulloff_t)
		{
			Block bp = read_block(b.pre);
			bp.nex = b.nex;
		}
		if (b.nex != nulloff_t)
		{
			Block bn = read_block(b.nex);
			bn.pre = b.pre;
		}
		MemoryManager.free(b.pos,Nodesize);
		delete &b;
	}
	bool IsEqual(const Key &x,const Key &y){
		return !Cmp1(x,y) && !Cmp1(y,x);
	}
	void read_bptree_info() {
		File.seekg(0);
		File.read(reinterpret_cast<char *>(&Root), sizeof(off_t));
		File.read(reinterpret_cast<char *>(&Head), sizeof(off_t));
		File.read(reinterpret_cast<char *>(&Tail), sizeof(off_t));
		File.read(reinterpret_cast<char *>(&ElementNumber),sizeof(size_t));
	}
	void write_bptree_info()
	{
		File.seekp(0);
		File.write(reinterpret_cast<char *>(&Root),sizeof(off_t));
		File.write(reinterpret_cast<char *>(&Head),sizeof(off_t));
		File.write(reinterpret_cast<char *>(&Tail),sizeof(off_t));
		File.write(reinterpret_cast<char *>(&ElementNumber),sizeof(size_t));
		File.flush();
	}
	Node read_node(off_t root) {
		Node p;
		File.seekg(root,std::ios::beg);
		File.read(reinterpret_cast<char *>(&p),sizeof(p));
		return p;
	}

	Block read_block(off_t root)
	{
		File.seekg(root);
		Block b;
		File.read(reinterpret_cast<char *>(&b),sizeof(Block));
		return b;
	}
	void write_node(Node &p)
	{
		File.close();
		File.open(FileName,std::ios::in|std::ios::out|std::ios::binary);
		File.seekp(p.pos);
		File.write(reinterpret_cast<char *>(&p),sizeof(Node));
		File.flush();
	}
	void write_block(Block &b)
	{
		File.close();
		File.open(FileName,std::ios::in|std::ios::out|std::ios::binary);
		File.seekp(b.pos);
		File.write(reinterpret_cast<char *>(&b),sizeof(Block));
		File.flush();
	}

	void fill_buffer_with_node(char * buffer,const Node &p)
	{
        File.close();
        File.open(FileName,std::ios::in|std::ios::out|std::ios::binary);
		File.seekg(p.pos + sizeof(Node));
		File.read(buffer,p.elementNumber * (sizeof(Key) + sizeof(off_t)));
		File.flush();
	}
	void fill_buffer_with_block(char *buffer,const Block &b)
	{
		File.seekg(b.pos + sizeof(Block));
		File.read(buffer,b.elementNumber * (sizeof(Key) + sizeof(Data)));
	}
	void write_buffer_into_node(char *buffer,const Node &p)
	{
	    File.close();
	    File.open(FileName,std::ios::in|std::ios::out|std::ios::binary);
		File.seekp(p.pos + sizeof(Node));
		File.write(buffer,p.elementNumber * (sizeof(Key) + sizeof(off_t)));
		File.flush();
	}
	void write_buffer_into_block(char *buffer,const Block &b)
	{
        File.close();
        File.open(FileName,std::ios::in|std::ios::out|std::ios::binary);
		File.seekp(b.pos + sizeof(Block));
		File.write(buffer,b.elementNumber * (sizeof(Key) + sizeof(Data)));
		File.flush();
	}
	off_t * pos_n_in_node(int n,char * buffer)
	{
		return reinterpret_cast<off_t *>(buffer +(n + 1) * sizeof(Key) + n * sizeof(off_t));
	}
	Key * key_n_in_node(int n,char * buffer)
	{
		return reinterpret_cast<Key *>(buffer + n * sizeof(Key) + n * sizeof(off_t));
	}
	Data * data_n_in_block(int n,char * buffer)
	{
		return reinterpret_cast<Data *>(buffer +(n + 1) * sizeof(Key) + n * sizeof(Data));
	}
	Key * key_n_in_block(int n,char * buffer)
	{
		return reinterpret_cast<Key *>(buffer + n * sizeof(Key) + n * sizeof(Data));
	}
    //It's actually lowerbound - 1;
    int search_rank_in_node_lowerbound(const Key &key,const Node &p,char * const buffer,std::function<bool(const Key &,const Key &)>CMP = Cmp())
    {
        int l = -1,r = p.elementNumber;
        while (l < r - 1)
        {
            int m = (l + r) >> 1;
            Key tmp = *key_n_in_node(m,buffer);
            if (CMP(tmp,key))
                l = m;
            else
                r = m;
        }
        return l;
    }
    int search_rank_in_node_upperbound(const Key &key,const Node &p,char * const buffer,std::function<bool(const Key &,const Key &)>CMP = Cmp())
    {
        int  l = -1,r = p.elementNumber;
        while (l < r - 1)
        {
            int m = (l + r) >> 1;
            Key tmp = *key_n_in_node(m,buffer);
            if (!CMP(key,tmp))
                l = m;
            else
                r = m;
        }
        return l;
    }
    int search_rank_in_block_lowerbound(const Key &key,const Block &b,char * const buffer,std::function<bool(const Key &,const Key &)>CMP = Cmp())
    {
        int l = -1,r = b.elementNumber;
        while (l < r - 1)
        {
            int m = (l + r) >> 1;
            Key tmp = *key_n_in_block(m,buffer);
            if (CMP(tmp,key))
                l = m;
            else
                r = m;
        }
        return l;
    }
    int search_rank_in_block_upperbound(const Key &key,const Block &b,char * const buffer,std::function<bool(const Key &,const Key &)>CMP = Cmp())
    {
        int l = -1,r = b.elementNumber;
        while (l < r - 1)
        {
            int m = (l + r) >> 1;
            Key tmp = *key_n_in_block(m,buffer);
            if (!CMP(key,tmp))
                l = m;
            else
                r = m;
        }
        return l;
    }
	//Used after nowroot has judged not to be nulloff_t
	int block_insert(Block &b,const Key &key,const Data &data)
	{
		char buffer[Nodesize];
		fill_buffer_with_block(buffer,b);
//#ifdef DEBUG
//		std::cout << "Block before insert: ";
//        for (int i = 0; i < b.elementNumber ; ++i)
//            std::cout << i << " " << *key_n_in_block(i,buffer) << " " << *data_n_in_block(i,buffer) << "\n";
//		#endif
		int n = search_rank_in_block_lowerbound(key,b,buffer);
		if (n + 1 >= 0 && n + 1 < b.elementNumber && (IsEqual(*key_n_in_block(n + 1,buffer),key)))
			return 0;
		for (int i = b.elementNumber - 1; i > n; --i)
		{
			*data_n_in_block(i + 1,buffer) = *data_n_in_block(i,buffer);
			*key_n_in_block(i + 1,buffer) = *key_n_in_block(i,buffer);
		}
		*data_n_in_block(n + 1,buffer) = data;
		*key_n_in_block(n + 1,buffer) = key;

		if (n == -1)
        {
		    Key prekey = b.key0;
		    b.key0 = key;
		    write_block(b);
		    off_t fa_off = b.father;
		    int n1 = n;
		    while (n1 == -1)
            {
                Node fa = read_node(fa_off);
                if (fa.elementNumber == 0)
                    break;
                char buffer1[Nodesize];
                fill_buffer_with_node(buffer1,fa);
                n1 = search_rank_in_node_lowerbound(prekey,fa,buffer1);
                *key_n_in_node(n1 + 1,buffer1) = key;
                write_buffer_into_node(buffer1,fa);
                if (n1 == -1)
                {
                    fa.key0 = key;
                    write_node(fa);
                    fa_off = fa.father;
                    if (fa_off == nulloff_t)
                        break;
                }
            }
//		    Node tmp = read_node(b.father);
//		    if (tmp.elementNumber)
//            {
//                char buffer1[Nodesize];
//                fill_buffer_with_node(buffer1,tmp);
//                int n1 = search_rank_in_node_lowerbound(b.key0,tmp,buffer1);
//                b.key0 = key;
//                    *key_n_in_node(n1 + 1,buffer1) = key;
//                    write_buffer_into_node(buffer1,tmp);
//                while(n1 == -1)
//                {
//                    tmp.key0 = key;
//                    write_node(tmp);
//                    if (tmp.father == nulloff_t)
//                        break;
//                    Node np = tmp;
//                    tmp = read_node(tmp.father);
//                    fill_buffer_with_node(buffer1,tmp);
//                    n1 = search_rank_in_node_lowerbound(np.key0,tmp,buffer1);
//                }
//                /*if (n1 == -1)
//                {
//                    tmp.key0 = key;
//                    write_node(tmp);
//                }*/
//            }
//            else
//                b.key0 = key;

        }
//#ifdef DEBUG
//        std::cout << "Block after insert: ";
//        for (int i = 0; i <= b.elementNumber ; ++i)
//            std::cout << i << " " << *key_n_in_block(i,buffer) << " " << *data_n_in_block(i,buffer) << "\n";
//#endif
		if (++b.elementNumber > M)
		{
			Block b2 = Block(0,*key_n_in_block((M >> 1) + 1,buffer),MemoryManager.malloc(Nodesize));
			b2.father = b.father;
			b2.pre = b.pos;
			b2.nex = b.nex;
			b.nex = b2.pos;
			if (b2.nex != nulloff_t)
			{
				Block b3 = read_block(b2.nex);
				b3.pre = b2.pos;
				write_block(b3);
			}
			else
				Tail = b2.pos;
			b2.key0 = *key_n_in_block((M >> 1) + 1,buffer);
			b2.elementNumber = M - (M >> 1);
			b.elementNumber = (M >> 1) + 1;
			char buffer2[Nodesize];
			fill_buffer_with_block(buffer2,b2);
			for (int i = (M >> 1) + 1; i <= M; ++i)
			{
				*data_n_in_block(i - (M >> 1) - 1,buffer2) = *data_n_in_block(i,buffer);
				*key_n_in_block(i - (M >> 1) - 1,buffer2) = *key_n_in_block(i,buffer);
			}
			write_block(b2);
			write_buffer_into_block(buffer2,b2);
			Node p = read_node(b.father);
			node_insert(p,b2.key0,b2.pos);
		}
//#ifdef DEBUG
//        std::cout << "Block after insert adjust: ";
//        for (int i = 0; i < b.elementNumber ; ++i)
//            std::cout << i << " " << *key_n_in_block(i,buffer) << " " << *data_n_in_block(i,buffer) << "\n";
//#endif
		write_buffer_into_block(buffer,b);
		write_block(b);
		return 1;
	}
	void node_insert(Node &p,const Key &key,const off_t &pos)
	{
		char buffer[Nodesize];
		fill_buffer_with_node(buffer,p);
//#ifdef DEBUG
//        std::cout << "Node before insert : ";
//        for (int i = 0; i < p.elementNumber ; ++i)
//            std::cout << i << " " << *key_n_in_node(i,buffer) << " " << *pos_n_in_node(i,buffer) << "\n";
//#endif
		int n = search_rank_in_node_lowerbound(key,p,buffer);
//		if (n >= 0 && n < p.elementNumber && *key_n_in_node(n) == key)
//			return 0;
		for (int i = p.elementNumber - 1; i > n; --i)
		{
			*pos_n_in_node(i + 1,buffer) = *pos_n_in_node(i,buffer);
			*key_n_in_node(i + 1,buffer) = *key_n_in_node(i,buffer);
		}
		*pos_n_in_node(n + 1,buffer) = pos;
		*key_n_in_node(n + 1,buffer) = key;
        if (n == -1)
        {
            Key prekey = p.key0;
            p.key0 = key;
            write_node(p);
            off_t fa_off = p.father;
            int n1 = n;
            while (n1 == -1 && fa_off != nulloff_t)
            {
                Node fa = read_node(fa_off);
                if (fa.elementNumber == 0)
                    break;
                char buffer1[Nodesize];
                fill_buffer_with_node(buffer1,fa);
                n1 = search_rank_in_node_lowerbound(prekey,fa,buffer1);
                *key_n_in_node(n1 + 1,buffer1) = key;
                write_buffer_into_node(buffer1,fa);
                if (n1 == -1)
                {
                    fa.key0 = key;
                    write_node(fa);
                    fa_off = fa.father;
                }
            }
        }
//		if (n == -1)
//        {
//		    if (p.father == nulloff_t)
//		        p.key0 = key;
//		    else
//            {
//                Node tmp = read_node(p.father);
//                char buffer1[Nodesize];
//                fill_buffer_with_node(buffer1,tmp);
//                int n1 = search_rank_in_node_lowerbound(p.key0,tmp,buffer1);
//                p.key0 = key;
//                *key_n_in_node(n1 + 1,buffer1) = key;
//                if (n1 == -1)
//                {
//                    tmp.key0 = key;
//                    write_node(tmp);
//                }
//                write_buffer_into_node(buffer1,tmp);
//            }
//        }
//#ifdef DEBUG
//        std::cout << "Node after insert : ";
//        for (int i = 0; i <= p.elementNumber ; ++i)
//            std::cout << i << " " << *key_n_in_node(i,buffer) << " " << *pos_n_in_node(i,buffer) << "\n";
//#endif
		if (++p.elementNumber > N)
		{
			Node p2 = Node(p.nodeType,*key_n_in_node((M >> 1) + 1,buffer),0,MemoryManager.malloc(Nodesize));
			p2.father = p.father;
			p2.pre = p.pos;
			p2.nex = p.nex;
			p.nex = p2.pos;
			if (p2.nex != nulloff_t)
			{
				Node p3 = read_node(p2.nex);
				p3.pre = p2.pos;
				write_node(p3);
			}
			p2.key0 = *key_n_in_node((N >> 1) + 1,buffer);
			p2.elementNumber = N - (N >> 1);
			p.elementNumber = (N >> 1) + 1;
			char buffer2[Nodesize];
			fill_buffer_with_node(buffer2,p2);
			for (int i = (N >> 1) + 1; i <= N; ++i)
			{
				*pos_n_in_node(i - (N >> 1) - 1,buffer2) = *pos_n_in_node(i,buffer);
				*key_n_in_node(i - (N >> 1) - 1,buffer2) = *key_n_in_node(i,buffer);
			}
			write_buffer_into_node(buffer2,p2);
			write_node(p2);
			if (p.father != nulloff_t)
			{
				Node pf = read_node(p.father);
				node_insert(pf,p2.key0,p2.pos);
			}
			else
			{
				Node newroot = Node(INTERNAL,p.key0,0,MemoryManager.malloc(Nodesize));
				p.father = p2.father = Root = newroot.pos;
				newroot.elementNumber = 2;
				newroot.key0 = p.key0;
				char buffer3[Nodesize];
				fill_buffer_with_node(buffer3,newroot);
				*pos_n_in_node(0,buffer3) = p.pos;
				*pos_n_in_node(1,buffer3) = p2.pos;
				*key_n_in_node(0,buffer3) = p.key0;
				*key_n_in_node(1,buffer3) = p2.key0;
				write_buffer_into_node(buffer3,newroot);
				write_node(newroot);
				write_bptree_info();
			}
		}
//#ifdef DEBUG
//        std::cout << "Node after insert adjust: ";
//        for (int i = 0; i < p.elementNumber ; ++i)
//            std::cout << i << " " << *key_n_in_node(i,buffer) << " " << *pos_n_in_node(i,buffer) << "\n";
//#endif
		write_buffer_into_node(buffer,p);
		write_node(p);
	}
	void block_rent_head(Block &outs,Block &ins,char * outsb,char * insb)
	{
		--outs.elementNumber;
		*key_n_in_block(ins.elementNumber,insb) = *key_n_in_block(0,outsb);
		*data_n_in_block(ins.elementNumber,insb) = *data_n_in_block(0,outsb);
		for (int i = 0; i < outs.elementNumber; ++i)
		{
			*key_n_in_block(i,outsb) = *key_n_in_block(i + 1,outsb);
			*data_n_in_block(i,outsb) = *data_n_in_block(i + 1,outsb);
		}
		++ins.elementNumber;
		char buffer3[Nodesize];
		Node p = read_node(ins.father);
		fill_buffer_with_node(buffer3,p);
		outs.key0 = *key_n_in_node(search_rank_in_node_lowerbound(outs.key0,p,buffer3),buffer3) = *key_n_in_block(0,outsb);
		write_block(ins);
		write_buffer_into_block(insb,ins);
		write_block(outs);
		write_buffer_into_block(outsb,outs);
		write_buffer_into_node(buffer3,p);
	}
	void block_rent_tail(Block &outs,Block &ins,char * outsb,char * insb)
	{
		--outs.elementNumber;
		for (int i = ins.elementNumber; i > 0; --i)
		{
			*key_n_in_block(i,insb) = *key_n_in_block(i - 1,insb);
			*data_n_in_block(i,insb) = *data_n_in_block(i - 1,insb);
		}
		*key_n_in_block(0,insb) = *key_n_in_block(outs.elementNumber,outsb);
		*data_n_in_block(0,insb) = *data_n_in_block(outs.elementNumber,outsb);
		++ins.elementNumber;
		char buffer3[Nodesize];
		Node p = read_node(ins.father);
		fill_buffer_with_node(buffer3,p);
		ins.key0 = *key_n_in_node(search_rank_in_node_lowerbound(ins.key0,p,buffer3),buffer3) = *key_n_in_block(0,insb);
		write_block(ins);
		write_buffer_into_block(insb,ins);
		write_block(outs);
		write_buffer_into_block(outsb,outs);
		write_buffer_into_node(buffer3,p);
	}
	void merge_block(Block &lhs,Block &rhs,char * lhsb,char *rhsb)
	{
		for (int i = 0; i < rhs.elementNumber; ++i)
		{
			*key_n_in_block(lhs.elementNumber + i,lhsb) = *key_n_in_block(i,rhsb);
			*data_n_in_block(lhs.elementNumber + i,lhsb) = *data_n_in_block(i,rhsb);
		}
		lhs.elementNumber += rhs.elementNumber;
		rhs.elementNumber = 0;
		Node p = read_node(lhs.father);
		node_erase(p,*key_n_in_block(0,rhsb));
		delete_block(rhs);
		write_block(lhs);
		write_buffer_into_block(lhsb,lhs);
	}
	void node_rent_head(Node &outs,Node &ins,char *outsb,char * insb)
	{
		--outs.elementNumber;
		*key_n_in_node(ins.elementNumber,insb) = *key_n_in_node(0,outsb);
		*pos_n_in_node(ins.elementNumber,insb) = *pos_n_in_node(0,outsb);
		for (int i = 0; i < outs.elementNumber; ++i)
		{
			*key_n_in_node(i,outsb) = *key_n_in_node(i + 1,outsb);
			*pos_n_in_node(i,outsb) = *pos_n_in_node(i + 1,outsb);
		}
		++ins.elementNumber;
		char buffer3[Nodesize];
		Node p = read_node(ins.father);
		fill_buffer_with_node(buffer3,p);
		outs.key0 = *key_n_in_node(search_rank_in_node_lowerbound(outs.key0,p,buffer3),buffer3) = *key_n_in_node(0,outsb);
		write_node(ins);
		write_buffer_into_node(insb,ins);
		write_node(outs);
		write_buffer_into_node(outsb,outs);
		write_buffer_into_node(buffer3,p);
	}
	void node_rent_tail(Node &outs,Node &ins,char *outsb,char * insb)
	{
		--outs.elementNumber;
		for (int i = ins.elementNumber; i > 0; --i)
		{
			*key_n_in_node(i,insb) = *key_n_in_node(i - 1,insb);
			*pos_n_in_node(i,insb) = *pos_n_in_node(i - 1,insb);
		}
		*key_n_in_node(0,insb) = *key_n_in_node(outs.elementNumber,outsb);
		*pos_n_in_node(0,insb) = *pos_n_in_node(outs.elementNumber,outsb);
		++ins.elementNumber;
		char buffer3[Nodesize];
		Node p = read_node(ins.father);
		fill_buffer_with_node(buffer3,p);
		ins.key0 = *key_n_in_node(search_rank_in_node_lowerbound(ins.key0,p,buffer3),buffer3) = *key_n_in_node(0,insb);
		write_node(ins);
		write_buffer_into_node(insb,ins);
		write_node(outs);
		write_buffer_into_node(outsb,outs);
		write_buffer_into_node(buffer3,p);
	}
	void merge_node(Node &lhs,Node &rhs,char *lhsb,char *rhsb)
	{
		for (int i = 0; i < rhs.elementNumber; ++i)
		{
			*key_n_in_node(lhs.elementNumber + i,lhsb) = *key_n_in_node(i,rhsb);
			*pos_n_in_node(lhs.elementNumber + i,lhsb) = *pos_n_in_node(i,rhsb);
		}
		lhs.elementNumber += rhs.elementNumber;
		rhs.elementNumber = 0;
		Node p = read_node(lhs.father);
		node_erase(p,*key_n_in_node(0,rhsb));
		delete_Node(rhs);
		write_node(lhs);
		write_buffer_into_node(lhsb,lhs);
	}
	void node_erase(Node &p,const Key &key)
	{
		char buffer[Nodesize];
		fill_buffer_with_node(buffer,p);
		int n = search_rank_in_node_lowerbound(key,p,buffer);
		assert(n != -1 && IsEqual(key,*key_n_in_node(n,buffer)));
		for (int i = n; i < p.elementNumber - 1; ++i)
		{
			*pos_n_in_node(i,buffer) = *pos_n_in_node(i + 1,buffer);
			*key_n_in_node(i,buffer) = *key_n_in_node(i + 1,buffer);
		}
		if (n == 0)
			p.key0 = *key_n_in_node(0,buffer);
		if (p.elementNumber == 1 && p.pos == Root)
		{
			if (ElementNumber == 1)
			{
				this->Clear();
				return;
			}
			Root = *pos_n_in_node(0,buffer);
			delete_Node(p);
			return;
		}
		if (--p.elementNumber < (N + 1) >> 1 && ElementNumber > N)
		{
			if (p.nex == nulloff_t)
			{
				Node p2 = read_node(p.pre);
				char buffer2[Nodesize];
				fill_buffer_with_node(buffer2,p2);
				if (p2.elementNumber > (M + 1) >> 1)
					node_rent_tail(p2,p,buffer2,buffer);
				else
					merge_node(p2,p,buffer2,buffer);
				return;
			}
			if (p.pre == nulloff_t)
			{
				Node p2 = read_node(p.nex);
				char buffer2[Nodesize];
				fill_buffer_with_node(buffer2,p2);
				if (p2.elementNumber > (M + 1) >> 1)
					node_rent_head(p2,p,buffer2,buffer);
				else
					merge_node(p2,p,buffer2,buffer);
				return;
			}
			Node p2 = read_node(p.pre);
			if (p2.elementNumber > (N + 1) >> 1)
			{
				char buffer2[Nodesize];
				fill_buffer_with_node(buffer2,p2);
				node_rent_tail(p2,p,buffer2,buffer);
				return;
			}
			Node p3 = read_node(p.nex);
			if (p3.elementNumber > (N + 1) >> 1)
			{
				char buffer3[Nodesize];
				fill_buffer_with_node(buffer3,p3);
				node_rent_head(p3,p,buffer3,buffer);
				return;
			}
			if (p2.father == p.father)
			{
				char buffer2[Nodesize];
				fill_buffer_with_node(buffer2,p2);
				merge_node(p2,p,buffer2,buffer);
				return;
			}
			else
			{
				char buffer3[Nodesize];
				fill_buffer_with_node(buffer3,p3);
				merge_node(p,p3,buffer,buffer3);
				return;
			}
		}
	}
	int block_erase(Block &b,const Key &key)
	{
		char buffer[Nodesize];
		fill_buffer_with_block(buffer,b);
		int n = search_rank_in_block_lowerbound(key,b,buffer);
		if (n == b.elementNumber - 1 || !IsEqual(key,*key_n_in_block(n + 1,buffer)))
			return 0;
		for (int i = n + 1; i < b.elementNumber - 1; ++i)
		{
			*data_n_in_block(i,buffer) = *data_n_in_block(i + 1,buffer);
			*key_n_in_block(i,buffer) = *key_n_in_block(i + 1,buffer);
		}
        if (n == -1)
        {
            Key prekey = b.key0;
            Key key_changeto = b.key0 = *key_n_in_block(0,buffer);
            write_block(b);
            off_t fa_off = b.father;
            int n1 = n;
            while (n1 == -1)
            {
                Node fa = read_node(fa_off);
                if (fa.elementNumber == 0)
                    break;
                char buffer1[Nodesize];
                fill_buffer_with_node(buffer1,fa);
                n1 = search_rank_in_node_lowerbound(prekey,fa,buffer1);
                *key_n_in_node(n1 + 1,buffer1) = key_changeto;
                write_buffer_into_node(buffer1,fa);
                if (n1 == -1)
                {
                    fa.key0 = key;
                    write_node(fa);
                    fa_off = fa.father;
                    if (fa_off == nulloff_t)
                        break;
                }
            }
        }
//		if (n == -1)
//        {
//			b.key0 = *key_n_in_block(0,buffer);
//		    //tobe change
//        }
		if (--b.elementNumber < (M + 1) >> 1 && ElementNumber > M)
		{
			if (b.nex == nulloff_t)
			{
				Block b2 = read_block(b.pre);
				char buffer2[Nodesize];
				fill_buffer_with_block(buffer2,b2);
				if (b2.elementNumber > (M + 1) >> 1)
					block_rent_tail(b2,b,buffer2,buffer);
				else
					merge_block(b2,b,buffer2,buffer);
				return 1;
			}
			if (b.pre == nulloff_t)
			{
				Block b2 = read_block(b.nex);
				char buffer2[Nodesize];
				fill_buffer_with_block(buffer2,b2);
				if (b2.elementNumber > (M + 1) >> 1)
					block_rent_head(b2,b,buffer2,buffer);
				else
					merge_block(b2,b,buffer2,buffer);
				return 1;
			}
			Block b2 = read_block(b.pre);
			if (b2.elementNumber > (M + 1) >> 1)
			{
				char buffer2[Nodesize];
				fill_buffer_with_block(buffer2,b2);
				block_rent_tail(b2,b,buffer2,buffer);
				return 1;
			}
			Block b3 = read_block(b.nex);
			if (b3.elementNumber > (M + 1) >> 1)
			{
				char buffer3[Nodesize];
				fill_buffer_with_block(buffer3,b3);
				block_rent_head(b3,b,buffer3,buffer);
				return 1;
			}
			if (b2.father == b.father)
			{
				char buffer2[Nodesize];
				fill_buffer_with_block(buffer2,b2);
				merge_block(b2,b,buffer2,buffer);
				return 1;
			}
			else
			{
				char buffer3[Nodesize];
				fill_buffer_with_block(buffer3,b3);
				merge_block(b,b3,buffer,buffer3);
				return 1;
			}
		}
		write_block(b);
		write_buffer_into_block(buffer,b);
		return 1;
	}
	int find_subtree(const Key &key,Data &data,off_t nowroot)
	{
		Node p = read_node(nowroot);
		char buffer[Nodesize];
		fill_buffer_with_node(buffer,p);
		int n = search_rank_in_node_lowerbound(key,p,buffer);
        if (n + 1 < p.elementNumber && IsEqual(key,*key_n_in_node(n + 1,buffer)))
            ++n;
        if (n == -1)
            return 0;
		if (p.nodeType == LEAF)
		{
			Block b = read_block(*pos_n_in_node(n,buffer));
			fill_buffer_with_block(buffer,b);
			int n = search_rank_in_block_lowerbound(key,b,buffer);
			if (n == b.elementNumber - 1)
			    return 0;
            if (IsEqual(key,*key_n_in_block(n + 1,buffer)))
            {
                data = *data_n_in_block(n + 1,buffer);
                return 1;
            }
            else
                return 0;
		}
		else
			return find_subtree(key,data,*pos_n_in_node(n,buffer));
	}
	int modify_subtree(const Key &key,const Data &data,off_t nowroot)
	{
		Node p = read_node(nowroot);
		char buffer[Nodesize];
		fill_buffer_with_node(buffer,p);
		int n = search_rank_in_node_lowerbound(key,p,buffer);
		if (n + 1 < p.elementNumber && IsEqual(key,*key_n_in_node(n + 1,buffer)))
			++n;
		if (n == -1)
			return 0;
		if (p.nodeType == LEAF)
		{
			Block b = read_block(*pos_n_in_node(n,buffer));
			fill_buffer_with_block(buffer,b);
			int n = search_rank_in_block_lowerbound(key,b,buffer);
			if (n == b.elementNumber - 1)
				return 0;
			if (IsEqual(key,*key_n_in_block(n + 1,buffer)))
			{
				*data_n_in_block(n + 1,buffer) = data;
				write_buffer_into_block(buffer,b);
				return 1;
			}
			else
				return 0;
		}
		else
			return modify_subtree(key,data,*pos_n_in_node(n,buffer));
	}
	int insert_subtree(const Key &key,const Data &data,Node &root,off_t pa)
	{
	    root.father = pa;
	    write_node(root);
		char buffer[Nodesize];
		fill_buffer_with_node(buffer,root);
		int n = search_rank_in_node_lowerbound(key,root,buffer);
		if (n + 1 >= 0 && n + 1 < root.elementNumber && IsEqual(key,*key_n_in_node(n + 1,buffer)))
			return 0;
		if (root.nodeType == LEAF)
		{
			Block b;
			if (n == -1)
				b = read_block(*pos_n_in_node(0,buffer));
			else
				b = read_block(*pos_n_in_node(n,buffer));
			b.father = root.pos;
			int tmp = block_insert(b,key,data);
			root.elementNumber += tmp;
			return tmp;
		}
		else
		{
            Node p;
		    if (n == -1)
		        p = read_node(*pos_n_in_node(0,buffer));
		    else
			    p = read_node(*pos_n_in_node(n,buffer));
			int tmp = insert_subtree(key,data,p,root.pos);
			root.elementNumber += tmp;
			return tmp;
		}
	}
	int erase_subtree(const Key &key,Node root)
	{
		char buffer[Nodesize];
		fill_buffer_with_node(buffer,root);
		int n = search_rank_in_node_lowerbound(key,root,buffer);
        if (n + 1 < root.elementNumber && IsEqual(key,*key_n_in_node(n + 1,buffer)))
            ++n;
		if (root.nodeType == LEAF)
		{
			if (n == -1)
				return 0;
			else
			{
				Block b = read_block(*pos_n_in_node(n,buffer));
				return block_erase(b,key);
			}
		}
		else
			return erase_subtree(key,read_node(*pos_n_in_node(n,buffer)));
	}
	void search_arr_in_subtree(off_t &larr,off_t &rarr,size_t &ln,size_t &rn,const Key &key,std::function<bool(const Key &,const Key &)>CMP,Node &p)
    {
        char buffer[Nodesize];
        fill_buffer_with_node(buffer,p);
        larr = search_rank_in_node_lowerbound(key,p,buffer,CMP);
        rarr = search_rank_in_node_upperbound(key,p,buffer,CMP);
        if (larr == -1 &&rarr == -1){
            rn = ln = nulloff_t;
            return;
        }
        if (p.nodeType == LEAF)
        {
            if (larr == rarr)
            {
                Block b =read_block(*pos_n_in_node(larr,buffer));
                char buffer2[Nodesize];
                fill_buffer_with_block(buffer2,b);
                ln = search_rank_in_block_lowerbound(key,b,buffer2,CMP);
                rn = search_rank_in_block_upperbound(key,b,buffer2,CMP);
                return;
            }
            else
            {

                Block b2 =read_block(*pos_n_in_node(larr,buffer));
                char buffer2[Nodesize];
                fill_buffer_with_block(buffer2,b2);
                ln = search_rank_in_block_lowerbound(key,b2,buffer2,CMP);
                if (rarr == nulloff_t)
                    rn = nulloff_t;
                else
                {
                    Block b3 =read_block(*pos_n_in_node(rarr,buffer));
                    char buffer3[Nodesize];
                    fill_buffer_with_block(buffer3,b3);
                    rn = search_rank_in_block_lowerbound(key,b3,buffer3,CMP);
                }
                return;
            }
        }
        else
        {
            Node p2 = read_node(*pos_n_in_node(larr,buffer));
            if (larr == rarr)
            {
                search_arr_in_subtree(larr,rarr,ln,rn,key,CMP,p2);
                return;
            }
            off_t llarr,rrarr;
            size_t lln,rrn;
            search_arr_in_subtree(llarr,rrarr,lln,rrn,key,CMP,p2);
            larr = llarr,ln = lln;
            if (rarr == nulloff_t)
            {
                rn = nulloff_t;
                return;
            }
            p2 = read_node(*pos_n_in_node(rarr,buffer));
            search_arr_in_subtree(llarr,rrarr,lln,rrn,key,CMP,p2);
            rarr = rrarr,rn = rrn;
            return;
        }
    }
    int askarr_subtree(const Key &key,std::function<bool(const Key &,const Key &)>CMP,sjtu::vector<Key> &vect1,sjtu::vector<Data> &vect2,Node &p)
    {
//#ifdef DEBUG
//        std::cout << "key.fisrt,key.,second,nkey.first,nkey.second" << key.first << " " << key.second << " " << p.key0.first << " " << p.key0.second <<"\n";
//        std::cout << "p.nodeType "<< p.nodeType <<"\n";
//#endif
        char buffer[Nodesize];
        fill_buffer_with_node(buffer,p);
        int n = search_rank_in_node_lowerbound(key,p,buffer,CMP);
//        if (n + 1 < p.elementNumber && !CMP(key,*key_n_in_node(n + 1,buffer)) && !CMP(*key_n_in_node(n + 1,buffer),key))
//            ++n;
        if (n == -1 && !(!CMP(key,*key_n_in_node(n + 1,buffer)) && !CMP(*key_n_in_node(n + 1,buffer),key)))
            return 0;
		else
		{
			if (n == -1)
				n = 0;
		}
        if (p.nodeType == LEAF)
        {
            Block b = read_block(*pos_n_in_node(n,buffer));
            fill_buffer_with_block(buffer,b);
            int n = search_rank_in_block_lowerbound(key,b,buffer,CMP);
            if (n == b.elementNumber - 1)
			{
            	if (b.nex == nulloff_t)
            		return 0;
				b = read_block(b.nex);
				fill_buffer_with_block(buffer,b);
				n = -1;
			}
            if (!CMP(key,*key_n_in_block(n + 1,buffer)) && !CMP(*key_n_in_block(n + 1,buffer),key))
            {
                Block nb = b;
                off_t nele = n + 1;
                char * nBuffer = buffer;
//                #ifdef DEBUG
//                std::cout << "key.fisrt,key.,second,nkey.first,nkey.second,data:\n" << key.first << " " << key.second << " " << (*key_n_in_block(nele,nBuffer)).first << " " << (*key_n_in_block(nele,nBuffer)).second << " " << *data_n_in_block(nele,nBuffer) << "\n";
//#endif
                while (!CMP(key,*key_n_in_block(nele,nBuffer)) && !CMP(*key_n_in_block(nele,nBuffer),key))
                {
//#ifdef DEBUG
//                    std::cout << "key" <<  (*key_n_in_block(nele,nBuffer)).first << " " << (*key_n_in_block(nele,nBuffer)).second;
//                    #endif
                    vect1.push_back(*key_n_in_block(nele,nBuffer));
//					#ifdef DEBUG
//					std::cout << "data:\n" << *data_n_in_block(nele,nBuffer) << "\n";
//                	#endif
                    vect2.push_back(*data_n_in_block(nele,nBuffer));
//#ifdef DEBUG
//                    std::cout << "!!!? " << vect2[0] << "\n";
//#endif
                    ++nele;
                    if (nele == nb.elementNumber)
                    {
                        if (nb.nex != nulloff_t)
                        {
                            nb = read_block(nb.nex);
                            fill_buffer_with_block(nBuffer,nb);
                            nele = 0;
                        }
                        else
                            break;
                    }
                }
                return 1;
            }
            else
                return 0;
        }
        else
        {
            Node son = read_node(*pos_n_in_node(n,buffer));
            return askarr_subtree(key,CMP,vect1,vect2,son);
        }
//        off_t bg,ed;
//        size_t bgn,edn;
//        search_arr_in_subtree(bg,ed,bgn,edn,key,CMP,p);
//        if (bg == ed && bgn == edn)
//            return 0;
//        Block b = read_block(bgn);
//        char buffer2[Nodesize];
//        fill_buffer_with_block(buffer2,b);
//        if (bg == ed)
//        {
//            for (size_t i = 0; i + bgn != edn; ++i)
//            {
//                vect1.push_back(*key_n_in_block(i + bgn,buffer2));
//                vect2.push_back(*data_n_in_block(i + bgn,buffer2));
//            }
//            return 1;
//        }
//        for (size_t i = 0; i + bgn != b.elementNumber; ++i)
//        {
//            vect1.push_back(*key_n_in_block(i + bgn,buffer2));
//            vect2.push_back(*data_n_in_block(i + bgn,buffer2));
//        }
//        while (b.nex != ed)
//        {
//            b = read_block(b.nex);
//            char buffer2[Nodesize];
//            fill_buffer_with_block(buffer2,b);
//            for (size_t i = 0; i < b.elementNumber; ++i)
//            {
//                vect1.push_back(*key_n_in_block(i + bgn,buffer2));
//                vect2.push_back(*data_n_in_block(i + bgn,buffer2));
//            }
//        }
//        if (ed != nulloff_t)
//        {
//            b = read_block(ed);
//            char buffer2[Nodesize];
//            fill_buffer_with_block(buffer2,b);
//            for (size_t i = 0; i < edn; ++i)
//            {
//                vect1.push_back(*key_n_in_block(i + bgn,buffer2));
//                vect2.push_back(*data_n_in_block(i + bgn,buffer2));
//            }
//        }
//        return 1;
    }
public:
	BPlusTree(const std::string &FName,const std::string &MMFName){
		MemoryManager.SetName(MMFName);
		FileName = FName;
		File.open(FName,std::ios::in|std::ios::binary);
		if (!File.is_open())
		{
			File.close();
			File.open(FName,std::ios::in|std::ios::out|std::ios::binary);
			MemoryManager.write_info();
			Clear();
		}
		else
		{
			MemoryManager.read_info();
			read_bptree_info();
			File.close();
			File.open(FName,std::ios::in|std::ios::out|std::ios::binary);
		}
	}
	~BPlusTree(){
		MemoryManager.write_info();
		write_bptree_info();
		File.close();
	}

	int Find(const Key &key,Data &data)
	{
		if (Root == nulloff_t)
			return 0;
		return find_subtree(key,data,Root);
	}
	int Erase(const Key &key)
	{
		if (Root == nulloff_t)
			return 0;
        Node p = read_node(Root);
        int res = erase_subtree(key,p);
		ElementNumber -= res;
		return res;
	}

	int Insert(const Key &key,const Data &data)
	{
		if(Root == nulloff_t)
		{
			Node p = Node(LEAF,key,0,MemoryManager.malloc(Nodesize));
			Block b = Block(0,key,MemoryManager.malloc(Nodesize));
			b.father = Root = p.pos;
			block_insert(b,key,data);
			node_insert(p,key,b.pos);
			Head = Tail = b.pos;
			write_bptree_info();
			ElementNumber = 1;
			return 1;
		}
        Node p = read_node(Root);
//#ifdef DEBUG
//        std::cout << "NdTp" << p.nodeType << "\n";
//#endif
		int tmp = insert_subtree(key,data,p,nulloff_t);
		ElementNumber += tmp;
		write_bptree_info();
		return tmp;
	}
	int Modify(const Key &key,const Data &data)
	{
		if (Root == nulloff_t)
			return 0;
		return modify_subtree(key,data,Root);
	}
	int AskArr(const Key &key,std::function<bool(const Key &,const Key &)>CMP,sjtu::vector<Key> &vect1,sjtu::vector<Data> &vect2)
	{
        if (Root == nulloff_t)
            return 0;
        Node p = read_node(Root);
        return askarr_subtree(key,CMP,vect1,vect2,p);
	}

	int Size()
    {
	    return ElementNumber;
    }

	void Clear()
	{
		File.close();
		File.open(FileName,std::ios::binary|std::ios::in|std::ios::out|std::ios::trunc);
		MemoryManager.clear();
		MemoryManager.write_info();
		Root = Head = Tail = nulloff_t;
		ElementNumber = 0;
		write_bptree_info();
	}

#ifdef DEBUG
	void printinfo()
	{
		std::cout << "NowBptreeInfo\n";
		std::cout << Root << " " << Head << " " << Tail << " " << ElementNumber << "\n";
		std::cout.flush();
	}
#endif
};

#endif //AKINA_BPLUSTREE_HPP
