/*
 * BonusSim.h
 */

#ifndef _BONUSSIM_H_
#define _BONUSSIM_H_

#define ERROR -1
#define INIT 0
#define MODULE 1
#define ENDMODULE 2
#define WIRE 3
#define COMPONENT 4
#define LPAREN 5 //list of component pins
#define BUSPIN 6 //part of bus as pin
#define BUSWIREPIN 7 //single wire of bus as pin
#define FULLBUS 8 //full buss as pin
#define WIREPIN 9 //single wire as pin
#define RPAREN 10 //Right parenthesis
#define COMMA 11
#define ASSIGN 12
#define NO_OF_SIMULATIONS 10

struct store
{
	std::string ComponentName;
	std::vector<struct currentvalue> CV;
	std::vector<struct currentvalue> PV;
	std::vector<struct trisvalue> TV;
};

typedef struct  {
	std::string modulename;
	std::vector<struct moduleparameters> moduleparameters;
	std::vector<struct componentstruct>componentdetails;
	std::vector<struct netstruct> net;
	std::vector<struct netstruct> unsortednet;
	std::vector<struct netlist> netList;
	std ::vector<struct currentvalue> currentValue;
	std ::vector<struct currentvalue> prevValue;
	std ::vector<struct trisvalue> trisValue;
}modulestruct;

struct moduleparameters
{
	int type; // 0 = input 1 = output
	std::string pinname;
	int busstartindex;//-1 for single wire
	int busendindex;//-1 for single wire
};

struct currentvalue{
	std::string wirename;
	//unsigned int wirevalue;
	std::vector<unsigned int> wirevalue;//index zero stores normal 0 and 1;
	//index 1 stores 1 if the value is 'Z' a floating net,and
	//index 2 stores 1 if the value is 'X', any logical value that is not known,
};// 'Z' appears for tris and buf only; No' X' for now


struct tempcurrentvalue{
	std::string wirename;
	unsigned int wirevalue;

};

struct trisvalue{
	std::string wirename;
	unsigned int wirevalueB;// stores normal 0 and 1;
	unsigned int wirevalueZ;// stores 1 if  Z;
};

struct netstruct {
	std::string wirename;
	int wireindex;
	int wiresize;
};

struct componentstruct {
	std::string componenttype;
	std::string componentname;
	std::vector<std::string> pinname;
	std::vector<int> busstartindex;
	std::vector<int> busendindex;
	std::vector<int> pass;
	unsigned  componentsize;
	int simulationID;
	short modulenotfound;
	short firstsimulation;
};

struct netlist
{
	std::string wireName;
	std::vector<struct componentlist> componentList;
};

struct componentlist
{
	std::string componentType;
	std::string componentName;
	int pinLocation;
	int componentID;
};



#endif /* _BONUSSIM_H_ */
