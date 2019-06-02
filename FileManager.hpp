//
// Created by 18146 on 2019/5/4.
//

#ifndef AKINA_FILEMANAGER_HPP
#define AKINA_FILEMANAGER_HPP
#include <fstream>
#include <stdio.h>
#include <cstring>
template <class T>
class FileManager
{
private:
	std::string FileName;
	std::fstream File;
public:
	FileManager()=default;
	void SetName(const std::string FName)
	{
		FileName=FName;
		File.open(FileName,std::fstream::in);
		if (!File) File.open(FileName,std::ios::binary|std::ios_base::out);
		File.close();
		File.open(FileName,std::ios::binary|std::ios::in|std::ios::out);
	}
	~FileManager(){File.close();}
	size_t size()
	{
		File.seekg(0,std::ios::end);
		return File.tellg()/sizeof(T);
	}
	void Read(T* TPointer,long long pos)
	{
		File.seekg(sizeof(T)*(pos-1),std::ios::beg);
		File.read((reinterpret_cast<char * > (TPointer)), sizeof(T));
	}
	void AllRead(T* TPointer,long long pos,size_t len)
	{
		File.seekg(sizeof(T)*(pos-1),std::ios::beg);
		File.read((reinterpret_cast<char * > (TPointer)), sizeof(T)*len);
	}
	void Write(const T* TPointer,long long pos)
	{
		File.seekp(sizeof(T)*(pos-1),std::ios::beg);
		File.write((reinterpret_cast<const char * > (TPointer)), sizeof(T));
	}
	void AlWrite(const T* TPointer,long long pos,size_t len)
	{
		File.seekp(sizeof(T)*(pos-1),std::ios::beg);
		File.write((reinterpret_cast<const char * > (TPointer)), sizeof(T)*len);
	}

	long Push_Back(const T* TPointer,size_t len)
	{
		File.seekp(0,std::ios::end);
		File.write((reinterpret_cast<const char * > (TPointer)), sizeof(T)*len);
		return File.tellp()/sizeof(T);
	}

	void Clear()
	{
		File.close();
		File.open(FileName,std::ios::binary|std::ios::in|std::ios::out|std::ios::trunc);
	}
};
#endif //AKINA_FILEMANAGER_HPP
