//
// Created by 18146 on InitId/5/4.
//

#ifndef AKINA_USERMANANGER_HPP
#define AKINA_USERMANANGER_HPP
#include "FileManager.hpp"
#include "BasicStruct.hpp"
class UserManager
{
private:
	FileManager<USER> File;
	const int InitId=2019;
public:
	size_t Siz;
	UserManager(){File.SetName("UserRecord.bin");Siz=File.size();}
	~UserManager()=default;
	size_t Register(USER &User)
	{
		User.Priv=1+(Siz==0);
		File.Write(&User,++Siz);
		return Siz+InitId-1;
	}
	int Login(const USER &User,size_t Id)
	{
		if (Id<=(InitId-1)||Id>(InitId-1)+Siz) return 0;
		USER X;
		File.Read(&X,Id-(InitId-1));
		return (X.Passwd==User.Passwd);
	}
	int Query(USER &User,size_t Id)
	{
		if (Id<=(InitId-1)||Id>(InitId-1)+Siz) return 0;
		File.Read(&User,Id-(InitId-1));
		return 1;
	}
	int Modify_Profile(USER &User,size_t Id)
	{
		if (Id<=(InitId-1)||Id>(InitId-1)+Siz) return 0;
		USER X;
		File.Read(&X,Id-(InitId-1));
		User.Priv=X.Priv;
		File.Write(&User,Id-(InitId-1));
		return 1;
	}
	int Modify_Privilege(size_t Id1,size_t Id2,int Priv)
	{
		USER x1,x2;
		if (Id1<=(InitId-1)||Id1>(InitId-1)+Siz) return 0;
		if (Id2<=(InitId-1)||Id2>(InitId-1)+Siz) return 0;
		File.Read(&x1,Id1-(InitId-1));
		File.Read(&x2,Id2-(InitId-1));
		//std::cout<<x1.Priv<<x2.Priv;
		if (x1.Priv<2||x2.Priv>Priv) return 0;
		x2.Priv=Priv;
		File.Write(&x2,Id2-(InitId-1));
		return 1;
	}
	void Clear()
	{
		File.Clear();
		Siz=0;
	}
};


#endif //AKINA_USERMANANGER_HPP
