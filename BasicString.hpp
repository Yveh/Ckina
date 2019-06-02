//
// Created by 18146 on 2019/5/4.
//

#ifndef AKINA_BASICSTRING_HPP
#define AKINA_BASICSTRING_HPP
#include <cstdio>
#include <ostream>
#include <cstring>
template <size_t N>
class String
{
public:
	friend bool operator<(const String<N> &l,const String<N> &r)
	{
		return strcmp(l.ch,r.ch)<0;
	}
private:
	char ch[N+1];
public:
	String(){ch[0]='\0';}
	String (const String &other){strcpy(ch,other.ch);}
	String (const char* other){strcpy(ch,other);}
	String &operator=(const String &other){strcpy(ch,other.ch);return *this;}
	~String()=default;
	int cmp(const String &other) const {return strcmp(ch,other.ch);}
	bool operator==(const String &other){return strcmp(ch,other.ch)==0;}
	bool operator!=(const String &other){return strcmp(ch,other.ch)!=0;}
	bool operator>(const String &other){return strcmp(ch,other.ch)>0;}
	bool operator<(const String &other){return strcmp(ch,other.ch)<0;}
	bool operator==(const char* other){return strcmp(ch,other)==0;}
	bool operator!=(const char* other){return strcmp(ch,other)!=0;}
	bool operator>(const char* other){return strcmp(ch,other)>0;}
	bool operator<(const char* other){return strcmp(ch,other)<0;}
	char &operator[](const size_t pos) {return ch[pos];}
	const char &operator[](const size_t pos) const {return ch[pos];}
	friend std::ostream &operator<<(std::ostream &os, const String &Str){os << Str.ch<<' ';return os;}
	friend std::istream &operator>>(std::istream &is, String &Str){is>>Str.ch;return is;}
	inline short ToTime(){return ch[0]*600+ch[1]*60+ch[3]*10+ch[4]-32208;}
	inline short ToDate(){return (ch[6]-'6')*30+ch[8]*10+ch[9]-528;}
	double ToPrice()
	{
		double x=0,f=1;
		for (int i=3;ch[i]!='\0';i++)
		{
			if (ch[i]=='.') f=0.1;
			else if (f==1) x=x*10+ch[i]-'0';
			else x=x+f*(ch[i]-'0'),f=f/10;
		}
		return x;
	}
};
String<5> StrTime(const short &t)
{
	String<5> x;
	x[0]=t/600+48;
	x[1]=(t/60)%10+48;
	x[2]=':';
	x[3]=(t%60)/10+48;
	x[4]=t%10+48;
	x[5]='\0';
	return x;
}
std::string StrDate(short t)
{
    if(t<=30){
        std::string x("2019-06-");
        x=x+char(t/10+48)+char(t%10+48)+' ';
        return x;
    }
    std::string x("2019-07-");
    x=x+char(t/10+45)+char(t%10+48)+' ';
	return x;
}

#endif //AKINA_BASICSTRING_HPP
