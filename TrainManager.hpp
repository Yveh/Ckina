//
// Created by 18146 on 2019/5/14.
//

#ifndef AKINA_TRAINMANAGER_HPP
#define AKINA_TRAINMANAGER_HPP
#include "FileManager.hpp"
#include "BPlusTree.hpp"
class TrainManager
{
private:
	BPlusTree<String<20>,TrainValue,20480> AlTrain;
	BPlusTree<StationKey,short,4096> LocToId;
	BPlusTree<UTicketKey,short,4096> UserTicket;
	FileManager<short> LeftTicket;
	long LSiz;
	void ShowTicket_Bought(const UTicketKey &k,short num,std::ostream &os)
	{
		TrainValue t;
		AlTrain.Find(k.TrainId,t);
		os<<k.TrainId<<t.Loc[k.l1]<<StrDate(k.Date)<<StrTime(t.Time2[k.l1]);
		double pri[5]={0};
		short plus=0,mk=t.Time2[k.l1];
		for (short i=k.l1+1;i<=k.l2;++i)
		{
			for (short j=0;j<t.KindNum;++j) pri[j]+=t.Price[i*5+j];
			if (t.Time1[i]<mk) ++plus;mk=t.Time1[i];
			if (t.Time2[i]<mk) ++plus;mk=t.Time2[i];
		}
		os<<t.Loc[k.l2]<<StrDate(k.Date+plus)<<StrTime(t.Time1[k.l2]);
		for (short j=0;j<t.KindNum;++j)
		{
			os<<t.TicketKind[j];
			if (j==k.Kind) os<<num;else os<<0;
			os<<" "<<pri[j]<<" ";
		}
		os<<'\n';
	}
	void ShowTicket_q(UTicketKey &k,std::ostream &os)
	{
		TrainValue t;
		AlTrain.Find(k.TrainId,t);
		double pri[5]={0};short lef[5]={0};
		if (t.Leftpos[k.Date]>0)
        {
            short a[t.LocNum*t.KindNum];
            LeftTicket.AllRead(a,t.Leftpos[k.Date],t.LocNum*t.KindNum);
            for (short i=k.l1+1;i<=k.l2;++i) {
                for (short j = 0; j < t.KindNum; ++j) {
                    if (a[j * t.LocNum + i] > lef[j])lef[j] = a[j * t.LocNum + i];
                }
            }
        }
		
		short plus=0,mk=t.Time2[0];
        for (short i=1;i<=k.l1;++i)
        {
            if (t.Time1[i]<mk) ++plus;mk=t.Time1[i];
            if (t.Time2[i]<mk) ++plus;mk=t.Time2[i];
        }
        os<<k.TrainId<<t.Loc[k.l1]<<StrDate(k.Date+plus)<<StrTime(t.Time2[k.l1]);
		for (short i=k.l1+1;i<=k.l2;++i)
		{
			for (short j=0;j<t.KindNum;++j)
			{
				pri[j] += t.Price[i * 5 + j];
			}
            if (t.Time1[i]<mk) ++plus;mk=t.Time1[i];
            if (t.Time2[i]<mk) ++plus;mk=t.Time2[i];
		}
		os<<t.Loc[k.l2]<<StrDate(k.Date+plus)<<StrTime(t.Time1[k.l2]);
		for (short j=0;j<t.KindNum;++j)
		{
			os<<t.TicketKind[j]<<2000-lef[j]<<" "<<pri[j]<<" ";
		}
		os<<'\n';

	};
	void ShowTicket_Trans(Trans x,std::ostream &os)
	{
		UTicketKey k;
		k.TrainId=x.TrainId1;k.l1=x.l11;k.l2=x.l12;k.Date=x.Date;
		ShowTicket_q(k,os);
		k.TrainId=x.TrainId2;k.l1=x.l21;k.l2=x.l22;k.Date=x.Date;
		ShowTicket_q(k,os);
	}

	long Delta(const Trans &k,TrainValue &t1,TrainValue &t2)
	{
		long ans=0;
		short mk=t1.Time2[k.l11];
		for (short i=k.l11+1;i<=k.l12;++i)
		{
			if (t1.Time1[i]<mk) ans+=86400;ans+=t1.Time1[i]-mk;mk=t1.Time1[i];
			if (t1.Time2[i]<mk) ans+=86400;ans+=t1.Time2[i]-mk;mk=t1.Time2[i];
		}
		for (short i=k.l21;i<=k.l12;++i)
		{
			if (t2.Time1[i]<mk) ans+=86400;ans+=t2.Time1[i]-mk;mk=t2.Time1[i];
			if (t2.Time2[i]<mk) ans+=86400;ans+=t2.Time2[i]-mk;mk=t2.Time2[i];
		}
		return ans;
	}
public:
	TrainManager():AlTrain("AlTrain.bin","_AlTrain.bin"),LocToId("LocToId.bin","_LocToId.bin"),UserTicket("UserTicket.bin","_UserTicket.bin")
	{
		LeftTicket.SetName("LeftTicket.bin");
		LSiz=LeftTicket.size();
	}

	~TrainManager() = default;
	int AddTrain(const String<20> &Id,TrainValue &Train)
	{
        Train.Leftpos[0]=-1;
		if (AlTrain.Insert(Id,Train))return 1;
		TrainValue t2;
		AlTrain.Find(Id,t2);
		if(t2.Leftpos[0]==-2) {AlTrain.Modify(Id,Train);return 1;}
		return 0;
	}
	int ModTrain(const String<20> &Id,TrainValue &Train)
	{
		TrainValue xTrain;
		if (AlTrain.Find(Id,xTrain)==0)  return 0;
		if (xTrain.Leftpos[0]!=-1) return 0;
		Train.Leftpos[0]=-1;
		return AlTrain.Modify(Id,Train);
	}
	int QueryTrain(const String<20> &Id,TrainValue &Train)
	{
		return AlTrain.Find(Id,Train);
	}
	int DelTrain(const String<20> &Id)
	{
		TrainValue Train;
		if (AlTrain.Find(Id,Train)==0)  return 0;
		if (Train.Leftpos[0]>=0) return 0;
		Train.Leftpos[0]=-2;
        AlTrain.Modify(Id,Train);
		return 1;
		//todo:find in tree 2 times for the same train?
	}
	int SaleTrain(const String<20> &Id)
	{
		TrainValue Train;
		if (AlTrain.Find(Id,Train)==0)  return 0;
		if (Train.Leftpos[0]>=0) return 0;
	//leftpos+kindpos*locnum=this kind of ticket left.
	    Train.Leftpos[0]=1;
	    for (int i=1;i<=30;++i) Train.Leftpos[i]=0;
		StationKey x;x.TrainId=Id;x.Catalog=Train.Catalog;
		for (short i=0;i<Train.LocNum;i++)
		{
			x.Loc=Train.Loc[i];
			LocToId.Insert(x,i);
		}

		AlTrain.Modify(Id,Train);

		return 1;
		//todo:find in tree 2 times for the same train?
	}
	int BuyTicket(Iticket &x,bool flag)
	{
		TrainValue Train;
		if (AlTrain.Find(x.TrainId,Train)==0)  return 0;
		if (Train.Leftpos[0]<0) return 0;
		UTicketKey key;key.Date=x.Date;
		short i=0;
		for (i=0;i<Train.KindNum;i++) if (Train.TicketKind[i]==x.Kind) {key.Kind=i;break;}
		if (i==Train.KindNum) return 0;
		for (i=0;i<Train.LocNum;i++)
		{
			if (Train.Loc[i]==x.Loc1) key.l1=i;
			else if (Train.Loc[i]==x.Loc2) key.l2=i;
		}//todo:probably faster but n=60 so not important.
		if (key.l1>=key.l2) return 0;
		//check:enough tickets left
		if (Train.Leftpos[key.Date]>0)
        {
            short a[Train.LocNum];
            LeftTicket.AllRead(a,Train.Leftpos[key.Date]+key.Kind*Train.LocNum,Train.LocNum);
            for (int i=key.l1+1;i<=key.l2;++i) if (a[i]+x.Num>2000) return 0;
            for (int i=key.l1+1;i<=key.l2;++i) a[i]+=x.Num;
            LeftTicket.AlWrite(a,Train.Leftpos[key.Date]+key.Kind*Train.LocNum,Train.LocNum);
        } else
            {
                Train.Leftpos[key.Date]=LSiz+1;
                short a[Train.LocNum*Train.KindNum];
                for (int i=0;i<Train.LocNum*Train.KindNum;i++) a[i]=0;
                for (int i=key.Kind*Train.LocNum+key.l1+1;
                i<=key.Kind*Train.LocNum+key.l2;++i) a[i]=x.Num;
                LSiz=LeftTicket.Push_Back(a,Train.LocNum*Train.KindNum);
                AlTrain.Modify(x.TrainId,Train);
            }

		//set values in UTicket
		if(flag)
		{
			key.UserId=x.UserId;key.Date=x.Date;key.TrainId=x.TrainId;key.Catalog=Train.Catalog;
			UserTicket.Insert(key,x.Num);

		}
		return 1;
	}
		int RefundTicket(Iticket &x)
	{
		TrainValue Train;
		if (AlTrain.Find(x.TrainId,Train)==0)  return 0;
		if (Train.Leftpos[0]<0) return 0;
		UTicketKey key;key.Date=x.Date;
		short i=0;
		for (i=0;i<Train.KindNum;i++) if (Train.TicketKind[i]==x.Kind) {key.Kind=i;break;}
		if (i==Train.KindNum) return 0;
		key.l1=66;key.l2=-1;
		for (i=0;i<Train.LocNum;i++)
		{
			if (Train.Loc[i]==x.Loc1) key.l1=i;
			else if (Train.Loc[i]==x.Loc2) key.l2=i;
		}//todo:probably faster but n=60 so not important.
		if (key.l1>=key.l2) return 0;
		key.UserId=x.UserId;key.Date=x.Date;key.TrainId=x.TrainId;key.Catalog=Train.Catalog;
		short y=-1;
		UserTicket.Find(key,y);if (y<x.Num) return 0;
		//refund in LeftTicket
		short a[Train.LocNum];
		LeftTicket.AllRead(a,Train.Leftpos[key.Date]+key.Kind*Train.LocNum,Train.LocNum);
		for (int i=key.l1+1;i<=key.l2;++i) a[i]-=x.Num;
		LeftTicket.AlWrite(a,Train.Leftpos[key.Date]+key.Kind*Train.LocNum,Train.LocNum);
		//set values in UTicket
		UserTicket.Modify(key,y-x.Num);
		return 1;
	}
	void QueryOrder(size_t UserId,short Date,const String<20> &ct,std::ostream &os)
	{
		bool ch[26]={0};
		for (int i=0;ct[i]<='Z'&&ct[i]>='A';++i) ch[ct[i]-'A']=true;
		UTicketKey x;x.UserId=UserId;x.Date=Date;
		TrainValue t;
		sjtu::vector<UTicketKey> Vkey;
		sjtu::vector<short> Vdata;
		size_t cnt=0;
		UserTicket.AskArr(x,Cmp_UT,Vkey,Vdata);
		for (int i=0;i<Vkey.size();++i)
			if (ch[Vkey[i].Catalog-'A']&&Vdata[i]>0) ++cnt;
		if (cnt==0) {os<<"-1\n";return;}
		os<<cnt<<'\n';
		for (int i=0;i<Vkey.size();++i)
			if (ch[Vkey[i].Catalog-'A'])
				ShowTicket_Bought(Vkey[i],Vdata[i],os);
	}
void QueryTicket(const String<20> l1,const String<20> &l2,short Date,const String<20> ct,std::ostream &os)
	{
		bool ch[26]={0};
		for (int i=0;ct[i]!='\0';++i) ch[ct[i]-'A']=true;
		StationKey k1,k2;k1.Loc=l1;k2.Loc=l2;
		sjtu::vector<StationKey> Vkey1,Vkey2;
		sjtu::vector<short> Vdata1,Vdata2,Vans1,Vans2;
		LocToId.AskArr(k1,Cmp_SK,Vkey1,Vdata1);
		LocToId.AskArr(k2,Cmp_SK,Vkey2,Vdata2);
		size_t i=0,j=0;
		TrainValue Train;
		while (i<Vkey1.size()&&j<Vkey2.size())
		{
			if (Vkey1[i].TrainId<Vkey2[j].TrainId) {++i;continue;}
			if (Vkey1[i].TrainId>Vkey2[j].TrainId) {++j;continue;}
			if (Vdata1[i]>=Vdata2[j]) {++i;++j;continue;}
			if (ch[Vkey1[i].Catalog-'A'])Vans1.push_back(i),Vans2.push_back(Vdata2[j]);
			++i,++j;
		}
		os<<Vans1.size()<<"\n";
		UTicketKey k;TrainValue t;k.Date=Date;
		for (int x=0;x<Vans1.size();++x)
		{
			k.TrainId=Vkey1[Vans1[x]].TrainId;
			k.l1=Vdata1[Vans1[x]];k.l2=Vans2[x];
			ShowTicket_q(k,os);
		}
	}
    void QueryTrans(const String<20> l1,const String<20> &l2,short Date,const String<20> ct,std::ostream &os)
    {
        bool ch[26]={0};
        for (int i=0;ct[i]!='\0';++i) ch[ct[i]-'A']=true;
        StationKey k1,k2;k1.Loc=l1;k2.Loc=l2;
        sjtu::vector<StationKey> Vkey1,Vkey2;
        sjtu::vector<short> Vdata1,Vdata2;

        LocToId.AskArr(k1,Cmp_SK,Vkey1,Vdata1);
        LocToId.AskArr(k2,Cmp_SK,Vkey2,Vdata2);
        TrainValue t1,t2;
        Trans Ans,anx;long Time=12345678,T2;
        for (size_t x=0;x<Vkey1.size();++x)
            if (ch[Vkey1[x].Catalog-'A'])
            {
                AlTrain.Find(Vkey1[x].TrainId,t1);
                StationKey ky;anx.TrainId1=Vkey1[x].TrainId;ky.TrainId=Vkey1[x].TrainId;
                anx.l11=Vdata1[x];
                for (short I=Vdata1[x]+1;I<t1.LocNum;++I)
                {
                    anx.l12=I;
                    ky.Loc=t1.Loc[I];
                    sjtu::vector<StationKey> Vkeyj;
                    sjtu::vector<short> Vdataj;
                    LocToId.AskArr(ky,Cmp_SK,Vkeyj,Vdataj);
                    size_t i=0,j=0;
                    while (i<Vkeyj.size()&&j<Vkey2.size())
                    {
                        if (Vkeyj[i].TrainId<Vkey2[j].TrainId) {++i;continue;}
                        if (Vkeyj[i].TrainId>Vkey2[j].TrainId) {++j;continue;}
                        if (Vdataj[i]>=Vdata2[j]) {++i;++j;continue;}
                        if (ch[Vkey2[j].Catalog-'A'])
                        {
                            anx.l21=Vdataj[i];
                            anx.l22=Vdata2[j];
                            anx.TrainId2=Vkey2[j].TrainId;
                            AlTrain.Find(Vkey2[j].TrainId,t2);
                            T2=Delta(anx,t1,t2);
                            if (T2<Time) Time=T2,Ans=anx;
                        }
                        ++i,++j;
                    }
                }
            }
        if (Time==12345678) {os<<"-1\n";return;}
        Ans.Date=Date;
        ShowTicket_Trans(Ans,os);
    }
	void Clear()
	{
		LocToId.Clear();
		AlTrain.Clear();
		UserTicket.Clear();
		LeftTicket.Clear();
		LSiz=0;
	}
};




#endif //AKINA_TRAINMANAGER_HPP
