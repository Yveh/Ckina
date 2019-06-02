//
// Created by 18146 on 2019/5/4.
//

#ifndef AKINA_COMMAND_HPP
#define AKINA_COMMAND_HPP
#include "BasicStruct.hpp"
#include "UserMananger.hpp"
#include "TrainManager.hpp"
void Main_Command(std::istream &is,std::ostream &os,UserManager &MainUser,TrainManager &MainTrain,bool &Exit)
{String<20> Cmd;
	is>>Cmd;
	if (Cmd=="exit")
	{
		Exit=false;
		os<<"BYE\n";
		return;
	}
	if (Cmd=="clean")
	{
		MainUser.Clear();
		MainTrain.Clear();
		os<<"1\n";
		return;
	}
	if (Cmd=="login")
	{
		size_t Id;
		USER User;
		is>>Id>>User.Passwd;
		os<<MainUser.Login(User,Id)<<"\n";
		return;
	}
	if (Cmd=="register")
	{
		USER User;
		is>>User.Name>>User.Passwd>>User.Email>>User.Phone;
		//os<<User.Name<<User.Email<<User.Phone<<User.Priv<<"\n";
		os<<MainUser.Register(User)<<"\n";
		return;
	}
	if (Cmd=="query_profile")
	{
		USER User;
		size_t Id;
		is>>Id;
		if (!MainUser.Query(User,Id)) os<<"0\n";
		else os<<User.Name<<User.Email<<User.Phone<<User.Priv<<"\n";
		return;
	}
	if (Cmd=="modify_profile")
	{
		USER User;
		size_t Id;
		is>>Id>>User.Name>>User.Passwd>>User.Email>>User.Phone;
		os<<MainUser.Modify_Profile(User,Id)<<"\n";
	}
	if (Cmd=="modify_privilege")
	{
		size_t Id1,Id2;int Priv;
		is>>Id1>>Id2>>Priv;
		os<<MainUser.Modify_Privilege(Id1,Id2,Priv)<<"\n";
		return;
	}
 	if (Cmd=="add_train")
	{
		TrainValue Value;
		String<20> Id;
		is>>Id;
		Value.ReadTrain(is);
		os<<MainTrain.AddTrain(Id,Value)<<"\n";
		return;
	}
	if (Cmd=="modify_train")
	{
		TrainValue Value;
		String<20> Id;
		is>>Id;
		Value.ReadTrain(is);
		os<<MainTrain.ModTrain(Id,Value)<<"\n";
		return;
	}
	if (Cmd=="query_train")
	{
		TrainValue Value;
		String<20> Id;
		is>>Id;
		if (MainTrain.QueryTrain(Id,Value)==0) os<<"0\n";
		else if (Value.Leftpos[0]<0) os<<"0\n";
		else {os<<Id;Value.WriteTrain(os);}
		return;
	}
	if (Cmd=="delete_train")
	{
		String<20> Id;
		is>>Id;
		os<<MainTrain.DelTrain(Id)<<"\n";
		return;
	}
	if (Cmd=="sale_train")
	{
		String<20> Id;
		is>>Id;
		os<<MainTrain.SaleTrain(Id)<<"\n";
		return;
	}
	if (Cmd=="buy_ticket")
	{
		Iticket x;String<20> y;
		is>>x.UserId>>x.Num>>x.TrainId>>x.Loc1>>x.Loc2>>y>>x.Kind;
		x.Date=y.ToDate();
		os<<MainTrain.BuyTicket(x,MainUser.Siz<1000)<<"\n";
		return;
	}
	if (Cmd=="refund_ticket")
	{
		Iticket x;String<20> y;
		is>>x.UserId>>x.Num>>x.TrainId>>x.Loc1>>x.Loc2>>y>>x.Kind;
		x.Date=y.ToDate();
		os<<MainTrain.RefundTicket(x)<<"\n";
		return;
	}
	if (Cmd=="query_order")
	{
		size_t UId;String<20> y;String<20> Cat;
		is>>UId>>y>>Cat;
		MainTrain.QueryOrder(UId,y.ToDate(),Cat,os);
		return;
	}
	if (Cmd=="query_ticket")
	{
		String<20> l1,l2,sp,Cat;
		is>>l1>>l2>>sp>>Cat;
		MainTrain.QueryTicket(l1,l2,sp.ToDate(),Cat,os);
		return;
	}
	if (Cmd=="query_transfer")
	{
		String<20> l1,l2,sp,Cat;
		is>>l1>>l2>>sp>>Cat;
		MainTrain.QueryTrans(l1,l2,sp.ToDate(),Cat,os);
		return;
	}
}


#endif //AKINA_COMMAND_HPP
