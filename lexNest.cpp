#include <iostream>
#include <fstream>
#include <string.h>
#include <vector>
#include <algorithm>
#include <functional>
#include <cctype>

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
#define NO_OF_SIMULATIONS 1000

struct store
{
	std::string ComponentName;
	std::vector<struct currentvalue> CV;
	std::vector<struct currentvalue> PV;
	std::vector<struct trisvalue> TV;
};
std::vector <struct store> storeData;

struct modulestruct {
	std::string modulename;
	std::vector<struct moduleparameters> moduleparameters;
	std::vector<struct componentstruct>componentdetails;
	std::vector<struct netstruct> net;
	std::vector<struct netstruct> unsortednet;
	std::vector<struct netlist> netList;
	std ::vector<struct currentvalue> currentValue;
	std ::vector<struct currentvalue> prevValue;
	std ::vector<struct trisvalue> trisValue;
};

struct moduleparameters
{
	int type; // 0 = input 1 = output
	std::string pinname;
	int busstartindex;//-1 for single wire
	int busendindex;//-1 for single wire
};
/*struct wirestruct {
std::string wirename;
int wiresize;
};*/

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
	//std::vector<unsigned int> pinvalue;
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


int tempglobalcounter= 0;

struct trisvalue temptv_vector;
unsigned no_of_transitions = 0;
std::ifstream readinput;
std::vector<struct modulestruct> moduledetails;
unsigned short me;// module in execution
unsigned short prev_me,prev_cde;// previous module and cd in execution

std::vector<unsigned short> me_vector;// module in execution store the value


void writetofile(/*const std::vector<struct modulestruct> & moduledetails,*/
/*					 const std::vector<struct componentstruct> & componentdetails,*/
/*					 const std::vector<struct netstruct> & net,*/
/*					 const std::vector<struct netlist> & netList,*/
					 std::ofstream &netlist_file);
int netadd(struct netstruct tempnet, std::vector<struct netstruct> &net);
template<typename T> int searchNet(const std::vector<T> vec, const T& key);
template<typename T> int searchString(const std::vector<T> vec, const T& key);
int CheckComponent(/*const std::vector<struct modulestruct> & moduledetails,*/struct componentstruct cs);
void CheckComponentPinValidity(std::vector<struct componentstruct> & cd,std::vector<struct netstruct> & net);
void Simulate(std::string argv,/*std::vector<struct modulestruct>&moduledetails,*/std::vector<struct componentstruct> & cd,unsigned cd_index,const std::vector<struct netstruct> & net, std::vector<struct netlist>&nl,int TransId,int flag);
std::string  CreatePinValueString(std::vector<struct componentstruct> & cd, /*std::vector<struct currentvalue> cv,*/unsigned int index);
std::string CreateInputFile(std::vector<struct componentstruct> & cd,int index,int TransId);
unsigned UpdateCurrentValue(unsigned OrgNumber, unsigned NewNumber, unsigned int endIndex, unsigned int startIndex);
unsigned  CheckComponentExecStatus(struct componentstruct cd,unsigned pinindex,unsigned wire_no,
		const std::vector<struct netstruct> & net,std::vector<struct netlist>&nl,unsigned *nlIndex,unsigned *clIndex);
unsigned  CheckModuleExecStatus(struct componentstruct cd,unsigned pinindex, unsigned wire_no, const std::vector<struct netstruct> & net,
							  std::vector<struct netlist>&nl,unsigned *nlIndex,unsigned *clIndex);
int lex(std::ifstream &input_file,std::vector<std::string>&tokenvector);
void CreatenetList(/*std::vector<struct componentstruct> componentdetails,*/
			 /*std::vector<struct netstruct> unsortednet,*//*std::vector<struct modulestruct>&moduledetails,*/
/*			 std::vector<struct netlist>&netList,*/
			 std::ofstream &netlist_file);
void parse(std::vector<std::string>&tokenvector/*,std::vector<struct modulestruct>&moduledetails*//*,std::vector<struct componentstruct>&componentdetails,std::vector<struct netstruct>&net,std::vector<struct netstruct>&unsortednet,std::vector<std::string> &modulenamevector*/);
void TokenizeAssign(std::vector<std::string> &tokenvector, unsigned *index, std::vector<struct componentstruct> &componentdetails,std::vector<struct netstruct> &net);
int TokenizeModule(std::vector<std::string> &tokenvector, unsigned *index,struct modulestruct *moduledetails);




int ComputeTrisValue(){
	unsigned i = 0,j = 0,k = 0,startk,endk,count=0,found = 0;
	unsigned wirevalueB, size;
	//check if more than one 'Z' in trisValue.wirevalueZ
	//std::cout<<"\n trisValue.size() "<<trisValue.size()<<std::endl;
	size = moduledetails[me].trisValue.size();
	if (size == 0) return 0;
	for(k = 0; k< size-1; k++)
	{
		//size = trisValue[k].wirevalueZ.size();
		startk = k;
		found = 0;
		count = 0;
		while((moduledetails[me].trisValue[k].wirename == moduledetails[me].trisValue[k+1].wirename)&&(k < (moduledetails[me].trisValue.size()-2)))
		{
			//std::cout<<"\ntrisValue.wirevalueZ.at(i) "<<trisValue.wirevalueZ.at(i)<<"i ="<<i<<std::endl;
			found = 1;
			if (moduledetails[me].trisValue[k].wirevalueZ == 0)
				count++;
			k++;
		}
		if ((moduledetails[me].trisValue[k+1].wirevalueZ == 0)&&(found))
		{
						count++;

		}
		endk = k+1;
		if (count > 1)
			return -1;// too many 'Z's//

		for(i = startk; i<= endk;i++)//trial == added
		{
			if (moduledetails[me].trisValue[i].wirevalueZ == 0)
			{
				//std::cout<<"\ntrisValue of wirevalueZ is 0 at   "<<i<<std::endl;

				wirevalueB = moduledetails[me].trisValue[i].wirevalueB;
				break;
			}
		}

		// find the pin in currentValue and copy wirevalueB in it
		//std::cout<<"\n trisValue.wirename() "<<trisValue[i].wirename<<std::endl;

		for(j = 0, found = 0; j < moduledetails[me].currentValue.size(); j++)
		{
			//std::cout<<"\n currentValue.wirename() "<<currentValue[j].wirename<<" j = "<<j<<std::endl;

			if(moduledetails[me].currentValue[j].wirename.compare(moduledetails[me].trisValue[i].wirename) == 0)
			{
				found = 1;
				break;
			}
		}
		if(found)
			moduledetails[me].currentValue[j].wirevalue[0] = wirevalueB;
		//std::cout<<"\nName = " <<moduledetails[me].currentValue[j].wirename<<" "<<"value= "<<wirevalueB<<std::endl;
	}
	return 0;
}

void CreatenetList(
			 std::ofstream &netlist_file)
{
	// Populating the NETLIST
	struct netlist tempnetList;
	struct componentlist tempcomponentList;
	int nComp = 0;
	unsigned int i = 0, j = 0, k=0, moduleno = 0;
	for(moduleno =0; moduleno < moduledetails.size();moduleno++)
	{
		for(i = 0; i < moduledetails[moduleno].unsortednet.size();i++)
		{
			//std::cout<<"In first for of CreatenetList  unsortednet.size = "<<moduledetails[moduleno].unsortednet.size()<<std::endl;

			nComp = 0;
			for ( j= 0 ; j < moduledetails[moduleno].componentdetails.size();j++)
			{
				for( k = 0; k < moduledetails[moduleno].componentdetails[j].pinname.size();k++)
				{
					if (moduledetails[moduleno].unsortednet[i].wirename.compare(moduledetails[moduleno].componentdetails[j].pinname[k]) == 0)
					{

						if(moduledetails[moduleno].unsortednet[i].wireindex <=  moduledetails[moduleno].componentdetails[j].busstartindex[k] )
						{
							if(moduledetails[moduleno].unsortednet[i].wireindex >=  moduledetails[moduleno].componentdetails[j].busendindex[k])
							{
								tempcomponentList.componentType.assign(moduledetails[moduleno].componentdetails[j].componenttype);
								tempcomponentList.componentName.assign(moduledetails[moduleno].componentdetails[j].componentname);
								tempcomponentList.pinLocation = k;
								tempcomponentList.componentID = j;
								tempnetList.componentList.push_back(tempcomponentList);
								nComp++;
								//td::cout<<"cd  = "<<j<<moduledetails[moduleno].componentdetails[j].componenttype<<std::endl;
							}
						}
					}
				}
			}
			tempnetList.wireName.assign(moduledetails[moduleno].unsortednet[i].wirename);
			moduledetails[moduleno].netList.push_back(tempnetList);
			while (nComp--)
				tempnetList.componentList.pop_back();
		}
		//std::cout<<"Once in CreateNet" <<std::endl;

	}
}

int TokenizeModule(std::vector<std::string> &tokenvector, unsigned *index,struct modulestruct *tempmodule)
{
	int wiresize = 0;
	int wireupper = 0;
	int wirelower = 0;
	struct netstruct tempnet, comparenet;
	struct moduleparameters temp_mp;
	int retValue = 0;

	unsigned i = *index;
	tempmodule->modulename.assign(tokenvector[i]);
	//std::cout<<"NEW modulename = " <<tokenvector[i]<<std::endl;

	i++;
	if(tokenvector[i] == ";")// this is a top level module , so return 1
	{
		*index = i;
		retValue = 1;
		temp_mp.type = -1;// top module
		tempmodule->moduleparameters.push_back(temp_mp);
	}
	else //get the remaining string and store in moduleparameters
	{
		i++;//skip "("
		while(tokenvector[i].compare(";")!=0)//till end of Wire list;
		{
			//std::cout<<"i =  "<<i<<" " <<tokenvector[i]<<std::endl;

			if(tokenvector[i].compare(",")==0)
			{
				i++;
				//std::cout<<"i =  "<<i<<" " <<tokenvector[i]<<std::endl;

			}
			else if(tokenvector[i].compare(")")==0)
			{
				i++;
				//std::cout<<"i =  "<<i<<" " <<tokenvector[i]<<std::endl;

			}
			else
			{
				//copy parametertype
				if(tokenvector[i] == "output")
					temp_mp.type = 1;
				else
					temp_mp.type = 0;
				i++;
				//std::cout<<"i =  "<<i<<" " <<tokenvector[i]<<std::endl;

				wiresize = 1;

				if(tokenvector[i].compare("[")==0)//wire is a bus
				{
					wireupper = atoi(tokenvector[i+1].c_str());
					wirelower = atoi(tokenvector[i+3].c_str());
					wiresize = wireupper - wirelower+1;//wire size = upperlimit-lowerlimit+1
					if(wiresize<1)
						std::cout<<"Error in bus range."<<std::endl;
					i = i+5;//skip [m:n] = 5 tokens
					//std::cout<<"i =  "<<i<<" " <<tokenvector[i]<<std::endl;

					for(int wireindex=wirelower; wireindex<(wireupper+1); wireindex++)
					{
						tempnet.wirename.assign(tokenvector[i]);
						tempnet.wireindex = wireindex;
						tempnet.wiresize = wiresize;
						//std::cout<<tempnet.wirename<<"["<<tempnet.wireindex<<"]"<<std::endl;
						netadd(tempnet, tempmodule->net);
						tempmodule->unsortednet.push_back(tempnet);
					}

					temp_mp.busstartindex = wireupper;
					temp_mp.busendindex = wirelower;
					temp_mp.pinname.assign(tokenvector[i]);
				}
				else//single wire
				{
					tempnet.wirename.assign(tokenvector[i]);
					tempnet.wireindex = -1;
					tempnet.wiresize = 1;

					netadd(tempnet, tempmodule->net);
					tempmodule->unsortednet.push_back(tempnet);
					temp_mp.busstartindex = -1;
					temp_mp.busendindex = -1;
					temp_mp.pinname.assign(tokenvector[i]);
				}
				i++;
				//std::cout<<"i =  "<<i<<" " <<tokenvector[i]<<std::endl;
				tempmodule->moduleparameters.push_back(temp_mp);
			}

		}//end of while
		*index = i;
		retValue = 0;
	}
	return retValue;
}

void TokenizeAssign(std::vector<std::string> &tokenvector, unsigned *index, std::vector<struct componentstruct> &componentdetails,std::vector<struct netstruct> &net)
{
	unsigned i = *index;
	//int wiresize = 0;
	//int wireupper = 0;
	//int wirelower = 0;
	int componentsize = 0;
	int comparelower = 0;
	int compareupper = 0;
	int wireexists = -1;
	int state = INIT;//initial state
	struct netstruct tempnet, comparenet;
	struct componentstruct tempcomp;

	tempcomp.componenttype.assign(tokenvector[i]);//tokenvector = ASSIGN
	i++;
	while(1)
	{
		//Get LHS
		//std::cout<<"tokenvector at i = "<<tokenvector[i]<<std::endl;
		componentsize++;

		tempcomp.pinname.push_back(tokenvector[i]);//any name tokens after first will be names of pins
		tempcomp.busstartindex.push_back(-1);//default value of index = -1
		tempcomp.busendindex.push_back(-1);//default value of index = -1
		comparenet.wirename.assign(tokenvector[i]);
		comparenet.wireindex = -1;
		if((tokenvector[i+1].compare("=")==0)||(tokenvector[i+1].compare(";")==0)) //SINGLE WIRE OR FULL BUS
		{
			wireexists = searchString(net, comparenet);//compare only wirename, not index/size
			if(wireexists != -1)//wire is found
			{
				tempcomp.pass.push_back(1);

				if(net[wireexists].wireindex== -1)
				{
					state = WIREPIN; //single wire with wiresize = 1
				}
				else if(net[wireexists].wireindex >=0 )
				{
					state = FULLBUS; //wire is a bus, but range not give => use full bus
				}
			}
			else
			{
				std::cout<<"wire name doesn't exist "<<comparenet.wirename<<std::endl;
				tempglobalcounter++;
				tempcomp.pass.push_back(0);
				state = ERROR;
			}
			if(state==FULLBUS)
			{
				//				std::cout<<"bus without indices"<<std::endl;
				tempcomp.busstartindex.pop_back();//token is a bus, and will have valid index
				tempcomp.busendindex.pop_back();//token is a bus, and will have valid index
				//If neither msb nor lsb is specied (both are -1), then the msb should be
				//updated to (width-1) and the lsb should be updated to 0, since the pin refers
				//to the whole bus.
				tempcomp.busstartindex.push_back(net[wireexists].wiresize-1);//TODO would not work for wirename[m:n] where n!=0
				tempcomp.busendindex.push_back(0);

				//	std::cout<<"wire details in tempcomp = "<<tempcomp.pinname<<" "<<tempcomp.busstartindex<<" "<<tempcomp.busendindex<<std::endl;
				//std::cout<<"wire details in tempcomp = "<<tempcomp.pinname[0]<<" "<<tempcomp.busstartindex[0]<<" "<<tempcomp.busendindex[0]<<std::endl;
			}
		}
		else if(tokenvector[i+1].compare("[")==0)
		{
			if((tokenvector[i+3].compare("]")==0)){//single bus element is used
				tempcomp.busstartindex.pop_back();//token is a bus, and will have valid index
				i++;//get array index; discard the opening [
				//std::cout<<"tokenvector at i = "<<tokenvector[i]<<std::endl;

				i++;
				//std::cout<<"tokenvector at i = "<<tokenvector[i]<<std::endl;

				tempcomp.busstartindex.push_back(atoi(tokenvector[i].c_str()));//save index

				//If the msb is specified but the lsb is not (the lsb is ..1), then it must be true
				//that width > msb . 0. The lsb should be updated to be the same as the
				//msb, since the pin refers to a single bit within the bus.
				tempcomp.busendindex.pop_back();//token is a bus, and will have valid index
				tempcomp.busendindex.push_back(atoi(tokenvector[i].c_str()));//save index

				comparenet.wireindex = atoi(tokenvector[i].c_str());
				i++;//get closing paren ]
				//std::cout<<"tokenvector at i = "<<tokenvector[i]<<std::endl;

				//i++;//go to next token
				//std::cout<<"tokenvector at i = "<<tokenvector[i]<<std::endl;

				wireexists = searchNet(net, comparenet);
				if(wireexists != -1){//wire is found
					state = BUSWIREPIN;
					tempcomp.pass.push_back(1);
				}
				else
				{
					state = ERROR;
					tempcomp.pass.push_back(0);
					std::cout<<"wire in a bus doesn't exist "<<comparenet.wirename<<std::endl;
				}
			}
			else if((tokenvector[i+3].compare(":")==0))//range of bus element is used [m:n]
			{
				tempcomp.busstartindex.pop_back();//token is a bus, and will have both indices
				tempcomp.busendindex.pop_back();//token is a bus, and will have both indices
				i++;
				i++;//get array starting index;
				//std::cout<<"tokenvector at i = "<<tokenvector[i]<<std::endl;

				tempcomp.busstartindex.push_back(atoi(tokenvector[i].c_str()));//save starting index
				compareupper = atoi(tokenvector[i].c_str());
				i++; //get colon :
				//std::cout<<"tokenvector at i = "<<tokenvector[i]<<std::endl;

				i++;//get array ending index;
				//std::cout<<"tokenvector at i = "<<tokenvector[i]<<std::endl;

				tempcomp.busendindex.push_back(atoi(tokenvector[i].c_str()));//save index
				comparelower = atoi(tokenvector[i].c_str());
				i++;//get closing paren ]
				//std::cout<<"tokenvector at i = "<<tokenvector[i]<<std::endl;

				//i++;//go to next token
				//std::cout<<"tokenvector at i = "<<tokenvector[i]<<std::endl;

				for(int compareidx = comparelower; compareidx<=compareupper; compareidx++)
				{
					comparenet.wireindex = compareidx;
					wireexists = searchNet(net, comparenet);
					if(wireexists != -1){//wire is found
						state = BUSPIN;
					}
					else{
						std::cout<<"bus (ex in[m:n]) doesn't exist"<<std::endl;
						state = ERROR;
					}
				}
				if (state == ERROR)
					tempcomp.pass.push_back(0);
				else
					tempcomp.pass.push_back(1);
			}
		}
		if(componentsize ==2) break;
		i++;//go to '='
		i++;//got to RHS;
	}

	tempcomp.componentsize = componentsize;
	//state = CheckComponent(tempcomp);

	//else if(cs.componenttype == "assign"){
		if(tempcomp.componentsize != 2) state = 0;// check no of pins;should be qxactly 2
		//check width of each pin; needs to be equal
		if((tempcomp.busstartindex[0] - tempcomp.busendindex[0] + 1) != (tempcomp.busstartindex[1] - tempcomp.busendindex[1] + 1)){
			state = 0;
		}
	//}

	if(state != 0)
		componentdetails.push_back(tempcomp);//TODO if state = ERROR, don't copy if state!=RPAREN means we exist without reaching end of pinlist
	else
		std::cout<<"CheckComponent failed "<<std::endl;
	for(int ii = 0; ii<tempcomp.pinname.size();ii++ )
	{
		
	}
	tempcomp.busendindex.clear();
	tempcomp.busstartindex.clear();
	tempcomp.pass.clear();
	tempcomp.pinname.clear();
	//tempcomp.pinvalue.clear();
	while(tokenvector[i].compare(";")!=0)// TO DO CHECK
	{
		i++;// since we broke the while loop
	}
	*index = i;
}

void parse( std::vector<std::string> &tokenvector,
		   std::vector<struct modulestruct> &moduledetails)
			//std::vector<std::string> &modulenamevector)

{
	unsigned int vecsize = tokenvector.size();
	int wiresize = 0;
	int wireupper = 0;
	int wirelower = 0;
	int componentsize = 0;
	int comparelower = 0;
	int compareupper = 0;
	int wireexists = -1;
	int state = INIT;//initial state
	struct netstruct tempnet, comparenet;
	unsigned int i = 0, j = 0, k = 0;
	int TokenizeModuleRetValue = 0;
	int RetValue = 0;

	struct modulestruct tempmodule;// will have to clear this

	for(i=0;i<vecsize; i++)
	{ //operations on tokenvector[i]
		if(tokenvector[i].compare("module")==0)
		{ //if state is MODULE, next state should be module name, next should be semicolon
			state = MODULE;
			i++;
			tempmodule.componentdetails.clear();
			tempmodule.moduleparameters.clear();
			tempmodule.net.clear();
			tempmodule.unsortednet.clear();

			TokenizeModuleRetValue = TokenizeModule(tokenvector,&i,&tempmodule);
		}
		else if(tokenvector[i].compare("wire")==0)
		{
			state = WIRE;
			//struct netstruct net;

			i++; //remove token name = WIRE and store only the wire names into wirenamevector
			while(tokenvector[i].compare(";")!=0)//till end of Wire list;
			{
				if(tokenvector[i].compare(",")==0)
				{
					i++;
				}
				else
				{
					//					tempwire.wirename.assign(tokenvector[i]);
					if(wiresize==0)//no [m:n] at beginning of wire token
						wiresize = 1;

					if(tokenvector[i].compare("[")==0)
					{//wire is a bus 
						//wiredetails.pop_back();// pop vector cause it now has name=[ and size=0 in it
						wireupper = atoi(tokenvector[i+1].c_str());
						wirelower = atoi(tokenvector[i+3].c_str());
						wiresize = wireupper - wirelower+1;//wire size = upperlimit-lowerlimit+1
						if(wiresize<1)
							std::cout<<"Error in bus range."<<std::endl;
						//						tempwire.wiresize = wiresize;
						i = i+5;//skip [m:n] = 5 tokens
						//						tempwire.wirename = tokenvector[i];//get name token
						//						wiredetails.push_back(tempwire);
						for(int wireindex=wirelower; wireindex<(wireupper+1); wireindex++)
						{
							tempnet.wirename.assign(tokenvector[i]);
							tempnet.wireindex = wireindex;
							tempnet.wiresize = wiresize;
							//std::cout<<tempnet.wirename<<"["<<tempnet.wireindex<<"]"<<std::endl;
							netadd(tempnet, tempmodule.net);
							tempmodule.unsortednet.push_back(tempnet);

						}
					}
					else
					{//single wire
						int tempsize = 0;
						//loop needed for wires defined as: wire [m:n] s0, s1;
						while(tempsize<wiresize){ //if wiresize is 1, loop will be run only once. 
							tempnet.wirename.assign(tokenvector[i]);
							if(wiresize ==1)
								tempnet.wireindex = -1;
							else
								tempnet.wireindex = tempsize;//count from 0 till wiresize-1
							tempnet.wiresize = wiresize;
							tempsize++;
							//std::cout<<tempnet.wirename<<"["<<tempnet.wireindex<<"]"<<std::endl;
							netadd(tempnet, tempmodule.net);
							tempmodule.unsortednet.push_back(tempnet);
						}
						//			std::cout<<tempnet.wirename<<"["<<tempnet.wireindex<<"]"<<std::endl;
					}
					i++;
				}
			}//end of while
			wiresize = 0;
			wireupper = 0;
			wirelower = 0;
		}//end of else if for WIRE
		else if(tokenvector[i].compare("endmodule")==0)
		{
			//if current state is ENDMODULE, next state should be EOF
			state = ENDMODULE;
			CheckComponentPinValidity(tempmodule.componentdetails,tempmodule.net);

			if(TokenizeModuleRetValue == 1)// top module, make it the first module to execute
				moduledetails.insert(moduledetails.begin(),tempmodule);
			else
				moduledetails.push_back(tempmodule);
		}
		else if(tokenvector[i].compare("assign")==0)
		{
			state = ASSIGN;
			TokenizeAssign(tokenvector,&i,tempmodule.componentdetails,tempmodule.net);

			//std::cout<<	"After TokenizeAssign i  = "<<tokenvector[i]<<std::endl;

		}
		else
		{
			state = COMPONENT;
			struct componentstruct tempcomp;
			int tempcs = tempcomp.pinname.size();

			tempcomp.componenttype.assign(tokenvector[i]);//tokenvector[i].c_str(),tokenvector[i].size()
			//std::cout<<	"NEW ComponentType  = "<<tokenvector[i]<<std::endl;

			i++;//skip component type to get to component name or parameter list

			if(tokenvector[i].compare("(")==0)
				tempcomp.componentname.assign("");//null if no component name is given
			else
			{
				tempcomp.componentname.assign(tokenvector[i]);
				i++;//first token will be name of component
			}
			while(tokenvector[i].compare(";")!=0)
			{
				if((tokenvector[i].compare("(")==0)
					||(tokenvector[i].compare(")")==0)
					||(tokenvector[i].compare(",")==0))
				{
					if(tokenvector[i].compare("(")==0)
						state = LPAREN;
					else if(tokenvector[i].compare(")")==0)
						state = RPAREN;
					else if(tokenvector[i].compare(",")==0)
						state = COMMA;
					i++;//skip parentheses and commas
				}
				else
				{
					if((state==LPAREN)||(state == COMMA))
					{//start storing component pins
						tempcomp.pinname.push_back(tokenvector[i]);//any name tokens after first will be names of pins
						comparenet.wirename.assign(tokenvector[i]);
						//						comparewire.wirename.assign(tokenvector[i]);
						//			std::cout<<comparenet.wirename<<std::endl;
						tempcomp.busstartindex.push_back(-1);//default value of index = -1
						tempcomp.busendindex.push_back(-1);//default value of index = -1
						comparenet.wireindex = -1;
						//						comparewire.wiresize = 0;//not used. only initialized
						componentsize++;
						i++;//get opening paren [ or next token
						if((tokenvector[i].compare(",")==0)||(tokenvector[i].compare(")")==0))
						{//element is a wire name, might be wire or full bus
							wireexists = searchString(tempmodule.net, comparenet);//compare only wirename, not index/size
							if(wireexists != -1)//wire is found 
							{
								tempcomp.pass.push_back(1);

								if(tempmodule.net[wireexists].wireindex== -1)
								{
									state = WIREPIN; //single wire with wiresize = 1
								}
								else if(tempmodule.net[wireexists].wireindex >=0 )
								{
									state = FULLBUS; //wire is a bus, but range not give => use full bus
								}
							}
							else
							{
								std::cout<<"wire name doesn't exist "<<comparenet.wirename<<std::endl;
								tempglobalcounter++;
								tempcomp.pass.push_back(0);
								state = ERROR;
							}
							if(state==FULLBUS)
							{
								//				std::cout<<"bus without indices"<<std::endl;
								tempcomp.busstartindex.pop_back();//token is a bus, and will have valid index
								tempcomp.busendindex.pop_back();//token is a bus, and will have valid index
								//If neither msb nor lsb is specied (both are -1), then the msb should be
								//updated to (width-1) and the lsb should be updated to 0, since the pin refers
								//to the whole bus.
								tempcomp.busstartindex.push_back(tempmodule.net[wireexists].wiresize-1);//TODO would not work for wirename[m:n] where n!=0
								tempcomp.busendindex.push_back(0);
							}
						}
						else if(tokenvector[i].compare("[")==0)
						{
							if((tokenvector[i+2].compare("]")==0)){//single bus element is used
								tempcomp.busstartindex.pop_back();//token is a bus, and will have valid index
								i++;//get array index; discard the opening [
								tempcomp.busstartindex.push_back(atoi(tokenvector[i].c_str()));//save index

								//If the msb is specied but the lsb is not (the lsb is ..1), then it must be true
								//that width > msb . 0. The lsb should be updated to be the same as the
								//msb, since the pin refers to a single bit within the bus.
								tempcomp.busendindex.pop_back();//token is a bus, and will have valid index
								tempcomp.busendindex.push_back(atoi(tokenvector[i].c_str()));//save index

								comparenet.wireindex = atoi(tokenvector[i].c_str());
								i++;//get closing paren ]
								i++;//go to next token
								wireexists = searchNet(tempmodule.net, comparenet);
								if(wireexists != -1){//wire is found
									state = BUSWIREPIN;
									tempcomp.pass.push_back(1);
								}
								else 
								{
									state = ERROR;
									tempcomp.pass.push_back(0);
									std::cout<<"wire in a bus doesn't exist "<<comparenet.wirename<<std::endl;
								}
							}
							else if((tokenvector[i+2].compare(":")==0))//range of bus element is used [m:n]
							{
								tempcomp.busstartindex.pop_back();//token is a bus, and will have both indices
								tempcomp.busendindex.pop_back();//token is a bus, and will have both indices
								i++;//get array starting index;
								tempcomp.busstartindex.push_back(atoi(tokenvector[i].c_str()));//save starting index
								compareupper = atoi(tokenvector[i].c_str());
								i++; //get colon :
								i++;//get array ending index;
								tempcomp.busendindex.push_back(atoi(tokenvector[i].c_str()));//save index
								comparelower = atoi(tokenvector[i].c_str());
								i++;//get closing paren ]
								i++;//go to next token
								for(int compareidx = comparelower; compareidx<=compareupper; compareidx++){
									comparenet.wireindex = compareidx;
									wireexists = searchNet(tempmodule.net, comparenet);
									if(wireexists != -1){//wire is found
										state = BUSPIN;
									}
									else{
										std::cout<<"bus (ex in[m:n]) doesn't exist"<<std::endl;
										state = ERROR;
									}
								}
								if (state == ERROR)
									tempcomp.pass.push_back(0);
								else
									tempcomp.pass.push_back(1);
							}
						}//end of else-if inside LPAREN
						else if((tokenvector[i].compare(")")==0))
						{
							//do nothing or break; TODO
						}
						else//state = ERROR because invalid token
						{
							state = ERROR;
							break;
						}
					}//end of LPAREN
				}//end of else
			}// end of while(tokenvector[i].compare(";")!=0)

			tempcomp.componentsize = componentsize;
			RetValue = CheckComponent(tempcomp);
			if(RetValue != 0)
			{
				if (RetValue == 2)
					tempcomp.modulenotfound = 1;
				tempmodule.componentdetails.push_back(tempcomp);//TODO if state = ERROR, don't copy if state!=RPAREN means we exist without reaching end of pinlist
			}
			else
				std::cout<<"CheckComponent failed "<<std::endl;

			tempcomp.busendindex.clear();
			tempcomp.busstartindex.clear();
			tempcomp.pass.clear();
			tempcomp.pinname.clear();
			tempcomp.modulenotfound = 0;

			while(tokenvector[i].compare(";")!=0)
				i++;// since we broke the while loop
			componentsize = 0;//reset size to be used for next component
		}// end of if of COMPONENT
	}

// call to check if all componemts in all modules are defined , else drop it
	unsigned short nomatch = 0;
	unsigned p = 0;
	for(i = 0; i < moduledetails.size(); i++)
	{
		for(j = 0; j < moduledetails[i].componentdetails.size(); j++)
		{
			nomatch = 0;
			//std::cout<<"moduledetails componentname = "<< moduledetails[i].componentdetails[j].componenttype <<std::endl;
			//std::cout<<"moduledetails componentdetails size = "<< moduledetails[i].componentdetails.size() <<std::endl;
			if( moduledetails[i].componentdetails[j].modulenotfound == 1)
			{
				//std::cout<<"moduledetails componentdetails type = "<< moduledetails[i].componentdetails[j].componenttype <<std::endl;

				for(k = 0; k < moduledetails.size(); k++)
				{
					//std::cout<<"moduledetails componenttype = "<< moduledetails[i].componentdetails[j].componenttype <<std::endl;
					//std::cout<<"moduledetails.modulename = "<< moduledetails[k].modulename <<std::endl;
					if(moduledetails[i].componentdetails[j].componenttype.compare(moduledetails[k].modulename) == 0)
					{
						moduledetails[i].componentdetails[j].modulenotfound = 0;
						//match found check other details
						//check pinsize
						if(moduledetails[i].componentdetails[j].pinname.size() != moduledetails[k].moduleparameters.size())
						{
							//std::cout<<"moduledetails moduleparameters.size() = "<< moduledetails[i].moduleparameters.size() <<std::endl;
							//std::cout<<"cs.pinname.size()  = "<< cs.pinname.size() <<std::endl;
							// fault ret = 0;
							nomatch = 1;
							break;
						}
						else if(moduledetails[i].componentdetails[j].pinname.size() == moduledetails[k].moduleparameters.size())
						{
							for(p = 0; p < moduledetails[i].componentdetails[j].pinname.size(); p++)
							{
								if((moduledetails[i].componentdetails[j].busstartindex[p]-moduledetails[i].componentdetails[j].busendindex[p]+1) !=
								   (moduledetails[k].moduleparameters[p].busstartindex-moduledetails[k].moduleparameters[p].busendindex+1))
								{
									//fault ret = 0;
									nomatch = 1;
									break;
								}
							}
						}
					}
				}
				if (( (k == moduledetails.size()) && (moduledetails[i].componentdetails[j].modulenotfound == 1)) || nomatch)
				{
					moduledetails[i].componentdetails.erase(moduledetails[i].componentdetails.begin()+j);
					j--;
				}
			}
		}
	}
}

//0 on success, -1 on error
int lex(std::ifstream &input_file,std::vector<std::string>&tokenvector)
{
	std::string line;
	int state = INIT;//initial state
	int line_no = 0;
	for (line_no = 1; std::getline(input_file, line); ++line_no)
	{
		//	printf("dealing with Line %d \n", line_no);
		for (size_t i = 0; i < line.size();)
		{
			// comments
			if (line[i] == '/')
			{
				++i;
				if ((i == line.size()) || (line[i] != '/'))
				{
					std::cerr << "LINE " << line_no
						<< ": a single / is not allowed" << std::endl;
					state = ERROR;
					return -1;
				}
				break; // skip the rest of the line by exiting the loop
			}

			// spaces
			if ((line[i] == ' ') || (line[i] == '\t')
				|| (line[i] == '\r') || (line[i] == '\n'))
			{
				++i; // skip this space character
				continue; // skip the rest of the iteration
			}

			// SINGLE
			if ((line[i] == '(') || (line[i] == ')')
				|| (line[i] == '[') || (line[i] == ']')
				|| (line[i] == ':') || (line[i] == ';')
				|| (line[i] == ',') || (line[i] == '='))
			{
				//		output_file << "SINGLE " << line[i] << std::endl;
				std::string tempsingle = "";
				tempsingle.append(&line[i], 1);//append one character starting at location &line[i]
				tokenvector.push_back(tempsingle);
				++i; // we consumed this character
				continue; // skip the rest of the iteration
			}

			//NUMBER
			if ((line[i]>='0') && (line[i]<='9')) //0 to 9

			{
				int number_begin = i;\
					for (++i; i<line.size();i++){
						if((line[i]>='0') && (line[i]<='9')){\
							continue;
						}
						else{ //character is not a number
							break;
						}
					}
					std::string tempnum = line.substr (number_begin, i-number_begin);
					//	output_file << "NUMBER "<< tempnum << std::endl;
					tokenvector.push_back(tempnum);
					continue;
			}

			// NAME
			if (((line[i] >= 'a') && (line[i] <= 'z'))       // a to z
				|| ((line[i] >= 'A') && (line[i] <= 'Z'))    // A to Z
				|| (line[i] == '_'))
			{
				size_t name_begin = i;
				for (++i; i < line.size(); ++i)
				{
					if (!(((line[i] >= 'a') && (line[i] <= 'z'))
						|| ((line[i] >= 'A') && (line[i] <= 'Z'))
						|| ((line[i] >= '0') && (line[i] <= '9'))
						|| (line[i] == '_') || (line[i] == '$')))
					{
						break; // [name_begin, i) is the range for the token
					}
				}
				std::string tempname =  line.substr(name_begin, i-name_begin);
				tokenvector.push_back(tempname);
			}
			else
			{
				std::cerr << "LINE " << line_no
					<< ": invalid character" << std::endl;
				state = ERROR;
				return -1;
			}
		}
	}//end of for loop

	if(state==ERROR){ //if state is ERROR, there was an error in input
		std::cout<<"Error was found. Please check input."<<std::endl;
		return -1;
	}
	return 0;
}
int CheckComponent(/*const std::vector<struct modulestruct> & moduledetails,*/struct componentstruct cs){
	int ret = 1;//all ok
	unsigned i = 0, j = 0;

	if((cs.componenttype == "and")||(cs.componenttype == "or")||(cs.componenttype == "xor")||(cs.componenttype == "xnor")){
		if(cs.componentsize < 3) ret = 0;// check no of pins;should be > 3
		for(i = 0;i< cs.componentsize;i++){ //check width of each pin; needs to be one
			if((cs.busstartindex[i] - cs.busendindex[i] + 1) != 1){
				ret = 0;
				break;
			}
		}
	}
	else if((cs.componenttype == "evl_dff")||(cs.componenttype == "tris")){
		if(cs.componentsize != 3) ret = 0;// check no of pins;should be = 3
		for(i = 0;i< cs.componentsize;i++){ //check width of each pin; needs to be one
			if((cs.busstartindex[i] - cs.busendindex[i] + 1) != 1){
				ret = 0;
				break;
			}
		}
	}
	else if((cs.componenttype == "not")||(cs.componenttype == "buf")){
		if(cs.componentsize != 2) ret = 0;// check no of pins;should be qxactly 2
		for(i = 0;i< cs.componentsize;i++) //check width of each pin; needs to be one
			if((cs.busstartindex[i] - cs.busendindex[i] + 1) != 1){
				ret = 0;
				break;
			}
	}	
	else if(cs.componenttype == "evl_clock"){
		if(cs.componentsize != 1) ret = 0;// check no of pins;should be qxactly 1
		//for(i = 0;i< cs.componentsize;i++) //check width of each pin; needs to be one
		if((cs.busstartindex[0] - cs.busendindex[0] + 1) != 1){
			ret = 0;
			//break;
		}
	}	
	else if((cs.componenttype == "evl_one")||(cs.componenttype == "evl_zero")||
		(cs.componenttype == "evl_input")||(cs.componenttype == "evl_output")){
			if(cs.componentsize < 1) ret = 0;// check no of pins;should be > 1
	}
	else if(cs.componenttype == "evl_lut")//bonus
			ret = 1;

	else // it is a module ; for module it sets modulenotfound flag too ; if modulenotfound, ret is set to 1 so that component gets added and it is later confirmed with all modules added to the system in second pass
	{
		//Initialize else if module lfsr10 (first module in bonustest2.evl)found before declaration of test , program will hang
		cs.modulenotfound = 1;
		ret = 2;
		//std::cout<<"moduledetails size = "<< moduledetails.size() <<std::endl;

		for(i = 0; i < moduledetails.size(); i++)
		{
			//std::cout<<"moduledetails modulename = "<< moduledetails[i].modulename <<std::endl;
			//std::cout<<"cs.componenttype = "<< cs.componenttype <<std::endl;

			if (cs.componenttype.compare(moduledetails[i].modulename)== 0)
			{
				cs.modulenotfound = 0;
				ret = 1;
				break;
			}
		}
		if( i == moduledetails.size() && ret ==2 )
		{
			cs.modulenotfound = 1;
			//ret = 2;
		}
		if(cs.modulenotfound == 0)
		{
			//check pinsize
			if(cs.pinname.size() != moduledetails[i].moduleparameters.size())
			{
				std::cout<<"moduledetails moduleparameters.size() = "<< moduledetails[i].moduleparameters.size() <<std::endl;
				std::cout<<"cs.pinname.size()  = "<< cs.pinname.size() <<std::endl;
				ret = 0;
			}
			// if component matched with some module, check the pin widths
			else if(cs.pinname.size() == moduledetails[i].moduleparameters.size())
			{
				for(j = 0; j < cs.pinname.size(); j++)
				{
					if((cs.busstartindex[j]-cs.busendindex[j]+1) != (moduledetails[i].moduleparameters[j].busstartindex-moduledetails[i].moduleparameters[j].busendindex+1))
					{
						ret = 0;
						break;
					}
				}
			}

		}//
	}
	return ret;
}

void writetofile( std::ofstream &netlist_file)
{
					 unsigned int n=0,m=0,mvs=0;
					 int k = 0;
					 // Creating output file of netlist
					 for(mvs = 0; mvs<moduledetails.size();mvs++){ //for every module in the code
						 netlist_file<< "module" << " "<< moduledetails[mvs].modulename << std::endl;

						 netlist_file<<"nets "<<moduledetails[mvs].unsortednet.size()<<std::endl;
						 for(n = 0; n< moduledetails[mvs].unsortednet.size();n++){
							 if(moduledetails[mvs].unsortednet[n].wireindex!=-1)
								 netlist_file<<"\t net "<< moduledetails[mvs].unsortednet[n].wirename<<"["<<moduledetails[mvs].unsortednet[n].wireindex<<"] "
								 <<moduledetails[mvs].netList[n].componentList.size()<<std::endl;
							 else
								 netlist_file<<"\t net "<< moduledetails[mvs].unsortednet[n].wirename<<" "<<moduledetails[mvs].netList[n].componentList.size()<<std::endl;

							 for(m =0;m< moduledetails[mvs].netList[n].componentList.size();m++){

								 if(moduledetails[mvs].netList[n].componentList[m].componentName != "")
									 netlist_file<<"\t\t "<<moduledetails[mvs].netList[n].componentList[m].componentType<<" "
									 <<moduledetails[mvs].netList[n].componentList[m].componentName<<" "<<moduledetails[mvs].netList[n].componentList[m].pinLocation<<std::endl;
								 else
									 netlist_file<<"\t\t "<<moduledetails[mvs].netList[n].componentList[m].componentType<<" "
									 <<moduledetails[mvs].netList[n].componentList[m].pinLocation<<std::endl;
							 }
						 }

						 netlist_file<<"components "<<moduledetails[mvs].componentdetails.size()<<std::endl;
						 for(n = 0; n< moduledetails[mvs].componentdetails.size();n++){
								//std::cout<<"In  write to file.componentdetails.size() = "<< moduledetails[mvs].componentdetails.size() <<"mvs = "<<mvs<<std::endl;

							 if(moduledetails[mvs].componentdetails[n].componentname !="")
								 netlist_file<<"\t component "<< moduledetails[mvs].componentdetails[n].componenttype<<" "
								 << moduledetails[mvs].componentdetails[n].componentname<<" "
								 <<moduledetails[mvs].componentdetails[n].componentsize<<std::endl;
							 else
								 netlist_file<<"\t component "<< moduledetails[mvs].componentdetails[n].componenttype<<" "
								 <<moduledetails[mvs].componentdetails[n].componentsize<<std::endl;

							 for(m = 0; m< moduledetails[mvs].componentdetails[n].pinname.size();m++){
								 if(moduledetails[mvs].componentdetails[n].busstartindex[m] == -1)//single wire
									 netlist_file<<"\t\t pin "<<moduledetails[mvs].componentdetails[n].busstartindex[m]-moduledetails[mvs].componentdetails[n].busendindex[m]+1<<" "
									 <<moduledetails[mvs].componentdetails[n].pinname[m]<<std::endl;
								 else if(moduledetails[mvs].componentdetails[n].busstartindex[m] != moduledetails[mvs].componentdetails[n].busendindex[m]){// many wires from a bus
									 netlist_file<<"\t\t pin "<<moduledetails[mvs].componentdetails[n].busstartindex[m]-moduledetails[mvs].componentdetails[n].busendindex[m]+1;
									 //removed space after pin size in string above
									 for(k = moduledetails[mvs].componentdetails[n].busendindex[m]; k <= moduledetails[mvs].componentdetails[n].busstartindex[m]; k++){
										 netlist_file<<" ";//so that extra space is not added after last element
										 netlist_file<<moduledetails[mvs].componentdetails[n].pinname[m]<<"["<<k<<"]";
									 }
									 //end of for loop
									 netlist_file<<std::endl;
									 //netlist_file<<"\t\t pin "<<componentdetails[n].busstartindex[m]-componentdetails[n].busendindex[m]+1<<" "
									 //	<<componentdetails[n].pinname[m]<<"["<<componentdetails[n].busstartindex[m]<<":"<<componentdetails[n].busendindex[m]<<"]"<<std::endl;
								 }
								 else // single wire in a bus
									 netlist_file<<"\t\t pin "<<moduledetails[mvs].componentdetails[n].busstartindex[m]-moduledetails[mvs].componentdetails[n].busendindex[m]+1<<" "
									 <<moduledetails[mvs].componentdetails[n].pinname[m]<<"["<<moduledetails[mvs].componentdetails[n].busstartindex[m]<<"]"<<std::endl;
							 }

						 }
					 }
				//	 netlist_file.close();
}

int binarysearchNet(const std::vector<struct netstruct> & vec, int start, int end, const struct netstruct & key)
//taken from https://en.wikibooks.org/wiki/Algorithm_Implementation/Search/Binary_search
{
	// Termination condition: start index greater than end index
	if(start > end){
		return -1;
	}
	// Find the middle element of the vector and use that for splitting
	// the array into two pieces.
	const int middle = start + ((end - start) / 2);
	int wireloc = -1;
	if(vec[middle].wirename.compare(key.wirename) == 0)
	{
		if(key.wireindex == vec[middle].wireindex){
			// 		std::cout<<key.wireindex<<" =key wireindex  "<<key.wirename<<" =key wirename"<<std::endl;
			//  		std::cout<<"got lucky!"<<std::endl;
			return middle;
		}
		else if(key.wireindex > vec[middle].wireindex){
			wireloc = middle;
			//  		std::cout<<"greater than middle"<<std::endl;
			while((wireloc<=end)&&(vec[wireloc].wirename.compare(key.wirename) == 0)&&(vec[wireloc].wireindex < key.wireindex)){
				wireloc++;
			}
			// 		std::cout<<"element at that location = "<<vec[wireloc].wireindex<<vec[wireloc].wirename<<std::endl;
			if(wireloc>end)//wire not found, but end of net reached
				return -1;
			if((key.wireindex == vec[wireloc].wireindex)&&(vec[wireloc].wirename.compare(key.wirename) == 0)){//wire found
				return wireloc;
			}
			else//wireindex not found in bus
			{
				return -1;
			}
			//	return wireloc-1;
		}
		else if(key.wireindex < vec[middle].wireindex){
			wireloc = middle;
			// 		std::cout<<"less than middle"<<std::endl;
			while((wireloc>=start)&&(vec[wireloc].wirename.compare(key.wirename) == 0)&&(vec[wireloc].wireindex > key.wireindex)){
				// 			std::cout<<wireloc<<std::endl;
				wireloc--;
			}
			//wireloc = -1 for case when s3[0] not found because s3 = [12:2]
			if(wireloc == -1) //wire not found, but beginning of net reached
				return -1;
			//	std::cout<<"element at that location = "<<vec[wireloc].wireindex<<vec[wireloc].wirename<<std::endl;
			else if((key.wireindex == vec[wireloc].wireindex)&&(vec[wireloc].wirename.compare(key.wirename) == 0)){//wire found
				return wireloc;
			}
			else //wireindex not found in bus
			{
				return -1;
			}
			//	return wireloc+1;
		}
		else return -1;
	}
	else if(vec[middle].wirename.compare(key.wirename) > 0)
	{
		return binarysearchNet(vec, start, middle - 1, key);
	}
	// else if (vec[middle]wirename.compare(key.wirename) < 0)
	return binarysearchNet(vec, middle + 1, end, key);
}

//! \brief A helper function for the binary search
template<typename T>
int searchString(const std::vector<T> vec, const T& key)
{
	return binarysearchString(vec, 0, vec.size()-1, key);
}

template<typename T>
int searchNet(const std::vector<T> vec, const T& key)
{
	return binarysearchNet(vec, 0, vec.size()-1, key);
}

int binarysearchString(const std::vector<struct netstruct> & vec, int start, int end, const struct netstruct & key)
{
	// Termination condition: start index greater than end index
	if(start > end){
		return -1;
	}
	// Find the middle element of the vector and use that for splitting
	// the array into two pieces.
	const int middle = start + ((end - start) / 2);

	if(vec[middle].wirename.compare(key.wirename) == 0)

		//if(vec[middle].wirename==key.wirename)
	{
		//		std::cout<<" Middle element of net array in if "<<middle<<" "<<vec[middle].wirename<<std::endl;

		return middle;
	}
	else if(vec[middle].wirename.compare(key.wirename) > 0)
		//else if(vec[middle].wirename > key.wirename)
	{
		return binarysearchString(vec, start, middle - 1, key);
	}
	return binarysearchString(vec, middle + 1, end, key);
}

int trisvalueadd(struct trisvalue tempnet, std::vector<struct trisvalue> &net){
	unsigned int idx = 0;
	std::vector<struct trisvalue>::iterator itr = net.begin();
	if(net.size()==0){ //no elements yet
		net.push_back(tempnet);
		return 0;
	}
	if((tempnet.wirename.compare(net[idx].wirename)) < 0){//Adding to beginning of list
		net.insert(itr, tempnet);
		return 0;
	}

	while((idx < net.size())&&(tempnet.wirename.compare(net[idx].wirename)) >= 0)
	{
		idx++;
	}

	idx--;
	if((tempnet.wirename.compare(net[idx].wirename)) >= 0){//Adding to end of list
		//Either the value of the first character that does not match
		//is higher in the compared string, or all compared characters match
		//but the compared string is longer.
		while((idx< net.size())&&((tempnet.wirename.compare(net[idx].wirename)) >= 0)){
			//		std::cout<<"smaller element; keep going"<<std::endl;
			idx++;
		}
		net.insert(itr+idx,tempnet);
	}
	return 0;
}

int netadd(struct netstruct tempnet, std::vector<struct netstruct> &net){
	unsigned int idx = 0;
	std::vector<struct netstruct>::iterator itr = net.begin();
	if(net.size()==0){ //no elements yet
		net.push_back(tempnet);
		return 0;
	}
	if((tempnet.wirename.compare(net[idx].wirename)) < 0){//Adding to beginning of list
		net.insert(itr, tempnet);
		return 0;
	}

	while((idx < net.size())&&(tempnet.wirename.compare(net[idx].wirename)) >= 0)
	{
		idx++;
	}

	idx--;
	if((tempnet.wirename.compare(net[idx].wirename)) == 0){//Adding elements of same bus
		//	std::cout<<" new element of bus "<<tempnet.wirename<<"["<<tempnet.wireindex<<"]"<<std::endl;

		if((tempnet.wireindex == net[idx].wireindex)){//same name and same index; duplicate input
			std::cout<<"Duplicate Wire";
			return -1;
		}
		while((idx < net.size())&&(tempnet.wireindex > net[idx].wireindex) &&
			((tempnet.wirename.compare(net[idx].wirename)) == 0)){//insert after other elements of bus
				//		std::cout<<"increment idx"<<std::endl;
				idx++;
		}

		net.insert(itr+idx, tempnet);
		return 0;
	}
	if((tempnet.wirename.compare(net[idx].wirename)) > 0){//Adding to end of list
		//Either the value of the first character that does not match
		//is higher in the compared string, or all compared characters match
		//but the compared string is longer.
		//		std::cout<<idx<<" new element "<<tempnet.wirename<<"["<<tempnet.wireindex<<"]"<<std::endl;
		//		std::cout<<idx<<" element from list "<<net[idx].wirename<<"["<<net[idx].wireindex<<"]"<<std::endl;
		while((idx< net.size())&&((tempnet.wirename.compare(net[idx].wirename)) > 0)){
			//		std::cout<<"smaller element; keep going"<<std::endl;
			idx++;
		}
		net.insert(itr+idx,tempnet);
	}
	return 0;
}
//int _tmain(int argc, _TCHAR* argv[])
int main(int argc, char *argv[])
{
	std::vector<std::string> tokenvector;
	//std::vector<struct modulestruct> moduledetails;

	moduledetails.clear();
	unsigned i = 0, j =0, k = 0, moduleno = 0;

	if (argc < 2)
	{
		std::cerr << "You should provide a file name." << std::endl;
		return -1;
	}
	//strcpy(argv[1],"cpu8.evl");//cpu8.evl");//cpu8.evl");//tris_lut_assign.evl");//counter.evl");//adder_test.evl");//s15850.evl");//tris_lut.evl");//"lfsr10.evl");//adder_test.evl");//"bonustest4.evl");
	std::ifstream input_file(argv[1]);
	if (!input_file)
	{
		std::cerr << "Can't read " << argv[1] << "." << std::endl;
		return -1;
	}

	// open stream for writing netlist
	std::string output_file_name = std::string(argv[1])+".netlist";
	std::ofstream netlist_file(output_file_name.c_str());
	if (!netlist_file)
	{
		std::cerr << "Can't write " << argv[1] << ".netlist ." << std::endl;
		return -1;
	}


	lex(input_file,tokenvector);

	std::cout<<"First part."<<std::endl;

	parse(tokenvector, moduledetails /*,componentdetails,net, unsortednet,modulenamevector*/);

	int tt1 = tokenvector.size();

	//int temp1 = modulenamevector.size();
	int temp2 = moduledetails.size();
	int temp3 = moduledetails[0].net.size();
	//int temp4 = componentdetails.size();

	tokenvector.clear();
	//net.clear();

	std::cout<<"Second part."<<std::endl;

	//for(moduleno = 0; moduleno < moduledetails.size(); moduleno++)
	CreatenetList(/*componentdetails,unsortednet,*//*netList,*/netlist_file);
	writetofile(/*moduledetails,*/ /*componentdetails, moduledetails[moduleno].unsortednet,netList,*/netlist_file);

	netlist_file.close();

	//std::cout<<"tempglobalcounter ="<<tempglobalcounter<<std::endl;
	std::cout<<"Third part."<<std::endl;

	//Logic Simulation code for Project 4
	std::string input_file_name;
	std::string apb = "";
	std::string firstargv = argv[1];
	struct currentvalue tempcv_vector;
	for(moduleno = 0; moduleno < moduledetails.size(); moduleno++)
	{
		for ( j= 0 ; j < moduledetails[moduleno].componentdetails.size();j++)
		{
			//for evl input bonus1
			if( moduledetails[moduleno].componentdetails[j].componenttype == "evl_input")
			{
				input_file_name = firstargv+"."+  moduledetails[moduleno].componentdetails[j].componentname+".evl_input";
				//std::ifstream readinput;
				readinput.open(input_file_name.c_str());
			}
			//	create the input file
			/*if(moduledetails[moduleno].componentdetails[j].componenttype == "evl_input")
			{
				input_file_name = firstargv+"."+ moduledetails[moduleno].componentdetails[j].componentname+".evl_input";
				std::ofstream evl_input_file(input_file_name.c_str());

				evl_input_file<<moduledetails[moduleno].componentdetails[j].pinname.size();

				for(k = 0; k< moduledetails[moduleno].componentdetails[j].pinname.size();k++){
					int pinsize = moduledetails[moduleno].componentdetails[j].busstartindex[k] - moduledetails[moduleno].componentdetails[j].busendindex[k] +1;
					//std::cout<<"Pin Width = "<<pinsize <<std::endl;
					evl_input_file<<" "<<pinsize;
				}
				evl_input_file<<std::endl;

				for ( i= 0 ; i < NO_OF_SIMULATIONS;i++)
				{
					apb = CreateInputFile(moduledetails[moduleno].componentdetails ,j ,i+1);
					evl_input_file<<apb<<std::endl;
				}
			}*/
			moduledetails[moduleno].componentdetails[j].firstsimulation = 1;//.push_back(1);
		}
	}

	for ( j= 0 ; j < NO_OF_SIMULATIONS;j++)
	{
		for(moduleno = 0; moduleno < moduledetails.size(); moduleno++)
		{
			for (k = 0; k < moduledetails[moduleno].componentdetails.size(); k++)
			{
				moduledetails[moduleno].componentdetails[k].simulationID = 0;
				if(j > 0)
					moduledetails[moduleno].componentdetails[k].firstsimulation = 0;
			}
		}
		me = 0;// module in execution is top module


		for(k = 0;k < moduledetails[me].componentdetails.size();k++)
		{
			prev_cde = k;
			//std::cout<<"componemt being executed = "<< moduledetails[0].componentdetails[k].componenttype <<std::endl;
			Simulate(argv[1],moduledetails[me].componentdetails,k,moduledetails[me].unsortednet,moduledetails[me].netList,j+1,0);
		}
		for(k = 0;k < moduledetails[0].componentdetails.size();k++)
		{
			prev_cde = k;
			Simulate(argv[1],moduledetails[me].componentdetails,k,moduledetails[me].unsortednet,moduledetails[me].netList,j+1,1);// generate evl_output
		}
		//for (int m = 0; m < currentValue.size(); m++)
		//	std::cout<<"curentValue in Sim Loop = "<<currentValue[m].wirename <<" "<<currentValue[m].wirevalue[0]<<std::endl;

	}
	moduledetails.clear();
	//netList.clear();
	readinput.close();//bonus1

	return 0;

}
// simsim
void Simulate(std::string argv,/*std::vector<struct modulestruct>&moduledetails,*/std::vector<struct componentstruct> & cd,unsigned cd_index,
			  const std::vector<struct netstruct> & net,std::vector<struct netlist>&nl, int TransId,int evloutput)
{
	unsigned int i=0,j=0,k=0,m=0,n=0;//general purpose variables for loop
	std::string output_file_name;
	std::string input_file_name;
	std::string input_string;

	unsigned int temppinv ;// temporary variables required for push back
	std::string apb = "";
	//std ::vector<struct currentvalue> currentValue;
	struct tempcurrentvalue tempcv;
	struct currentvalue tempcv_vector;// copy tempcv to tempcv_vector so that it can be copied to currectValue vector easily.
	//since currentValue is now a vector whose only first value of wirevalue is used, vector of wirevalue used for evl_lut
	int found = 0;
	int lsb =0, msb = 0;
	unsigned result = 0, en = 0, in = 0, out = 0;
	int retValue;

	tempcv.wirename.assign("");  
	tempcv.wirevalue = 0xffffffff;
	unsigned nlIndex = 0, clIndex =0,input_pin_cd;// variables used to find the execution state of the input variables
	//for evl input bonus1
	std::string filenum;
	std::vector<int> pinwidth_read;
	std::vector<unsigned int> pindata_read;
	static unsigned int no_of_pins;

	i = cd_index;

	{
		if(cd[i].componenttype == "assign")
		{
			if (cd[i].simulationID == TransId) return;
			//if(moduledetails[prev_me].componentdetails[prev_cde].componenttype.compare("module") != 0)
			cd[i].simulationID = TransId;

			//std::cout<<"In Component Type ="<<cd[i].componenttype<<std::endl;
			//for(k = 0; k< moduledetails[me].currentValue.size(); k++) //
			//std::cout<<"CONTENTS OF currentValue before  assign = "<<moduledetails[me].currentValue[k].wirename<<" "<<moduledetails[me].currentValue[k].wirevalue[0]<<std::endl;

			short pn = cd[i].busendindex[1];
			do
			{
				// find the input pin in the netList structure and correspond ComponentID to check if it is already computed
				if (CheckModuleExecStatus(cd[i],1,pn,net,nl,&nlIndex,&clIndex) == 1)
				{
					// kth netlist and jth component in the list ; this component generated output that is  an input to this component

					input_pin_cd = nl[nlIndex].componentList[clIndex].componentID;
					int tt22 = cd[input_pin_cd].simulationID;// temp to remove
					if(cd[input_pin_cd].simulationID < TransId)
					{
						Simulate(argv,cd,input_pin_cd,net,nl,TransId,0);
						//std::cout<<"back to  Component Type ="<<cd[i].componenttype<<std::endl;
					}
				}
				pn++;
				//}while(pn >= 0);
			}while(pn <= cd[i].busstartindex[1]);

			found = 0;lsb = 0; msb = 0;
			for(k = 0; k< moduledetails[me].currentValue.size(); k++)//find input pin value in currentValue vector
			{
				if(moduledetails[me].currentValue[k].wirename.compare(cd[i].pinname[1]) == 0)
				{
					found = 1;
					break;
				}
			}

			if(found)// get the pin in the bus
			{
				if(cd[i].busstartindex[1] != -1)// not a single wire
				{
					msb = cd[i].busstartindex[1];
					lsb = cd[i].busendindex[1];
					result = (moduledetails[me].currentValue[k].wirevalue[0] >> lsb) & ~(~0 << (msb-lsb+1));
				}
				else
					result = moduledetails[me].currentValue[k].wirevalue[0];
			}
			else
				result = 0;

			// move temppinv by lsb of the output pin. It has to fit there.
			lsb = cd[i].busendindex[0];
			if(lsb != -1) // if not a single wire
				result = result<<lsb; // move the bit to the location pointed by lsb



			found = 0;// search for the out pin
			for(k = 0; k< moduledetails[me].currentValue.size(); k++)
			{
				//std::cout<<"wirenames  OF currentValue  ="<<currentValue[k].wirename<<std::endl;
				//std::cout<<"pinnames  OF cd  ="<<cd[i].pinname[0]<<std::endl;

				if(moduledetails[me].currentValue[k].wirename.compare(cd[i].pinname[0]) == 0)
				{
					found = 1;
					break;
				}
			}
			//std::cout<<"pinnames  OF cd  ="<<cd[i].pinname[0]<<" " <<"currentValue  ="<<moduledetails[me].currentValue[k].wirevalue[0]<<std::endl;

			tempcv.wirename.assign(cd[i].pinname[0]);  // initialise tempcv
			//const char *gg = tempcv.wirename.c_str();//temp
			if(found)
			{
				if(cd[i].busstartindex[0] == -1)
					tempcv.wirevalue = result;
				else
					tempcv.wirevalue = UpdateCurrentValue(moduledetails[me].currentValue[k].wirevalue[0], result,cd[i].busendindex[0], cd[i].busstartindex[0]);
			}
			else
				tempcv.wirevalue = result;

			if(found)
				moduledetails[me].currentValue.erase(moduledetails[me].currentValue.begin()+k);	// earse the exsting value

			tempcv_vector.wirename = tempcv.wirename;
			tempcv_vector.wirevalue.insert(tempcv_vector.wirevalue.begin(),tempcv.wirevalue);//

			moduledetails[me].currentValue.insert(moduledetails[me].currentValue.begin()+k,tempcv_vector);
			tempcv_vector.wirevalue.pop_back();
		}
		else if((cd[i].componenttype == "not") && (evloutput == 0))//Initial Release 
		{
			if (cd[i].simulationID == TransId) return;
				cd[i].simulationID = TransId;

			// find the input pin in the netList structure and correspond ComponentID to check if it is already computed
			if(CheckComponentExecStatus(cd[i],1,cd[i].busstartindex[1],net,nl,&nlIndex,&clIndex)==1)
			{

			// kth netlist and jth component in the list ; this component generated output that is  an input to this component

			input_pin_cd = nl[nlIndex].componentList[clIndex].componentID;
			//int tt22 = cd[input_pin_cd].simulationID;// temp to remove
			if(cd[input_pin_cd].simulationID < TransId) 
			{
				Simulate(argv,cd,input_pin_cd,net,nl,TransId,0);
				//std::cout<<"back to  Component Type ="<<cd[i].componenttype<<std::endl;
			}
			}
			found = 0;lsb = 0; msb = 0;
			for(k = 0; k< moduledetails[me].currentValue.size(); k++)//find input pin value in currentValue vector
			{
				//	std::cout<<"CONTENTS OF currentValue  ="<<currentValue[k].wirename<<std::endl;
				if(moduledetails[me].currentValue[k].wirename.compare(cd[i].pinname[1]) == 0)
				{
					found = 1;
					break;
				}
			}
			if(found)// get the pin in the input pin of the bus
			{
				if(cd[i].busstartindex[1] != -1)// not a single wire
				{
					msb = cd[i].busstartindex[1];
					lsb = cd[i].busendindex[1];
					result = (moduledetails[me].currentValue[k].wirevalue[0] >> lsb) & ~(~0 << (msb-lsb+1));
				}
				else 
					result = moduledetails[me].currentValue[k].wirevalue[0];
			}
			else
				result = 0;

			if(result == 0)//cd[i].pinvalue[1] ==0)
				temppinv = 1;
			else 
				temppinv = 0;
			// move temppinv by lsb of the output pin. It has to fit there.
			lsb = cd[i].busendindex[0];
			if(lsb != -1) // if not a single wire
				temppinv = temppinv<<lsb; // move the bit to the location pointed by lsb or msb; both are same for NOT gate


			found = 0;// search for the out pin of the NOT gate
			for(k = 0; k< moduledetails[me].currentValue.size(); k++)
			{
				//std::cout<<"CONTENTS OF currentValue  ="<<currentValue[k].wirename<<std::endl;
				if(moduledetails[me].currentValue[k].wirename.compare(cd[i].pinname[0]) == 0)
				{
					found = 1;
					break;
				}
			}
			tempcv.wirename.assign(cd[i].pinname[0]);  // initialise tempcv

			if(found)
				tempcv.wirevalue = UpdateCurrentValue(moduledetails[me].currentValue[k].wirevalue[0], temppinv,cd[i].busendindex[0], cd[i].busstartindex[0]);
			else 
				tempcv.wirevalue = temppinv;

			if(found)
				moduledetails[me].currentValue.erase(moduledetails[me].currentValue.begin()+k);	// earse the exsting value

			tempcv_vector.wirename = tempcv.wirename;
			tempcv_vector.wirevalue.insert(tempcv_vector.wirevalue.begin(),tempcv.wirevalue);//

			moduledetails[me].currentValue.insert(moduledetails[me].currentValue.begin()+k,tempcv_vector);
			tempcv_vector.wirevalue.pop_back();

			//cd[i].pinvalue.insert(cd[i].pinvalue.begin(), tempcv.wirevalue);// update out pin of the component

			//for(k = 0; k< currentValue.size(); k++) //
			//std::cout<<"CONTENTS OF currentValue in not = "<<currentValue[k].wirename<<" "<<currentValue[k].wirevalue[0]<<std::endl;
		}
		else if((cd[i].componenttype == "buf") && (evloutput == 0))
		{
			if(cd[i].simulationID == TransId) return;
			//if(moduledetails[prev_me].componentdetails[prev_cde].componenttype.compare("module") != 0)
				cd[i].simulationID = TransId;

			//std::cout<<"In Component Type ="<<cd[i].componenttype<<std::endl;
			// find the input pin in the netList structure and correspond ComponentID to check if it is already computed
			if(CheckComponentExecStatus(cd[i],1,cd[i].busstartindex[1],net,nl,&nlIndex,&clIndex)==1)
			{

			input_pin_cd = nl[nlIndex].componentList[clIndex].componentID;
			//int tt22 = cd[input_pin_cd].simulationID;// temp to remove
			if(cd[input_pin_cd].simulationID < TransId) 
			{
				Simulate(argv,cd,input_pin_cd,net,nl,TransId,0);
				//std::cout<<"back to  Component Type ="<<cd[i].componenttype<<std::endl;
			}
			}
			// get the value of the pin
			found = 0;lsb = 0; msb = 0;
			for(k = 0; k< moduledetails[me].currentValue.size(); k++)//find input pin value in currentValue vector
			{
				//	std::cout<<"CONTENTS OF currentValue  ="<<currentValue[k].wirename<<std::endl;
				if(moduledetails[me].currentValue[k].wirename.compare(cd[i].pinname[1]) == 0)
				{
					found = 1;
					break;
				}
			}
			if(found)// get the pin in the bus
			{
				if(cd[i].busstartindex[1] != -1)// not a single wire
				{
					msb = cd[i].busstartindex[1];
					lsb = cd[i].busendindex[1];
					result = (moduledetails[me].currentValue[k].wirevalue[0] >> lsb) & ~(~0 << (msb-lsb+1));
				}
				else 
					result = moduledetails[me].currentValue[k].wirevalue[0];
			}
			else
				result = 0;

			if(result == 1)//cd[i].pinvalue[1] ==1)//If the input in is 1, then the output out is 1; otherwise out is 0.
				temppinv = 1;
			else 
				temppinv = 0;


			lsb = cd[i].busendindex[0];
			if(lsb != -1) // if not a single wire
				temppinv = temppinv<<lsb; // move the bit to the location pointed by lsb or msb; both are same for buf gate


			found = 0;// search for the out pin of the BUF component
			for(k = 0; k< moduledetails[me].currentValue.size(); k++)
			{
				if(moduledetails[me].currentValue[k].wirename.compare(cd[i].pinname[0]) == 0)
				{
					found = 1;
					break;
				}
			}
			tempcv.wirename.assign(cd[i].pinname[0]);  // initialise tempcv
			if(found)
				//tempcv.wirevalue = currentValue[k].wirevalue[0] | temppinv;
				tempcv.wirevalue = UpdateCurrentValue(moduledetails[me].currentValue[k].wirevalue[0], temppinv,cd[i].busendindex[0], cd[i].busstartindex[0]);

			else 
				tempcv.wirevalue = temppinv;

			if(found)
				moduledetails[me].currentValue.erase(moduledetails[me].currentValue.begin()+k);	// earse the exsting value

			tempcv_vector.wirename = tempcv.wirename;
			tempcv_vector.wirevalue.insert(tempcv_vector.wirevalue.begin(),tempcv.wirevalue);//

			moduledetails[me].currentValue.insert(moduledetails[me].currentValue.begin()+k,tempcv_vector);
			tempcv_vector.wirevalue.pop_back();// was missing
			// this may be redundant now.
			//cd[i].pinvalue.insert(cd[i].pinvalue.begin(), tempcv.wirevalue);// update out pin of the component
			//	for(k = 0; k< currentValue.size(); k++) 
			//	std::cout<<"CONTENTS OF currentValue in BUF ="<<currentValue[k].wirename<<std::endl;
		}
		
		else if((cd[i].componenttype == "and") && (evloutput == 0))
		{
			if(cd[i].simulationID == TransId) return;
			//if(moduledetails[prev_me].componentdetails[prev_cde].componenttype.compare("module") != 0)
				cd[i].simulationID = TransId;

			temppinv = 1;

			//std::cout<<"In Component Type ="<<cd[i].componenttype<<" "<<"cd index " << i <<" SimID "<<cd[i].simulationID <<std::endl;
			//std::cout<<"In Component Type ="<<cd[i].componenttype<<std::endl;

			//std::cout<<"In Component Type ="<<cd[i].componenttype<<std::endl;
			// get the value of the pin
			for(j = 1; j < cd[i].componentsize; j++)
			{

				// find the input pin in the netList structure and correspond ComponentID to check if it is already computed
				if(CheckComponentExecStatus(cd[i],j,cd[i].busstartindex[j],net,nl,&nlIndex,&clIndex) ==1)
				{

					input_pin_cd = nl[nlIndex].componentList[clIndex].componentID;
					//int tt22 = cd[input_pin_cd].simulationID;// temp to remove
					if(cd[input_pin_cd].simulationID < TransId)
					{
						Simulate(argv,cd,input_pin_cd,net,nl,TransId,0);
						//std::cout<<"back to  Component Type ="<<cd[i].componenttype<<std::endl;
					}
				}
				found = 0;lsb = 0; msb = 0;
				for(k = 0; k< moduledetails[me].currentValue.size(); k++)//find input pin value in currentValue vector
				{
					//	std::cout<<"CONTENTS OF currentValue  ="<<currentValue[k].wirename<<std::endl;
					if(moduledetails[me].currentValue[k].wirename.compare(cd[i].pinname[j]) == 0)
					{
						found = 1;
						break;
					}
				}
				if(found)// get the pin in the bus
				{
					if(cd[i].busstartindex[j] != -1)// not a single wire
					{
						msb = cd[i].busstartindex[j];
						lsb = cd[i].busendindex[j];
						result = (moduledetails[me].currentValue[k].wirevalue[0] >> lsb) & ~(~0 << (msb-lsb+1));
					}
					else 
						result = moduledetails[me].currentValue[k].wirevalue[0];
				}
				else
					result = 0;

				//	for(k = 1; k < cd[i].componentsize; k++)// ANDing function
				temppinv = temppinv & result;//cd[i].pinvalue[k];
			}

			lsb = cd[i].busendindex[0];
			if(lsb != -1)// not a single wire
				temppinv = temppinv<<lsb; // move the bit to the location pointed by lsb  or msb of output pin; both are same for AND gate


			found = 0;// search for the out pin of the AND gate
			for(k = 0; k< moduledetails[me].currentValue.size(); k++)
			{
				//std::cout<<"CONTENTS OF currentValue  ="<<currentValue[k].wirename<<std::endl;
				if(moduledetails[me].currentValue[k].wirename.compare(cd[i].pinname[0]) == 0)
				{
					found = 1;
					break;
				}
			}
			tempcv.wirename.assign(cd[i].pinname[0]);  // initialise tempcv

			if(found)
				tempcv.wirevalue = UpdateCurrentValue(moduledetails[me].currentValue[k].wirevalue[0], temppinv,cd[i].busendindex[0], cd[i].busstartindex[0]);
			else 
				tempcv.wirevalue = temppinv;

			if(found)
				moduledetails[me].currentValue.erase(moduledetails[me].currentValue.begin()+k);	// earse the exsting value

			tempcv_vector.wirename = tempcv.wirename;
			tempcv_vector.wirevalue.insert(tempcv_vector.wirevalue.begin(),tempcv.wirevalue);//

			moduledetails[me].currentValue.insert(moduledetails[me].currentValue.begin()+k,tempcv_vector);
			tempcv_vector.wirevalue.pop_back();

			//cd[i].pinvalue.insert(cd[i].pinvalue.begin(), tempcv.wirevalue);// update out pin of the component

			//for(k = 0; k< moduledetails[me].currentValue.size(); k++)
				//std::cout<<"CONTENTS OF currentValue in AND ="<<moduledetails[me].currentValue[k].wirename<< " " <<moduledetails[me].currentValue[k].wirevalue[0] <<std::endl;
		}
		else if((cd[i].componenttype == "or") && (evloutput == 0))
		{
			if(cd[i].simulationID == TransId) return;
			//if(moduledetails[prev_me].componentdetails[prev_cde].componenttype.compare("module") != 0)
				cd[i].simulationID = TransId;

			temppinv = 0;
			//std::cout<<"In Component Type ="<<cd[i].componenttype<<" "<<"cd index " << i <<" SimID "<<cd[i].simulationID <<std::endl;
			// get the value of the pin
			for(j = 1; j < cd[i].componentsize; j++)
			{
				//std::cout<<"\nOR input Component ="<<cd[i].pinname[j]<<std::endl;
				// find the input pin in the netList structure and correspond ComponentID to check if it is already computed
				if(CheckComponentExecStatus(cd[i],j,cd[i].busstartindex[j],net,nl,&nlIndex,&clIndex) == 1)
				{

				input_pin_cd = nl[nlIndex].componentList[clIndex].componentID;

				if(cd[input_pin_cd].simulationID < TransId) 
				{
					Simulate(argv,cd,input_pin_cd,net,nl,TransId,0);
					//std::cout<<"back to  Component Type ="<<cd[i].componenttype<<std::endl;
				}
				}
				found = 0;lsb = 0; msb = 0;
				for(k = 0; k< moduledetails[me].currentValue.size(); k++)//find input pin value in currentValue vector
				{
					//	std::cout<<"CONTENTS OF currentValue  ="<<currentValue[k].wirename<<std::endl;
					if(moduledetails[me].currentValue[k].wirename.compare(cd[i].pinname[j]) == 0)
					{
						found = 1;
						break;
					}
				}
				if(found)// get the pin in the bus
				{
					if(cd[i].busstartindex[j] != -1)// not a single wire
					{
						msb = cd[i].busstartindex[j];
						lsb = cd[i].busendindex[j];
						result = (moduledetails[me].currentValue[k].wirevalue[0] >> lsb) & ~(~0 << (msb-lsb+1));
					}
					else 
						result = moduledetails[me].currentValue[k].wirevalue[0];
				}
				else
					result = 0;

				//std::cout<<"value ="<<result<<std::endl;

				temppinv = temppinv | result;//ORing function
			}

			lsb = cd[i].busendindex[0];
			if(lsb != -1)// not a single wire
				temppinv = temppinv<<lsb; // move the bit to the location pointed by lsb or msb of output pin

			//std::cout<<"\nOR output Component ="<<cd[i].pinname[0]<<std::endl;

			found = 0;// search for the out pin of the OR gate
			for(k = 0; k< moduledetails[me].currentValue.size(); k++)
			{
				//std::cout<<"CONTENTS OF currentValue  ="<<currentValue[k].wirename<<std::endl;
				if(moduledetails[me].currentValue[k].wirename.compare(cd[i].pinname[0]) == 0)
				{
					found = 1;
					break;
				}
			}
			tempcv.wirename.assign(cd[i].pinname[0]);  // initialise tempcv
			if(found)
				//tempcv.wirevalue = currentValue[k].wirevalue[0] | temppinv;
				tempcv.wirevalue = UpdateCurrentValue(moduledetails[me].currentValue[k].wirevalue[0], temppinv,cd[i].busendindex[0], cd[i].busstartindex[0]);
			else 
				tempcv.wirevalue = temppinv;

			if(found)
				moduledetails[me].currentValue.erase(moduledetails[me].currentValue.begin()+k);	// earse the exsting value

			tempcv_vector.wirename = tempcv.wirename;
			tempcv_vector.wirevalue.insert(tempcv_vector.wirevalue.begin(),tempcv.wirevalue);//

			moduledetails[me].currentValue.insert(moduledetails[me].currentValue.begin()+k,tempcv_vector);
			tempcv_vector.wirevalue.pop_back();

			//cd[i].pinvalue.insert(cd[i].pinvalue.begin(), tempcv.wirevalue);// update out pin of the component

			//for(k = 0; k< moduledetails[me].currentValue.size(); k++)
				//std::cout<<"CONTENTS OF currentValue in OR ="<<moduledetails[me].currentValue[k].wirename<< " " <<moduledetails[me].currentValue[k].wirevalue[0] <<std::endl;
		}
		else if(((cd[i].componenttype == "xor")||(cd[i].componenttype == "xnor")) && (evloutput == 0))
		{
			if(cd[i].simulationID == TransId) return;
			//if(moduledetails[prev_me].componentdetails[prev_cde].componenttype.compare("module") != 0)
				cd[i].simulationID = TransId;

			temppinv = 0;

			//std::cout<<"In Component Type ="<<cd[i].componenttype<<" cd index = "<<i<<std::endl;
			for(j = 1; j < cd[i].componentsize; j++)
			{
				// find the input pin in the netList structure and correspond ComponentID to check if it is already computed
				if(CheckComponentExecStatus(cd[i],j,cd[i].busstartindex[j],net,nl,&nlIndex,&clIndex) == 1)
				{
				input_pin_cd = nl[nlIndex].componentList[clIndex].componentID;
				if(cd[input_pin_cd].simulationID < TransId) 
				{
					Simulate(argv,cd,input_pin_cd,net,nl,TransId,0);
					//std::cout<<"back to  Component Type ="<<cd[i].componenttype<<std::endl;
				}
				}
				// get the value of the pin
				found = 0;lsb = 0; msb = 0;
				for(k = 0; k< moduledetails[me].currentValue.size(); k++)//find input pin value in currentValue vector
				{
					//std::cout<<" XOR wirename  ="<<moduledetails[me].currentValue[k].wirename<<std::endl;
					if(moduledetails[me].currentValue[k].wirename.compare(cd[i].pinname[j]) == 0)
					{
						//std::cout<<" XOR wire bus start  ="<<cd[i].busstartindex[j]<<std::endl;
						//std::cout<<" XOR wire bus end  ="<<cd[i].busendindex[j]<<std::endl;

						found = 1;
						break;
					}
				}
				if(found)// get the pin in the bus
				{
					if(cd[i].busstartindex[j] != -1)// not a single wire
					{
						msb = cd[i].busstartindex[j];
						lsb = cd[i].busendindex[j];
						result = (moduledetails[me].currentValue[k].wirevalue[0] >> lsb) & ~(~0 << (msb-lsb+1));
					}
					else 
						result = moduledetails[me].currentValue[k].wirevalue[0];
				}
				else
					result = 0;

				temppinv = temppinv ^ result;//XORing function
				//std::cout<<"CONTENTS OF temppinv  ="<<temppinv<<std::endl;

			}
			if (cd[i].componenttype == "xnor")
				temppinv = ~temppinv; // XNOR is complement of XOR

			lsb = cd[i].busendindex[0];
			if(lsb != -1)// not a single wire
				temppinv = temppinv<<lsb; // move the bit to the location pointed by lsb or msb of output; both are same for XOR gate


			found = 0;// search for the out pin of the OR gate
			for(k = 0; k< moduledetails[me].currentValue.size(); k++)
			{
				//std::cout<<"CONTENTS OF currentValue  ="<<currentValue[k].wirename<<std::endl;
				if(moduledetails[me].currentValue[k].wirename.compare(cd[i].pinname[0]) == 0)
				{
					found = 1;
					break;
				}
			}
			tempcv.wirename.assign(cd[i].pinname[0]);  // initialise tempcv
			if(found)
				//tempcv.wirevalue = currentValue[k].wirevalue[0] | temppinv;
				tempcv.wirevalue = UpdateCurrentValue(moduledetails[me].currentValue[k].wirevalue[0], temppinv,cd[i].busendindex[0], cd[i].busstartindex[0]);
			else 
				tempcv.wirevalue = temppinv;

			if(found)
				moduledetails[me].currentValue.erase(moduledetails[me].currentValue.begin()+k);	// earse the exsting value

			tempcv_vector.wirename = tempcv.wirename;
			tempcv_vector.wirevalue.insert(tempcv_vector.wirevalue.begin(),tempcv.wirevalue);//

			moduledetails[me].currentValue.insert(moduledetails[me].currentValue.begin()+k,tempcv_vector);
			tempcv_vector.wirevalue.pop_back();

			//cd[i].pinvalue.insert(cd[i].pinvalue.begin(), tempcv.wirevalue);// update out pin of the component
			//for(k = 0; k< moduledetails[me].currentValue.size(); k++)
			//std::cout<<"CONTENTS OF currentValue in xor ="<<moduledetails[me].currentValue[k].wirename<< " " <<moduledetails[me].currentValue[k].wirevalue[0] <<std::endl;
		}
		else if((cd[i].componenttype == "evl_zero") && (evloutput == 0))
		{
			if(cd[i].simulationID == TransId) return;
			//if(moduledetails[prev_me].componentdetails[prev_cde].componenttype.compare("module") != 0)
				cd[i].simulationID = TransId;
			//std::cout<<"In Component Type ="<<cd[i].componenttype<<std::endl;

			//for(k = 0; k< moduledetails[me].currentValue.size(); k++)
				//std::cout<<"CONTENTS OF currentValue in evl_zero ="<<moduledetails[me].currentValue[k].wirename<<std::endl;


			for(k = 0; k < cd[i].componentsize; k++)
			{
				tempcv.wirename.assign("");  
				tempcv.wirevalue = 0;
				found = 0;
				for(j = 0; j< moduledetails[me].currentValue.size(); j++)
				{
					if(moduledetails[me].currentValue[j].wirename.compare(cd[i].pinname[k]) == 0)
					{ 
						found = 1;
						break;
					}
				}// old implementation made like evl_one

				tempcv.wirename.assign(cd[i].pinname[k]);  
				msb = cd[i].busstartindex[k];
				lsb = cd[i].busendindex[k];
				// select only the bits required to be set in the 	currentValue
				result = (0 >> lsb) & ~(~0 << (msb-lsb+1));
				if (lsb != -1)
					result = result << lsb;

				if(found)// get the pin in the bus
				{
					if(cd[i].busstartindex[k] == -1)
						tempcv.wirevalue = moduledetails[me].currentValue[j].wirevalue[0] | 0x0;
					else 
					{
						tempcv.wirevalue =  UpdateCurrentValue(moduledetails[me].currentValue[j].wirevalue[0], result, lsb, msb);
					}
				}
				else 
					tempcv.wirevalue =  UpdateCurrentValue(0, result, lsb, msb);

				if(found)
					moduledetails[me].currentValue.erase(moduledetails[me].currentValue.begin()+j);	// earse the exsting value

				tempcv_vector.wirename = tempcv.wirename;
				tempcv_vector.wirevalue.insert(tempcv_vector.wirevalue.begin(),tempcv.wirevalue);//

				moduledetails[me].currentValue.insert(moduledetails[me].currentValue.begin()+j,tempcv_vector);
				tempcv_vector.wirevalue.pop_back();

				//cd[i].pinvalue.insert(cd[i].pinvalue.begin() + k, tempcv.wirevalue);// update out pin of the component
			}
			//for(k = 0; k< moduledetails[me].currentValue.size(); k++)
			//	std::cout<<"CONTENTS OF currentValue in evl_zero ="<<moduledetails[me].currentValue[k].wirename<<std::endl;
		}
		else if((cd[i].componenttype == "evl_one") && (evloutput == 0))
		{
			if(cd[i].simulationID == TransId) return;
			//if(moduledetails[prev_me].componentdetails[prev_cde].componenttype.compare("module") != 0)
				cd[i].simulationID = TransId;

			//std::cout<<"In Component Type ="<<cd[i].componenttype<<std::endl;

			for(k = 0; k < cd[i].componentsize; k++)
			{
				tempcv.wirename.assign("");  
				tempcv.wirevalue = 0;
				found = 0;

				for(j = 0; j< moduledetails[me].currentValue.size(); j++)
				{
					if(moduledetails[me].currentValue[j].wirename.compare(cd[i].pinname[k]) == 0)
					{
						found = 1;
						break;
					}
				}
				tempcv.wirename.assign(cd[i].pinname[k]);  
				msb = cd[i].busstartindex[k];
				lsb = cd[i].busendindex[k];
				// select only the bits required to be set in the 	currentValue
				result = (0xffffffff >> lsb) & ~(~0 << (msb-lsb+1));
				if (lsb != -1)
					result = result << lsb;

				if(found)// get the pin in the bus
				{
					if(cd[i].busstartindex[k] == -1)
						tempcv.wirevalue = moduledetails[me].currentValue[j].wirevalue[0] | 0x1;
					else 
					{
						tempcv.wirevalue =  UpdateCurrentValue(moduledetails[me].currentValue[j].wirevalue[0], result, lsb, msb);
					}
				}
				else 
					tempcv.wirevalue =  UpdateCurrentValue(0, result, lsb, msb);

				if(found)
					moduledetails[me].currentValue.erase(moduledetails[me].currentValue.begin()+j);	// earse the exsting value

				tempcv_vector.wirename = tempcv.wirename;
				tempcv_vector.wirevalue.insert(tempcv_vector.wirevalue.begin(),tempcv.wirevalue);//

				moduledetails[me].currentValue.insert(moduledetails[me].currentValue.begin()+j,tempcv_vector);
				tempcv_vector.wirevalue.pop_back();

				//cd[i].pinvalue.insert(cd[i].pinvalue.begin() + k, tempcv.wirevalue);// update out pin of the component
			}
			//for(k = 0; k< moduledetails[me].currentValue.size(); k++)
			//std::cout<<"CONTENTS OF currentValue in evl_one ="<<moduledetails[me].currentValue[k].wirename<<" "<<moduledetails[me].currentValue[k].wirevalue[0]<<std::endl;

		}
		else if((cd[i].componenttype == "evl_dff") && (evloutput == 0))//Initial Release
		{	
			if(cd[i].simulationID == TransId) return;
			//if(moduledetails[prev_me].componentdetails[prev_cde].componenttype.compare("module") != 0)
				cd[i].simulationID = TransId;
			//std::cout<<"In Component Type ="<<cd[i].componenttype<<" "<<"cd index " << i <<std::endl;
// if this D Flip flop is called first time , set its OUTPUT  to 0 as initial state of each output is supposed to be 0 as per project documnet
			if(cd[i].firstsimulation==1)
			{
				//std::cout<<"In first simulation ="<<cd[i].componenttype<<" "<<"cd index " << i <<std::endl;
				found = 0;// search for the OUT pin Q of the FLIP FLOP gate
				for(k = 0; k< moduledetails[me].currentValue.size(); k++)
				{

					if(moduledetails[me].currentValue[k].wirename.compare(cd[i].pinname[0]) == 0) //pin 0 = Q
					{
						found = 1;
						break;
					}
				}
				tempcv.wirename.assign(cd[i].pinname[0]);  
				result = 0;// set output  to zero in first cycle
// redundant three lines
				lsb = cd[i].busendindex[0];
				if(lsb != -1)// not a single wire
					result = result<<lsb; // move the bit to the location pointed by lsb  or msb of output pin; both are same for dff gate

				if(found)
					tempcv.wirevalue = UpdateCurrentValue(moduledetails[me].currentValue[k].wirevalue[0], result, cd[i].busendindex[0], cd[i].busstartindex[0]);
				else 
					tempcv.wirevalue = UpdateCurrentValue(0, result, cd[i].busendindex[0], cd[i].busstartindex[0]);

				if(found)// earse the exsting value
					moduledetails[me].currentValue.erase(moduledetails[me].currentValue.begin()+k);

				tempcv_vector.wirename = tempcv.wirename;
				tempcv_vector.wirevalue.insert(tempcv_vector.wirevalue.begin(),tempcv.wirevalue);

				moduledetails[me].currentValue.insert(moduledetails[me].currentValue.begin()+k,tempcv_vector);
				// done with currentValue
				// create prevValue vector; find matching pin in  pinvalue
				for(j = 0, found = 0; j < moduledetails[me].prevValue.size(); j++)
				{
					if(moduledetails[me].prevValue[j].wirename.compare(moduledetails[me].currentValue[k].wirename) == 0) //pin 0 = Q
					{
						found = 1;
						break;
					}
				}
				tempcv_vector.wirevalue.pop_back();
//////////////////////////////////////////////////////////////////
				result = 0;// set output  to zero in first cycle
				lsb = cd[i].busendindex[0]; // redundant lines
				if(lsb != -1)// not a single wire
					result = result<<lsb; // move the bit to the location pointed by lsb  or msb of output pin; both are same for dff gate

				if(found)
					tempcv.wirevalue = UpdateCurrentValue(moduledetails[me].prevValue[j].wirevalue[0], result, cd[i].busendindex[0], cd[i].busstartindex[0]);
				else
					tempcv.wirevalue = UpdateCurrentValue(0, result, cd[i].busendindex[0], cd[i].busstartindex[0]);

				tempcv.wirename.assign(cd[i].pinname[0]);
				tempcv_vector.wirename = tempcv.wirename;
				tempcv_vector.wirevalue.insert(tempcv_vector.wirevalue.begin(),tempcv.wirevalue);
				if (found)// erase the existing value
					moduledetails[me].prevValue.erase(moduledetails[me].prevValue.begin()+j);
/////////////////////////////////////////////////////////////////
				moduledetails[me].prevValue.insert(moduledetails[me].prevValue.begin()+j,tempcv_vector);
				//std::cout<<"prevValue size ="<<prevValue.size()<<std::endl;

				//std::cout<<"CONTENTS OF currentValue in first simulation evl_dff ="<<moduledetails[me].currentValue[0].wirename<<" "<<moduledetails[me].currentValue[0].wirevalue[0]<<std::endl;
				//std::cout<<"CONTENTS OF prevValue in first simulation evl_dff ="<<prevValue[0].wirename<<" "<<prevValue[0].wirevalue[0]<<std::endl;

				tempcv_vector.wirevalue.pop_back();

				cd[i].firstsimulation =  0;//.insert(cd[i].firstsimulation.begin()+i,0);
			}
			else
			{
				//std::cout<<"In else of simulation ="<<cd[i].componenttype<<" "<<"cd index " << i <<std::endl;
//copy CV = prevValue.
				int found_cv = 0;
				for(k = 0; k< moduledetails[me].currentValue.size(); k++)
				{
					//std::cout<<"cd[i].pinname[0] printed in evl_dff ="<<cd[i].pinname[0]<<" "<<moduledetails[me].currentValue[k].wirename<<std::endl;

					if(moduledetails[me].currentValue[k].wirename.compare(cd[i].pinname[0]) == 0) //pin 0 = Q
					{
						found_cv = 1;
						break;
					}
				}
				for(j = 0; j < moduledetails[me].prevValue.size(); j++)
				{
					if(moduledetails[me].prevValue[j].wirename.compare(moduledetails[me].currentValue[k].wirename) == 0) //pin 0 = Q
					{
						found = 1;
						break;
					}
				}
// set currentValue = prevValue
				result = moduledetails[me].prevValue[j].wirevalue[0];
				/////////////////error was here
				msb = cd[i].busstartindex[0];
				lsb = cd[i].busendindex[0];
				if(found)// get the pin in the bus
				{
					if(cd[i].busstartindex[0] != -1)// not a single wire
						result = (result >> lsb) & ~(~0 << (msb-lsb+1));
					else
						result = moduledetails[me].prevValue[j].wirevalue[0];
				}
				else
					result = 0;
				if(lsb != -1)// not a single wire
					result = result<<lsb; // move the bit to the location pointed by lsb  or msb of output pin; both are same for dff gate
						unsigned uu = moduledetails[me].currentValue[k].wirevalue[0];
				if(found_cv)
					tempcv.wirevalue =  UpdateCurrentValue(moduledetails[me].currentValue[k].wirevalue[0], result, cd[i].busendindex[0], cd[i].busstartindex[0]);
				else
					tempcv.wirevalue =  UpdateCurrentValue(0, result, cd[i].busendindex[0], cd[i].busstartindex[0]);


				//////////////////////////////////////
				tempcv.wirename.assign(moduledetails[me].prevValue[j].wirename);
				//tempcv.wirevalue = prevValue[j].wirevalue[0];
				tempcv_vector.wirename = tempcv.wirename;
				tempcv_vector.wirevalue.insert(tempcv_vector.wirevalue.begin(),tempcv.wirevalue);
				if(found_cv)
					moduledetails[me].currentValue.erase(moduledetails[me].currentValue.begin()+k);
				moduledetails[me].currentValue.insert(moduledetails[me].currentValue.begin()+k,tempcv_vector);
				tempcv_vector.wirevalue.pop_back();
				tempcv_vector.wirename = "";
			}
			//std::cout<<"In normal simulation ="<<cd[i].componenttype<<" "<<"cd index " << i <<std::endl;
			if(CheckComponentExecStatus(cd[i],1,cd[i].busstartindex[1],net,nl,&nlIndex,&clIndex)==1)
			{
				input_pin_cd = nl[nlIndex].componentList[clIndex].componentID;
				//std::cout<<"\n in DFF  ="<<cd[input_pin_cd].componentname<<"cd index " << input_pin_cd <<" SimulationID "<<tt22<<std::endl;
				//std::cout<<"\n in DFF  ="<<cd[i].componentname<<" "<<"cd index " << i <<" SimID "<<cd[i].simulationID <<std::endl;
				if(cd[input_pin_cd].simulationID < TransId)
				{
					//Simulate(argv,cd,input_pin_cd,net,nl,TransId,0);
					Simulate(argv,cd,input_pin_cd,net,nl,TransId,0);

					//std::cout<<"Back to Component Type ="<<cd[i].componenttype<<" "<<"cd index " << i <<std::endl;
				}
			}
			// get the value of the input pin D
			found = 0;lsb = 0; msb = 0;
			//std::cout<<"\n D input of Flipflop ="<<cd[i].pinname[1]<<" index = "<<cd[i].busstartindex[1]<< std::endl;

			for(k = 0; k< moduledetails[me].currentValue.size(); k++)//find input pin D value in currentValue vector
			{
				//std::cout<<"CONTENTS OF currentValue  ="<<moduledetails[me].currentValue[k].wirename<<std::endl;
				//std::cout<<"cd[i].pinname[1]  ="<<cd[i].pinname[1]<<std::endl;
				if(moduledetails[me].currentValue[k].wirename.compare(cd[i].pinname[1]) == 0)
				{
					found = 1;
					break;
				}
			}
			//result = moduledetails[me].currentValue[k].wirevalue[0];
			msb = cd[i].busstartindex[1];
			lsb = cd[i].busendindex[1];
			unsigned result1;
			if(found)// get the pin in the bus (input pin D)
			{
				result1 = moduledetails[me].currentValue[k].wirevalue[0];// temp to remove
				if(cd[i].busstartindex[1] != -1)// not a single wire
					result = (moduledetails[me].currentValue[k].wirevalue[0] >> lsb) & ~(~0 << (msb-lsb+1));
				else
					result = moduledetails[me].currentValue[k].wirevalue[0];
			}
			else
				result = 0;

			lsb = cd[i].busendindex[0];
			if(lsb != -1)// not a single wire
				result = result<<lsb; // move the bit to the location pointed by lsb  or msb of output pin; both are same for AND gate
			found = 0;// search for the out pin Q of the FLIP FLOP gate
			for(k = 0; k< moduledetails[me].currentValue.size(); k++)
			{
				if(moduledetails[me].currentValue[k].wirename.compare(cd[i].pinname[0]) == 0)
				{
					found = 1;
					break;
				}
			}
// search in prevValue
			for(j = 0,found = 0; j < moduledetails[me].prevValue.size(); j++)
			{
				if(moduledetails[me].prevValue[j].wirename.compare(moduledetails[me].currentValue[k].wirename) == 0) //pin 0 = Q
				{
					found = 1;
					break;
				}
			}
			// ignoring the clk, make q = d
			if(found)
			{
				unsigned ssd = moduledetails[me].prevValue[j].wirevalue[0];// temp to remove
				//std::cout<<"\n current value of Q output of Flipflop ="<< prevValue[j].wirename<<" "<<prevValue[j].wirevalue[0]<<std::endl;
				tempcv.wirevalue =  UpdateCurrentValue(moduledetails[me].prevValue[j].wirevalue[0], result, cd[i].busendindex[0], cd[i].busstartindex[0]);
			}
			else
				tempcv.wirevalue =  UpdateCurrentValue(0, result, cd[i].busendindex[0], cd[i].busstartindex[0]);

			if(found)// earse the exsting value
				moduledetails[me].prevValue.erase(moduledetails[me].prevValue.begin()+j);

			tempcv.wirename.assign(cd[i].pinname[0]);
			tempcv_vector.wirename = tempcv.wirename;
			tempcv_vector.wirevalue.insert(tempcv_vector.wirevalue.begin(),tempcv.wirevalue);

			moduledetails[me].prevValue.insert(moduledetails[me].prevValue.begin()+j,tempcv_vector);
			//std::cout<<"CONTENTS OF prevValue in evl_dff ="<<moduledetails[me].prevValue[j].wirename<<"["<<cd[i].busstartindex[0]<<"]"<<" "<<moduledetails[me].prevValue[j].wirevalue[0]<<std::endl;

			tempcv_vector.wirevalue.pop_back();
		}
		else if(cd[i].componenttype == "evl_output")
		{
			if(evloutput == 1)
			{
				if(cd[i].simulationID == TransId) return;
				//if(moduledetails[prev_me].componentdetails[prev_cde].componenttype.compare("module") != 0)
					cd[i].simulationID = TransId;

				if(cd[i].firstsimulation==1)
				{
					std::cout<<"going into first simulation of evl_output"<<std::endl;
					output_file_name = argv+"."+ cd[i].componentname+".evl_output";
					std::ofstream evl_output_file(output_file_name.c_str());

					//std::cout<<"N_number_of_pins ="<<cd[i].componentname<<" "<<cd[i].pinname.size()<<std::endl;
					evl_output_file<<cd[i].pinname.size()<<std::endl;

					for(j = 0; j< cd[i].pinname.size();j++){
						int pinsize = cd[i].busstartindex[j] - cd[i].busendindex[j] +1;
						//std::cout<<"Pin Width = "<<pinsize <<std::endl;
						evl_output_file<<pinsize <<std::endl;
					}
					int size = moduledetails[me].trisValue.size();
					retValue = ComputeTrisValue();//
					if (size)
						moduledetails[me].trisValue.clear();
					if(retValue == -1) return;//

					// output pin values

					apb = CreatePinValueString( cd,i);
					evl_output_file<<apb<<std::endl;
					evl_output_file.close();

					cd[i].firstsimulation = 0;//[0].insert(firstsimulation[0].begin()+i,0);
				}
				else
				{
					std::cout<<"going into subsequent simulation of evl_output"<<std::endl;

					output_file_name = argv+"."+ cd[i].componentname+".evl_output";

					std::ofstream ofs;
					ofs.open (output_file_name.c_str(), std::ofstream::out | std::ofstream::app);
					int size = moduledetails[me].trisValue.size();
					retValue = ComputeTrisValue();//
					if (size)
						moduledetails[me].trisValue.clear();
					if(retValue == -1) return;//

					apb = CreatePinValueString( cd,i);

					ofs<<apb<<std::endl;
					ofs.close();
				}
			}
		}
		else if((cd[i].componenttype == "evl_input") && (evloutput == 0))
		{
			if(cd[i].simulationID == TransId) return;
			//if(moduledetails[prev_me].componentdetails[prev_cde].componenttype.compare("module") != 0)
				cd[i].simulationID = TransId;

			if(cd[i].firstsimulation==1)
			{
				std::cout<<"going into first simulation of evl_input"<<std::endl;
				readinput>>filenum;// read total no of pins
				no_of_pins = (unsigned) atol(filenum.c_str());
				//std::cout<<"\nfile input "<<filenum;

				for(k = 0; k < no_of_pins; k++)
				{
					readinput>>filenum;// read pin width
					pinwidth_read.push_back((unsigned int)atol(filenum.c_str()));
					//std::cout<<"\nfile input "<<filenum;
				}
				readinput>>filenum;// read no_of_transitions, store it

				no_of_transitions = ((unsigned int)atol(filenum.c_str()));

				for(k = 0; k < no_of_pins; )// read actual data
				{
					readinput>>filenum;// read pin data
					unsigned int tempvar = (unsigned int)strtoul (filenum.c_str(),NULL,16);
					pindata_read.push_back(tempvar);
					//std::cout<<"\nfile input "<<filenum;
					k++;
				}

				for (k= 0; k< no_of_pins; k++)
				{
					found = 0;
					tempcv.wirename.assign("");
					tempcv.wirevalue = 0;

					for(j = 0; j< moduledetails[me].currentValue.size(); j++)
					{
						if(moduledetails[me].currentValue[j].wirename.compare(cd[i].pinname[k]) == 0)
						{
							found = 1;
							break;
						}
					}

					if(cd[i].busstartindex[k] == -1)// single wire
						tempcv.wirevalue= pindata_read[k];
					else
					{
						// move the bit to the location pointed by lsb or msb; both are same for NOT gate
						pindata_read[k] = pindata_read[k] << cd[i].busendindex[k];

						if (found)
						{
							//tempcv.wirevalue =	currentValue[j].wirevalue[0] | pindata_read[k];
							tempcv.wirevalue = UpdateCurrentValue(moduledetails[me].currentValue[j].wirevalue[0], pindata_read[k],
								cd[i].busendindex[k], cd[i].busstartindex[k]);

						}
						else
							//tempcv.wirevalue =	pindata_read[k];
							tempcv.wirevalue = UpdateCurrentValue(0, pindata_read[k],
							cd[i].busendindex[k], cd[i].busstartindex[k]);


					}
					tempcv.wirename.assign(cd[i].pinname[k]);

					if(found)
						moduledetails[me].currentValue.erase(moduledetails[me].currentValue.begin()+j);	// earse the exsting value

					tempcv_vector.wirename = tempcv.wirename;
					tempcv_vector.wirevalue.insert(tempcv_vector.wirevalue.begin(),tempcv.wirevalue);
					tempcv_vector.wirevalue.insert(tempcv_vector.wirevalue.begin()+1,0);
					tempcv_vector.wirevalue.insert(tempcv_vector.wirevalue.begin()+2,0);


					//std::cout<<"\nDATA inserted at 0 "<<tempcv_vector.wirevalue[0]<<std::endl;
					//std::cout<<"\nDATA inserted at 1 "<<tempcv_vector.wirevalue[1]<<std::endl;

					moduledetails[me].currentValue.insert(moduledetails[me].currentValue.begin()+j,tempcv_vector);

					//cd[i].pinvalue.insert(cd[i].pinvalue.begin(),tempcv.wirevalue);
					tempcv_vector.wirevalue.pop_back();
				}
				no_of_transitions--;// consumed one transition
				//firstsimulation.erase(firstsimulation.begin()+i);
				//firstsimulation.insert(firstsimulation.begin()+i,0);
				cd[i].firstsimulation = 0;
				//readinput.close();
			}
			else
			{
				std::cout<<"going into subsequent simulation of evl_input"<<std::endl;
				int loop_var1;

				if(no_of_transitions == 0) // all data has been consumed, read more data
				{
					//for(loop_var1 = 0; loop_var1 < NO_OF_SIMULATIONS; loop_var1++)
					//{//kk
					readinput>>filenum;// read TansId; break if filenum is "", no more lines.
					//The last line will be used to populate currentValue
					if(filenum == "")
					{
						std::cout<<"\nfilenum is null input "<<std::endl;
						no_of_transitions = 0;
						no_of_pins = 0;
					}
					else
						no_of_transitions = ((unsigned int)atol(filenum.c_str()));

					for(j = 0; j < (unsigned)no_of_pins; )// read actual data
					{
						readinput>>filenum;// read pin data
						unsigned int tempvar = (unsigned int)strtoul (filenum.c_str(),NULL,16);
						pindata_read.push_back(tempvar);
						//std::cout<<"\nfile input "<<filenum;
						j++;
					}
					for (k= 0; k< no_of_pins; k++)
					{
					found = 0;
					tempcv.wirename.assign("");
					tempcv.wirevalue = 0;

					for(j = 0; j< moduledetails[me].currentValue.size(); j++)
					{
						if(moduledetails[me].currentValue[j].wirename.compare(cd[i].pinname[k]) == 0)
						{
							found = 1;
							break;
						}
					}

					if(cd[i].busstartindex[k] == -1)// single wire
						tempcv.wirevalue= pindata_read[k];
					else
					{
						// move the bit to the location pointed by lsb or msb; both are same for NOT gate
						pindata_read[k] = pindata_read[k] << cd[i].busendindex[k];

						if (found)
						{
							//tempcv.wirevalue =	currentValue[j].wirevalue[0] | pindata_read[k];
							tempcv.wirevalue = UpdateCurrentValue(moduledetails[me].currentValue[j].wirevalue[0], pindata_read[k],
								cd[i].busendindex[k], cd[i].busstartindex[k]);

						}
						else
							//tempcv.wirevalue =	pindata_read[k];
							tempcv.wirevalue = UpdateCurrentValue(0, pindata_read[k],
							cd[i].busendindex[k], cd[i].busstartindex[k]);


					}
					tempcv.wirename.assign(cd[i].pinname[k]);

					if(found)
						moduledetails[me].currentValue.erase(moduledetails[me].currentValue.begin()+j);	// earse the exsting value

					tempcv_vector.wirename = tempcv.wirename;
					tempcv_vector.wirevalue.insert(tempcv_vector.wirevalue.begin(),tempcv.wirevalue);
					tempcv_vector.wirevalue.insert(tempcv_vector.wirevalue.begin()+1,0);
					tempcv_vector.wirevalue.insert(tempcv_vector.wirevalue.begin()+2,0);


					//std::cout<<"\nDATA inserted at 0 "<<tempcv_vector.wirevalue[0]<<std::endl;
					//std::cout<<"\nDATA inserted at 1 "<<tempcv_vector.wirevalue[1]<<std::endl;

					moduledetails[me].currentValue.insert(moduledetails[me].currentValue.begin()+j,tempcv_vector);

					//cd[i].pinvalue.insert(cd[i].pinvalue.begin(),tempcv.wirevalue);
					tempcv_vector.wirevalue.pop_back();
				}
				}
				no_of_transitions--;// consumed one transition

				//readinput.close(); 
			} //else of subsequent simulation
			}
		else if((cd[i].componenttype == "evl_lut") && (evloutput == 0))
		{
			std::string filenum;
			std::vector<int> pinwidth_read;
			//std::vector<unsigned int> pindata_read;

			if(cd[i].simulationID == TransId) return;
			//if(moduledetails[prev_me].componentdetails[prev_cde].componenttype.compare("module") != 0)
				cd[i].simulationID = TransId;
			struct componentstruct tempcd;
			//unsigned i = cd_index;

			// split the pins in the second argument so that CheckExecStatus can be called without modification
			tempcd.componentname = cd[i].componentname;
			tempcd.componenttype = cd[i].componenttype;
			tempcd.componentsize = cd[i].busstartindex[1] - cd[i].busendindex[1] +1;
			tempcd.simulationID  = TransId;

			int temp_pp1 =  cd[i].busstartindex[1];
			int temp_pp2 =  cd[i].busendindex[1];

			//for (j = cd[i].busstartindex[1],k = 0; j >= (cd[i].busstartindex[1] - cd[i].busendindex[1] +1) ;j--,k++)
			for (j = cd[i].busendindex[1],k = 0; j <= cd[i].busstartindex[1] ;j++,k++)
			{
				tempcd.pinname.insert(tempcd.pinname.begin(),cd[i].pinname[1]);
				//tempcd.pinname[j] = cd[i].pinname[1];
				tempcd.busstartindex.insert(tempcd.busstartindex.begin(),j) ;
				tempcd.busendindex.insert(tempcd.busendindex.begin(),j) ;
//				std::cout<<"\nROM CREATED "<<j<<std::endl;
//				std::cout<<"\nROM CREATED "<<tempcd.pinname[0]<<std::endl;
//				std::cout<<"\nROM CREATED "<<tempcd.busstartindex[0]<<std::endl;
//				std::cout<<"\nROM CREATED "<<tempcd.busstartindex[0]<<std::endl;
			}
			//std::cout<<"Value  OF currentValue[1] in LUT  ="<<moduledetails[me].currentValue[1].wirevalue[0]<<std::endl;

			//check if a[4],a[5],a[6],a[7] have been computed in this cycle
			for(j = 0; j < tempcd.pinname.size() ;j++)
			{
				int yytemp = tempcd.busstartindex[j];

				if(CheckComponentExecStatus(tempcd,j,tempcd.busstartindex[j],net,nl,&nlIndex,&clIndex) ==1)
				{
				input_pin_cd = nl[nlIndex].componentList[clIndex].componentID;
				if(cd[input_pin_cd].simulationID < TransId)
				{
					//Simulate(argv,cd,input_pin_cd,net,nl,TransId,0);
					Simulate(argv,cd,input_pin_cd,net,nl,TransId,0);

					//std::cout<<"back to  Component Type ="<<cd[i].componenttype<<std::endl;
				}
				}

			}
			// Now a[7:4] is updated; Get the value from from currentValue
			found = 0;lsb = 0; msb = 0;
			for(k = 0; k< moduledetails[me].currentValue.size(); k++)//find input pin value in currentValue vector
			{
				//std::cout<<"Name OF currentValue  ="<<moduledetails[me].currentValue[k].wirename<<std::endl;
				if(moduledetails[me].currentValue[k].wirename.compare(cd[i].pinname[1]) == 0)
				{
					found = 1;
					break;
				}
			}
			//std::cout<<"Value  OF currentValue[1] in LUT ="<<moduledetails[me].currentValue[1].wirevalue[0]<<std::endl;

			if(found)// get the pin in the bus
			{
				if(cd[i].busstartindex[1] != -1)// not a single wire
				{
					msb = cd[i].busstartindex[1];
					lsb = cd[i].busendindex[1];
					result = (moduledetails[me].currentValue[k].wirevalue[0] >> lsb) & ~(~0 << (msb-lsb+1));
				}
				else
					result = moduledetails[me].currentValue[k].wirevalue[0];
			}
			else
				result = 0;
			//std::cout<<"Value  OF currentValue[1] in LUT ="<<moduledetails[me].currentValue[1].wirevalue[0]<<std::endl;

			// result is the line no to read in the LUT file after skipping the first line

			//std::cout<<"going into first simulation of evl_lut"<<std::endl;
			input_file_name = argv+"."+ cd[i].componentname+".evl_lut";
			std::ifstream readinput;
			readinput.open(input_file_name.c_str());
			if (!readinput)
				std::cout << "Can't read " << input_file_name << "." << std::endl;
			else
			{
				std::cout << "Reading " << input_file_name << "." << std::endl;

				readinput>>filenum;// read word_width
				unsigned int word_width = atoi(filenum.c_str());
				//std::cout<<"\nword_width "<<filenum;

				readinput>>filenum;// read address_width

				unsigned int address_width = atoi(filenum.c_str());
				unsigned int data_to_read = 1;

				for(k = 0; k< address_width;k++)
					data_to_read = data_to_read *2;
				//for(k = 0; k < data_to_read;k++)// read actual data
				{
					k = 0;
					do{
						readinput>>filenum;// read pin data
						if(filenum == "")
							break;
						k++;
					}while(result >= k);

					//readinput>>filenum;// read pin data
					unsigned int tempvar = (unsigned int)strtoul (filenum.c_str(),NULL,16);

					//std::cout<<"\nROM tempvar values = "<<tempvar<< "result = "<<result<<std::endl;

					//pindata_read.push_back(tempvar);
					//std::cout<<"\ndata_to_read "<<filenum;

					found = 0;
					tempcv.wirename.assign("");
					tempcv.wirevalue = 0;

					for(j = 0; j< moduledetails[me].currentValue.size(); j++)
					{
						if(moduledetails[me].currentValue[j].wirename.compare(cd[i].pinname[0]) == 0)
						{
							found = 1;
							break;
						}
					}
					if(cd[i].busstartindex[0] == -1)// single wire
						tempcv.wirevalue= tempvar;//pindata_read[k];
					else
					{
						// move the bit to the location pointed by lsb 
						//pindata_read[k] = pindata_read[k] << cd[i].busendindex[0];
						tempvar = tempvar << cd[i].busendindex[0];

						if (found)
							tempcv.wirevalue = UpdateCurrentValue(moduledetails[me].currentValue[j].wirevalue[0], tempvar,
							cd[i].busendindex[0], cd[i].busstartindex[0]);
						else 
							tempcv.wirevalue = UpdateCurrentValue(0, tempvar,
							cd[i].busendindex[0], cd[i].busstartindex[0]);
					}
					tempcv.wirename.assign(cd[i].pinname[0]);  

					if(found)
						moduledetails[me].currentValue.erase(moduledetails[me].currentValue.begin()+j);	// earse the exsting value

					tempcv_vector.wirename = tempcv.wirename;
					tempcv_vector.wirevalue.push_back(tempcv.wirevalue);

					moduledetails[me].currentValue.insert(moduledetails[me].currentValue.begin()+j,tempcv_vector);
					tempcv_vector.wirevalue.pop_back();//
				}
				for(k = 0; k < moduledetails[me].currentValue.size();k++)// clear the vector
				{
//					std::cout<<"\nROM LUT values "<<tempcv_vector.wirevalue[k]<<std::endl;
					//std::cout<<"\ncurrentValue in LUT "<<moduledetails[me].currentValue[k].wirename<<std::endl;
//					tempcv_vector.wirevalue.pop_back();
				}

				readinput.close();
			}
		}// else if of evl_lut
		else if((cd[i].componenttype == "tris") && (evloutput == 0))
		{
			if(cd[i].simulationID == TransId) return;
			//if(moduledetails[prev_me].componentdetails[prev_cde].componenttype.compare("module") != 0)
				cd[i].simulationID = TransId;

			//std::cout<<"In Component Type ="<<cd[i].componenttype<<" "<<"cd index " << i <<" SimID "<<cd[i].simulationID <<std::endl;

			// get the value of the input pin en 
			// find the en pin in the netList structure and correspond ComponentID to check if it is already computed
			if(CheckComponentExecStatus(cd[i],2,cd[i].busstartindex[2],net,nl,&nlIndex,&clIndex) == 1)
			{

			input_pin_cd = nl[nlIndex].componentList[clIndex].componentID;
			//int tt22 = cd[input_pin_cd].simulationID;// temp to remove
			if(cd[input_pin_cd].simulationID < TransId) 
			{
				//Simulate(argv,cd,input_pin_cd,net,nl,TransId,0);
				Simulate(argv,cd,input_pin_cd,net,nl,TransId,0);

				//std::cout<<"back to  Component Type ="<<cd[i].componenttype<<std::endl;
			}
			}
			found = 0;lsb = 0; msb = 0;
			for(k = 0; k< moduledetails[me].currentValue.size(); k++)//find input pin en value in moduledetails[me].currentValue vector
			{
				//	std::cout<<"CONTENTS OF moduledetails[me].currentValue  ="<<moduledetails[me].currentValue[k].wirename<<std::endl;
				if(moduledetails[me].currentValue[k].wirename.compare(cd[i].pinname[2]) == 0)
				{
					found = 1;
					break;
				}
			}		
			if(found)// get the pin in the bus (input pin en)
			{
				if(cd[i].busstartindex[2] != -1)// not a single wire
				{
					msb = cd[i].busstartindex[2];
					lsb = cd[i].busendindex[2];
					//std::cout<<"en pinvalue ="<<moduledetails[me].currentValue[k].wirevalue[0]<<std::endl;
					//std::cout<<"en pinname ="<<moduledetails[me].currentValue[k].wirename<<std::endl;

					en = (moduledetails[me].currentValue[k].wirevalue[0] >> lsb) & ~(~0 << (msb-lsb+1));
				}
				else 
					en = moduledetails[me].currentValue[k].wirevalue[0];
			}
			else
				en = 0;
			//std::cout<<"en ="<<en<<" " <<"TransID = "<<TransId<<std::endl;

			///////////////////////////////////////////////////////////////////////////////////////
			// get the index of input pin 'in' 
			// find the input pin in the netList structure and correspond ComponentID to check if it is already computed
			if (CheckComponentExecStatus(cd[i],1,cd[i].busstartindex[1],net,nl,&nlIndex,&clIndex) == 1)
			{

			input_pin_cd = nl[nlIndex].componentList[clIndex].componentID;
			//int tt22 = cd[input_pin_cd].simulationID;// temp to remove
			if(cd[input_pin_cd].simulationID < TransId) 
			{
				//Simulate(argv,cd,input_pin_cd,net,nl,TransId,0);
				Simulate(argv,cd,input_pin_cd,net,nl,TransId,0);

				//std::cout<<"back to  Component Type ="<<cd[i].componenttype<<std::endl;
			}
			}
			found = 0;lsb = 0; msb = 0;
			for(k = 0; k< moduledetails[me].currentValue.size(); k++)//find input pin D value in moduledetails[me].currentValue vector
			{
				//	std::cout<<"CONTENTS OF currentValue  ="<<moduledetails[me].currentValue[k].wirename<<std::endl;
				if(moduledetails[me].currentValue[k].wirename.compare(cd[i].pinname[1]) == 0)
				{
					found = 1;
					break;
				}
			}		
			if(found)// get the pin in the bus (input pin in)
			{
				if(cd[i].busstartindex[1] != -1)// not a single wire
				{
					msb = cd[i].busstartindex[1];
					lsb = cd[i].busendindex[1];
					in = (moduledetails[me].currentValue[k].wirevalue[0] >> lsb) & ~(~0 << (msb-lsb+1));
				}
				else 
					in = moduledetails[me].currentValue[k].wirevalue[0];
			}
			else
				in = 0;
			//std::cout<<"in ="<<in<<std::endl;

			/////////////////////////////////////////////////////////////
			// get the index of output pin 'out' in trisValue
			/*found = 0*/;lsb = 0; msb = 0;
			if(en) 
				out = in ; 
			else out = 1;// to store 1 in the array index for 'Z'

			lsb = cd[i].busendindex[0];
			if(lsb != -1) // if not a single wire
				out = out<<lsb; // move the bit to the location pointed by lsb or msb; both are same for TRIS gate

			if(en)
			{
					tempcv.wirevalue =  out;//0|out;//UpdateCurrentValue(0, out, cd[i].busendindex[0], cd[i].busstartindex[0]);
			}
			else
			{
					tempcv.wirevalue = out;//0|out;// UpdateCurrentValue(0, out, cd[i].busendindex[0], cd[i].busstartindex[0]);
			}
			unsigned tempwirevalue = 0;
			temptv_vector.wirename.assign(cd[i].pinname[0]);

			if(en)
			{
				temptv_vector.wirevalueB = tempcv.wirevalue;//insert(temptv_vector.wirevalueB.begin(),tempcv.wirevalue);
				temptv_vector.wirevalueZ = tempwirevalue;//insert(temptv_vector.wirevalueZ.begin(),tempwirevalue);
				//std::cout<<"temptv_vector wirevalueZ size in tris  ="<<temptv_vector.wirevalueZ.size()<<std::endl;
				trisvalueadd(temptv_vector, moduledetails[me].trisValue);//
				//std::cout<<"trisValue size in tris  ="<<trisValue.size()<<std::endl;

				/*if (found)
					trisValue.erase(trisValue.begin()+k);*/
				//trisValue.insert(trisValue.begin() +k,temptv_vector);//tempcv.wirevalue);
				//trisValue.push_back(temptv_vector);//
				//trisValue[k].wirevalueB.push_back(tempcv.wirevalue);//
				//trisValue[k].wirevalueZ.push_back(tempwirevalue);//
				//std::cout<<"trisValue wirevalueZ size in tris  ="<<trisValue[k].wirevalueZ.size()<<std::endl;
				//std::cout<<"trisValue wirename  in tris  ="<<trisValue[k].wirename<<std::endl;

			}
			else // store 'Z' in the next element of tempcv_vector.wirevalue
			{
/*				if (found)
					tempwirevalue = trisValue[k].wirevalueB.back();// keeping old values of '0/1', if any for other bits
				else
					tempwirevalue = 0;*/

				temptv_vector.wirevalueB = tempwirevalue;//insert(temptv_vector.wirevalueB.begin(),tempwirevalue);
				temptv_vector.wirevalueZ = tempcv.wirevalue;//insert(temptv_vector.wirevalueZ.begin(),tempcv.wirevalue);
				trisvalueadd(temptv_vector, moduledetails[me].trisValue);//
			}
			
// These  lines are just to populate currentValue structure else mux_out will not be found when matched with trisValue in CreatePinString
			//std::cout<<"CONTENTS OF cd[i] pinname MUX_IN = "<< cd[i].pinname[0] <<std::endl;
			for(k = 0,found = 0; k< moduledetails[me].currentValue.size(); k++)//find output pin D
			{
				//std::cout<<"CONTENTS OF currentValue MUX_IN ="<<moduledetails[me].currentValue[k].wirename<<std::endl;
				if(moduledetails[me].currentValue[k].wirename.compare(cd[i].pinname[0]) == 0)
				{
					found = 1;
					break;
				}
			}
			if(found)// erase the exsting value
				moduledetails[me].currentValue.erase(moduledetails[me].currentValue.begin()+k);
			tempcv_vector.wirename.assign(cd[i].pinname[0]);  // = tempcv.wirename;
			tempcv_vector.wirevalue.push_back(0);
			tempcv_vector.wirevalue.push_back(0);
			moduledetails[me].currentValue.insert(moduledetails[me].currentValue.begin()+k,tempcv_vector);
			tempcv_vector.wirevalue.pop_back();// inserted twice, so pop twice
			tempcv_vector.wirevalue.pop_back();

		}
		else if((cd[i].componenttype == "evl_clock") && (evloutput == 0))
		{
			if(cd[i].simulationID == TransId) return;
			//if(moduledetails[prev_me].componentdetails[prev_cde].componenttype.compare("module") != 0)
				cd[i].simulationID = TransId;

			for(j = 0, found = 0; j < moduledetails[me].currentValue.size(); j++)
			{
				//std::cout<<"cd[i].pinname[k] = "<< cd[i].pinname[k] <<std::endl;
				//std::cout<<"moduledetails[me].currentValue[j].wirename = "<< moduledetails[me].currentValue[j].wirename <<std::endl;
				if(cd[i].pinname[0] == moduledetails[me].currentValue[j].wirename)
				{
					found = 1; break;
				}
			}
			tempcv_vector.wirevalue.clear();
			std::string name = cd[i].pinname[0];
			tempcv_vector.wirename.assign(name);//cd[i].pinname[k]);
			if (found)
				tempcv_vector.wirevalue.push_back(moduledetails[me].currentValue[j].wirevalue[0]);
			else
				tempcv_vector.wirevalue.push_back(0);

			if(found)// erase the exsting value
				moduledetails[me].currentValue.erase(moduledetails[me].currentValue.begin()+j);
			moduledetails[me].currentValue.insert(moduledetails[me].currentValue.begin()+j,tempcv_vector);

			tempcv_vector.wirevalue.pop_back();
		}
		else// it is a module modmod
		{
			if((evloutput == 0))
			{
				if(cd[i].simulationID == TransId) return;

				//std::cout<<"In Component Type ="<<cd[i].componenttype<<" component name "<<cd[i].componentname<<std::endl;

// move it to the end after all components in the called modules are computed
				cd[i].simulationID = TransId;

				for(k = 0; k < moduledetails.size(); k++)
				{
					if(cd[i].componenttype == moduledetails[k].modulename)
					{
						break;
					}
				}
				//std::cout<<"module TO BE EXECUTED = "<< k <<std::endl;
// check if all parameters required for moduledetails[k] computed
// find the input pin in the netList structure and correspond ComponentID to check if it is already computed
				short pn = 0;
				for(m=0; m< cd[i].componentsize;m++)
				{
					//std::cout<<"moduledetails componentname in for loop = "<< moduledetails[k].moduleparameters[m].pinname<<std::endl;

					if(moduledetails[k].moduleparameters[m].type == 0)//input in
					{
						// to do each bit of the passed parameter will have to be checked for execution status
						// this issue could be there in older cases too. when in pin is wider and a smaller pin is checked for execststus
						pn = cd[i].busendindex[m];
						do
						{
							if(CheckModuleExecStatus(cd[i],m,pn,net,nl,&nlIndex,&clIndex) ==1)
							{

								input_pin_cd = nl[nlIndex].componentList[clIndex].componentID;
								//std::cout<<"cd being simulated = "<< cd[input_pin_cd].componenttype <<std::endl;

								//int tt22 = cd[input_pin_cd].simulationID;// temp to remove
								if(cd[input_pin_cd].simulationID < TransId)
								{
									//std::cout<<"moduledetails size before Simulate in MODULE= "<<moduledetails[k].currentValue.size()<<std::endl;

									Simulate(argv,cd,input_pin_cd,net,nl,TransId,0);
									//std::cout<<"back to  Component Type ="<<cd[i].componenttype<<std::endl;
								}
							}//if
							pn++;
						}while(pn<=cd[i].busstartindex[m]);
					}
				}
// load the computed data
				struct store tempstoreData;
				unsigned ww,xx,found_sd;
				for(ww = 0,found_sd = 0; ww<storeData.size();ww++)
				{
					//std::cout<<"storeData[ww].ComponentName = "<< storeData[ww].ComponentName <<std::endl;
					//std::cout<<"moduledetails[me].componentdetails[k].componentname = "<< moduledetails[me].componentdetails[i].componentname <<std::endl;
					if(storeData[ww].ComponentName.compare(moduledetails[me].componentdetails[i].componentname)==0)
					{
						found_sd = 1;
						break;
					}
				}
				me_vector.push_back(me);
				prev_me =me;
				me = k;// module in execution

				if(found_sd)
				{
					//std::cout<<" Found storedata Componentname = "<<storeData[ww].ComponentName<<std::endl;

					for(xx = 0; xx<storeData[ww].CV.size();xx++)
					{

						tempcv_vector.wirename = storeData[ww].CV[xx].wirename;
						tempcv_vector.wirevalue.insert(tempcv_vector.wirevalue.begin(),storeData[ww].CV[xx].wirevalue[0]);

						moduledetails[me].currentValue.insert(moduledetails[me].currentValue.begin(),tempcv_vector);
						tempcv_vector.wirevalue.pop_back();
					}
					for(xx = 0; xx<storeData[ww].PV.size();xx++)
					{
						tempcv_vector.wirename = storeData[ww].PV[xx].wirename;
						tempcv_vector.wirevalue.insert(tempcv_vector.wirevalue.begin(),storeData[ww].PV[xx].wirevalue[0]);

						moduledetails[me].prevValue.insert(moduledetails[me].prevValue.begin(),tempcv_vector);
						tempcv_vector.wirevalue.pop_back();
					}
					for(xx = 0; xx<storeData[ww].TV.size();xx++)
					{
						temptv_vector.wirename = storeData[ww].TV[xx].wirename;
						temptv_vector.wirevalueB = storeData[ww].TV[xx].wirevalueB;

						moduledetails[me].trisValue.insert(moduledetails[me].trisValue.begin(),temptv_vector);
						//tempcv_vector.wirevalue.pop_back();


					}
				}
				// send the input values to the called routine
				for(k = 0;k < moduledetails[me].moduleparameters.size();k++)
				{
					if(moduledetails[me].moduleparameters[k].type == 0)//input in
					{
						//std::cout<<"  moduledetails[prev_me].currentValue.size() = "<< moduledetails[prev_me].currentValue.size() <<std::endl;
						for(j = 0,found = 0; j < moduledetails[prev_me].currentValue.size(); j++)
						{
							if(cd[i].pinname[k] == moduledetails[prev_me].currentValue[j].wirename)
							{
								found = 1;
								break;
							}
						}
						tempcv_vector.wirename.assign(moduledetails[me].moduleparameters[k].pinname);

						if(found)// get the pin in the bus
						{
							if(cd[i].busstartindex[k] != -1)// not a single wire
							{
								msb = cd[i].busstartindex[k];
								lsb = cd[i].busendindex[k];
								result = (moduledetails[prev_me].currentValue[j].wirevalue[0] >> lsb) & ~(~0 << (msb-lsb+1));
							}
							else
								result = moduledetails[prev_me].currentValue[j].wirevalue[0];
						}
						else
							result = 0;

						if(found)
							tempcv_vector.wirevalue.push_back(result);//moduledetails[prev_me].currentValue[j].wirevalue[0]);
						else
							tempcv_vector.wirevalue.push_back(0); // added to take care of loop in module cpu8 in cpu8.evl

						for(j = 0,found = 0; j < moduledetails[me].currentValue.size(); j++)
						{
						
							if(moduledetails[me].moduleparameters[k].pinname == moduledetails[me].currentValue[j].wirename)
							{
								found = 1;
								break;
							}
						}
						if(found)
						{
							moduledetails[me].currentValue.erase(moduledetails[me].currentValue.begin()+j);
						}
						moduledetails[me].currentValue.push_back(tempcv_vector);
						tempcv_vector.wirevalue.pop_back();
						//moduledetails[0].currentValue.insert(moduledetails[0].currentValue.begin()+m,tempcv_vector);
					}
				}
				prev_cde = i;// store the cd index from which the module is being called
				for(k = 0;k < moduledetails[me].componentdetails.size();k++)
				{
					//std::cout<<"componemt being executed in module = "<< moduledetails[me].componentdetails[k].componenttype <<" " <<moduledetails[me].componentdetails[k].componentname<<std::endl;
					Simulate(argv,moduledetails[me].componentdetails,k,moduledetails[me].unsortednet,moduledetails[me].netList,TransId,0);
				}

				unsigned short called_me = me;
				me = me_vector.back();
				me_vector.pop_back();

// copy data back to the calling module
				for(k = 0 ; k< cd[i].pinname.size(); k++) // calling routine pins
				{
					found = 0;
					for(m = 0; m < moduledetails[me].currentValue.size(); m++)
					{
						//std::cout<<"moduledetails[me].currentValue[m].wirename = "<< moduledetails[me].currentValue[m].wirename <<" "<<moduledetails[me].currentValue[m].wirevalue[0] <<std::endl;
						if (moduledetails[me].currentValue[m].wirename == cd[i].pinname[k])
						{
							found = 1;
							break;
						}
					}

//m is the index of CV in calling module. data to be copied there
					// j and k to be matched one to one
					for(j = k ; j< moduledetails[called_me].moduleparameters.size(); j++)// called routine pins
					{
						if(moduledetails[called_me].moduleparameters[j].type == 1)// output pin
						{
							for(n = 0; n < moduledetails[called_me].currentValue.size(); n++)
							{
								if (moduledetails[called_me].moduleparameters[j].pinname == moduledetails[called_me].currentValue[n].wirename)
								{
									break;
								}
							}
							//copy cv from md[me] to md[0]
							tempcv_vector.wirename.assign(cd[i].pinname[k]);

							std::string hhstr = moduledetails[called_me].currentValue[n].wirename;
							result = moduledetails[called_me].currentValue[n].wirevalue[0];
							msb = cd[i].busstartindex[k];
							lsb = cd[i].busendindex[k];

							//m is the index of CV in calling module. data to be copied there
							if(lsb != -1) // if not a single wire
								result = result<<cd[i].busendindex[k]; // move the bit to the location pointed by lsb

							if(found)
								tempcv.wirevalue = UpdateCurrentValue(moduledetails[me].currentValue[m].wirevalue[0], result,cd[i].busendindex[k], cd[i].busstartindex[k]);
							else
								tempcv.wirevalue = result;

							tempcv_vector.wirevalue.push_back(tempcv.wirevalue);

							//tempcv_vector.wirevalue.push_back(moduledetails[called_me].currentValue[n].wirevalue[0]);
							if(found)// erase the existing value
								moduledetails[me].currentValue.erase(moduledetails[me].currentValue.begin()+m);
							moduledetails[me].currentValue.insert(moduledetails[me].currentValue.begin()+m,tempcv_vector);
							tempcv_vector.wirevalue.pop_back();
							break;
						}
					}
				}

				for(j = 0 ; j< moduledetails[called_me].moduleparameters.size(); j++)// called routine pins
				{
					if(moduledetails[called_me].moduleparameters[j].type == 1)// output pin
					{
						for(ww = 0; ww<moduledetails[me].componentdetails.size();ww++) // in all components
						{

							if((moduledetails[me].componentdetails[ww].componenttype == "evl_dff") )
							{
								if (moduledetails[me].componentdetails[ww].pinname[1].compare(cd[i].pinname[j])==0)
								{
									for(n = 0; n < moduledetails[called_me].currentValue.size(); n++)
									{
										if (moduledetails[called_me].moduleparameters[j].pinname == moduledetails[called_me].currentValue[n].wirename)
										{
											break;
										}
									}
									//Simulate(argv,moduledetails[me].componentdetails,ww,moduledetails[me].unsortednet,moduledetails[me].netList,TransId,0);
									result = moduledetails[called_me].currentValue[n].wirevalue[0];
									msb = cd[i].busstartindex[j];
									lsb = cd[i].busendindex[j];

									if(lsb != -1) // if not a single wire
										result = result<<cd[i].busendindex[j]; // move the bit to the location pointed by lsb
									// find the prevVaue index to be modified
									for(m = 0,found = 0; m < moduledetails[me].prevValue.size(); m++)
									{
										//std::cout<<"moduledetails[me].prevValue[m].wirename = "<< moduledetails[me].prevValue[m].wirename <<std::endl;
										//std::cout<<"moduledetails[me].componentdetails[ww].pinname[0] = "<< moduledetails[me].componentdetails[ww].pinname[0] <<std::endl;
										if (moduledetails[me].prevValue[m].wirename == moduledetails[me].componentdetails[ww].pinname[0])
										{
											found = 1;
											break;
										}
									}

									if(found)
										tempcv.wirevalue = UpdateCurrentValue(moduledetails[me].prevValue[m].wirevalue[0], result,cd[i].busendindex[j], cd[i].busstartindex[j]);
									else
										tempcv.wirevalue = result;

									tempcv_vector.wirevalue.push_back(tempcv.wirevalue);
									tempcv_vector.wirename = moduledetails[me].prevValue[m].wirename;
									if(found)// erase the existing value
										moduledetails[me].prevValue.erase(moduledetails[me].prevValue.begin()+m);
									moduledetails[me].prevValue.insert(moduledetails[me].prevValue.begin()+m,tempcv_vector);
									tempcv_vector.wirevalue.pop_back();
								}
							}
						}
					}
				}
				// store away the computed data

				tempstoreData.ComponentName.assign(cd[i].componentname);
				//std::cout<<"Storing to storeData ="<<moduledetails[called_me].currentValue.size()<<std::endl;

				for(ww = 0; ww<moduledetails[called_me].currentValue.size();ww++)
				{
					tempcv_vector.wirename = moduledetails[called_me].currentValue[ww].wirename;
					tempcv_vector.wirevalue.insert(tempcv_vector.wirevalue.begin(),moduledetails[called_me].currentValue[ww].wirevalue[0]);
					tempstoreData.CV.push_back(tempcv_vector);
					//tempstoreData.CV.pop_back();
					//tempcv_vector.wirevalue.pop_back();
				}

				for(ww = 0; ww<moduledetails[called_me].prevValue.size();ww++)
				{
					tempcv_vector.wirename = moduledetails[called_me].prevValue[ww].wirename;
					tempcv_vector.wirevalue.insert(tempcv_vector.wirevalue.begin(),moduledetails[called_me].prevValue[ww].wirevalue[0]);
					tempstoreData.PV.push_back(tempcv_vector);
					//tempstoreData.PV.pop_back();
					//tempcv_vector.wirevalue.pop_back();
				}
				for(ww = 0; ww<moduledetails[called_me].trisValue.size();ww++)
				{
					temptv_vector.wirename = moduledetails[called_me].trisValue[ww].wirename;
					temptv_vector.wirevalueB = moduledetails[called_me].trisValue[ww].wirevalueB;
					tempstoreData.TV.push_back(temptv_vector);
//					tempstoreData.TV.pop_back();

				}
				// check componemtname in storeData, if found , erase and insert
				int rr;
				for(rr = 0; rr< storeData.size();rr++)
				{
					if (storeData[rr].ComponentName.compare(tempstoreData.ComponentName) == 0)
					{
						found_sd = 1;break;
					}
				}
				if(found_sd)
				{
					storeData.erase(storeData.begin()+rr);	// earse the exsting value
				}

				storeData.insert(storeData.begin()+rr,tempstoreData);

				tempstoreData.CV.clear();
				tempstoreData.PV.clear();
				tempcv_vector.wirevalue.clear();
				moduledetails[called_me].currentValue.clear();
				moduledetails[called_me].prevValue.clear();
				moduledetails[called_me].trisValue.clear();
				std::cout<<"All is well"<<std::endl;

				for(k = 0 ; k< moduledetails[called_me].componentdetails.size(); k++)
				{
					moduledetails[called_me].componentdetails[k].simulationID = 0;
					if(TransId == 1)
						moduledetails[called_me].componentdetails[k].firstsimulation = 1;
				}
				//if(moduledetails[prev_me].componentdetails[prev_cde].componenttype.compare("module") != 0)
				//cd[i].simulationID = TransId;// move it to the end after all components in the called modules are computed
			}
		}
	}// for loop running through all components
}

void CheckComponentPinValidity(std::vector<struct componentstruct> & cd,std::vector<struct netstruct> & net)
{
	unsigned int i=0,j=0;//general purpose variables for loop
	int wireexists;
	struct netstruct comparenet;

	for(i = 0; i < cd.size();i++)
	{
		for(j = 0; j < cd[i].pinname.size();j++)
		{
			if(cd[i].pass[j] ==0)//pin was not found
			{
				if(cd[i].busstartindex[j] == -1)
				{
					comparenet.wirename = cd[i].pinname[j];

					wireexists = searchString(net, comparenet);//compare only wirename, not index/size
					if (wireexists == -1) 
					{
						cd.erase (cd.begin()+i);
						break; //go to look into next cd 
					}
				}
				else
				{
					//for (int k = cd[i].busendindex[j]; k< cd[i].busstartindex[j];k++)
					{
						comparenet.wirename = cd[i].pinname[j];
						comparenet.wireindex = cd[i].busstartindex[j];
						wireexists = searchNet(net,comparenet);
						if (wireexists == -1) 
						{
							cd.erase (cd.begin()+i);
							i--;// when an element is deleted all others moved up , so i-- is necessary to access the next cd
							break;//go to look into next cd
						}

					}
				}
			}
			//	if(!flag) break;//go to look into next cd 
		}
	}
}

std::string CreatePinValueString(std::vector<struct componentstruct> & cd, /*std::vector<struct currentvalue> cv,*/unsigned int index)
{
	unsigned int i=0,j=0,k=0,buflen;
	unsigned int tempvalue = 0;
	int pin_size = 0;int found = 0,lsb=0;
	char buffer [9] = "";//one space for \0 character
	char newbuffer [9] = "";//one space for \0 character
	std::string apb = "";
	std::string space = " ";
	i = index;				
	for(j = 0; j< cd[i].pinname.size();j++)
	{
		// sprintf(buffer,"%x",cd[i].pinvalue[j]);
		found = 0;
		for(k = 0; k< moduledetails[me].currentValue.size(); k++)
		{
			if(moduledetails[me].currentValue[k].wirename.compare(cd[i].pinname[j]) == 0)
			{
				found = 1;
				lsb = cd[i].busendindex[j];
				if (lsb == -1) lsb = 0;//
				break;
			}
		}

		if(found)
			tempvalue = moduledetails[me].currentValue[k].wirevalue[0];//cd[i].pinvalue[j];
		else 
			tempvalue = 0;

		//sprintf(buffer,"%x",tempvalue);//moduledetails[me].currentValue[k].wirevalue[0]);

		pin_size = (cd[i].busstartindex[j]-cd[i].busendindex[j]+1);

		if(pin_size==1)//
		{
			//sprintf(buffer,"%x",tempvalue & 0x1);
			sprintf(buffer,"%x",(tempvalue>>lsb) & 0x1);//

			apb.append(buffer);//copy last character of bus 
			apb.append(space);
		}
		else if((pin_size == 2))//bus
		{
			sprintf(buffer,"%x",(tempvalue>>lsb) & 0x3);
			apb.append(buffer);//copy last 2 characters of buffer 
			apb.append(space);
		}
		else if((pin_size == 3))//bus
		{
			sprintf(buffer,"%x",(tempvalue>>lsb) & 0x7);
			apb.append(buffer);//copy last 3 characters of buffer 
			apb.append(space);
		}
		else if((pin_size == 4))//bus
		{
			sprintf(buffer,"%x",(tempvalue>>lsb) & 0xF);
			apb.append(buffer);//copy last 4 characters of buffer 
			apb.append(space);
		}
		else if(pin_size == 5)//
		{
			sprintf(buffer,"%x",(tempvalue>>lsb) & 0x1F);
			buflen = strlen(buffer);
			if(buflen == 1)
			{
				newbuffer[0] = '0';
				strcpy(&newbuffer[1],buffer);
				apb.append(newbuffer);
			}
			else
				apb.append(buffer);
			apb.append(space);
		}
		else if((pin_size == 6))//bus
		{
			sprintf(buffer,"%x",(tempvalue>>lsb) & 0x3F);
			buflen = strlen(buffer);
			if(buflen == 1)
			{
				newbuffer[0] = '0';
				strcpy(&newbuffer[1],buffer);
				apb.append(newbuffer);
			}
			else
				apb.append(buffer); 
			apb.append(space);
		}
		else if((pin_size == 7))//bus
		{
			sprintf(buffer,"%x",(tempvalue>>lsb) & 0x7F);
			buflen = strlen(buffer);
			if(buflen == 1)
			{
				newbuffer[0] = '0';
				strcpy(&newbuffer[1],buffer);
				apb.append(newbuffer);
			}
			else
				apb.append(buffer);
			apb.append(space);
		}
		else if((pin_size == 8))//bus
		{
			sprintf(buffer,"%x",(tempvalue>>lsb) & 0xFF);
			buflen = strlen(buffer);
			if(buflen == 1)
			{
				newbuffer[0] = '0';
				strcpy(&newbuffer[1],buffer);
				apb.append(newbuffer);
			}
			else
				apb.append(buffer);
			apb.append(space);
		}
		else if((pin_size == 9))//bus
		{
			sprintf(buffer,"%x",(tempvalue>>lsb) & 0x1FF);
			buflen = strlen(buffer);
			if(buflen == 1)
			{
				newbuffer[0] = '0';
				newbuffer[1] = '0';

				strcpy(&newbuffer[2],buffer);
				apb.append(newbuffer);
			}
			else if(buflen == 2)
			{
				newbuffer[0] = '0';
				strcpy(&newbuffer[1],buffer);
				apb.append(newbuffer);
			}
			else
				apb.append(buffer);
			apb.append(space);
		}
		else if((pin_size == 10))//bus
		{
			sprintf(buffer,"%x",(tempvalue>>lsb) & 0x3FF);
			buflen = strlen(buffer);
			if(buflen == 1)
			{
				newbuffer[0] = '0';
				newbuffer[1] = '0';

				strcpy(&newbuffer[2],buffer);
				apb.append(newbuffer);
			}
			else if(buflen == 2)
			{
				newbuffer[0] = '0';
				strcpy(&newbuffer[1],buffer);
				apb.append(newbuffer);
			}
			else
				apb.append(buffer);
			apb.append(space);
		}
		else if((pin_size == 11))//bus
		{
			sprintf(buffer,"%x",(tempvalue>>lsb) & 0x7FF);
			buflen = strlen(buffer);
			if(buflen == 1)
			{
				newbuffer[0] = '0';
				newbuffer[1] = '0';
				strcpy(&newbuffer[2],buffer);
				apb.append(newbuffer);
			}
			else if(buflen == 2)
			{
				newbuffer[0] = '0';
				strcpy(&newbuffer[1],buffer);
				apb.append(newbuffer);
			}
			else
				apb.append(buffer); 
			apb.append(space);
		}
		else if((pin_size == 12))//bus
		{
			sprintf(buffer,"%x",(tempvalue>>lsb) & 0xFFF);

			buflen = strlen(buffer);
			if(buflen == 1)
			{
				newbuffer[0] = '0';
				newbuffer[1] = '0';

				strcpy(&newbuffer[2],buffer);
				apb.append(newbuffer);
			}
			else if(buflen == 2)
			{
				newbuffer[0] = '0';
				strcpy(&newbuffer[1],buffer);
				apb.append(newbuffer);
			}
			else
				apb.append(buffer);
			apb.append(space);
		}
		else if((pin_size == 13))//bus
		{
			sprintf(buffer,"%x",(tempvalue>>lsb) & 0x1FFF);
			buflen = strlen(buffer);
			if(buflen == 1)
			{
				newbuffer[0] = '0';
				newbuffer[1] = '0';
				newbuffer[2] = '0';

				strcpy(&newbuffer[3],buffer);
				apb.append(newbuffer);
			}
			else if(buflen == 2)
			{
				newbuffer[0] = '0';
				newbuffer[1] = '0';
				strcpy(&newbuffer[2],buffer);
				apb.append(newbuffer);
			}
			else if(buflen == 3)
			{
				newbuffer[0] = '0';
				strcpy(&newbuffer[1],buffer);
				apb.append(newbuffer);
			}

			else
				apb.append(buffer);
			apb.append(space);
		}
		else if((pin_size == 14))//bus
		{
			sprintf(buffer,"%x",(tempvalue>>lsb) & 0x3FFF);

			buflen = strlen(buffer);
			if(buflen == 1)
			{
				newbuffer[0] = '0';
				newbuffer[1] = '0';
				newbuffer[2] = '0';

				strcpy(&newbuffer[3],buffer);
				apb.append(newbuffer);
			}
			else if(buflen == 2)
			{
				newbuffer[0] = '0';
				newbuffer[1] = '0';
				strcpy(&newbuffer[2],buffer);
				apb.append(newbuffer);
			}
			else if(buflen == 3)
			{
				newbuffer[0] = '0';
				strcpy(&newbuffer[1],buffer);
				apb.append(newbuffer);
			}

			else
				apb.append(buffer);
			apb.append(space);
		}
		else if((pin_size == 15))//bus
		{
			sprintf(buffer,"%x",(tempvalue>>lsb) & 0x7FFF);

			buflen = strlen(buffer);
			if(buflen == 1)
			{
				newbuffer[0] = '0';
				newbuffer[1] = '0';
				newbuffer[2] = '0';

				strcpy(&newbuffer[3],buffer);
				apb.append(newbuffer);
			}
			else if(buflen == 2)
			{
				newbuffer[0] = '0';
				newbuffer[1] = '0';
				strcpy(&newbuffer[2],buffer);
				apb.append(newbuffer);
			}
			else if(buflen == 3)
			{
				newbuffer[0] = '0';
				strcpy(&newbuffer[1],buffer);
				apb.append(newbuffer);
			}

			else
				apb.append(buffer);
			apb.append(space);
		}
		else if((pin_size == 16))//bus
		{
			sprintf(buffer,"%x",(tempvalue>>lsb) & 0xFFFF);

			buflen = strlen(buffer);
			if(buflen == 1)
			{
				newbuffer[0] = '0';
				newbuffer[1] = '0';
				newbuffer[2] = '0';

				strcpy(&newbuffer[3],buffer);
				apb.append(newbuffer);
			}
			else if(buflen == 2)
			{
				newbuffer[0] = '0';
				newbuffer[1] = '0';
				strcpy(&newbuffer[2],buffer);
				apb.append(newbuffer);
			}
			else if(buflen == 3)
			{
				newbuffer[0] = '0';
				strcpy(&newbuffer[1],buffer);
				apb.append(newbuffer);
			}

			else
				apb.append(buffer);
			apb.append(space);
		}
		else if((pin_size == 17))//bus
		{
			sprintf(buffer,"%x",(tempvalue>>lsb) & 0x1FFFF);

			buflen = strlen(buffer);
			if(buflen == 1)
			{
				newbuffer[0] = '0';
				newbuffer[1] = '0';
				newbuffer[2] = '0';
				newbuffer[3] = '0';

				strcpy(&newbuffer[4],buffer);
				apb.append(newbuffer);
			}
			else if(buflen == 2)
			{
				newbuffer[0] = '0';
				newbuffer[1] = '0';
				newbuffer[2] = '0';
				strcpy(&newbuffer[3],buffer);
				apb.append(newbuffer);
			}
			else if(buflen == 3)
			{
				newbuffer[0] = '0';
				newbuffer[1] = '0';
				strcpy(&newbuffer[2],buffer);
				apb.append(newbuffer);
			}
			else if(buflen == 4)
			{
				newbuffer[0] = '0';
				strcpy(&newbuffer[1],buffer);
				apb.append(newbuffer);
			}
			else
				apb.append(buffer);
			apb.append(space);
		}
		else if((pin_size == 18))//bus
		{
			sprintf(buffer,"%x",(tempvalue>>lsb) & 0x3FFFF);

			buflen = strlen(buffer);
			if(buflen == 1)
			{
				newbuffer[0] = '0';
				newbuffer[1] = '0';
				newbuffer[2] = '0';
				newbuffer[3] = '0';

				strcpy(&newbuffer[4],buffer);
				apb.append(newbuffer);
			}
			else if(buflen == 2)
			{
				newbuffer[0] = '0';
				newbuffer[1] = '0';
				newbuffer[2] = '0';
				strcpy(&newbuffer[3],buffer);
				apb.append(newbuffer);
			}
			else if(buflen == 3)
			{
				newbuffer[0] = '0';
				newbuffer[1] = '0';
				strcpy(&newbuffer[2],buffer);
				apb.append(newbuffer);
			}
			else if(buflen == 4)
			{
				newbuffer[0] = '0';
				strcpy(&newbuffer[1],buffer);
				apb.append(newbuffer);
			}
			else
				apb.append(buffer);
			apb.append(space);
		}
		else if((pin_size == 19))//bus
		{
			sprintf(buffer,"%x",(tempvalue>>lsb) & 0x7FFFF);

			buflen = strlen(buffer);
			if(buflen == 1)
			{
				newbuffer[0] = '0';
				newbuffer[1] = '0';
				newbuffer[2] = '0';
				newbuffer[3] = '0';

				strcpy(&newbuffer[4],buffer);
				apb.append(newbuffer);
			}
			else if(buflen == 2)
			{
				newbuffer[0] = '0';
				newbuffer[1] = '0';
				newbuffer[2] = '0';
				strcpy(&newbuffer[3],buffer);
				apb.append(newbuffer);
			}
			else if(buflen == 3)
			{
				newbuffer[0] = '0';
				newbuffer[1] = '0';
				strcpy(&newbuffer[2],buffer);
				apb.append(newbuffer);
			}
			else if(buflen == 4)
			{
				newbuffer[0] = '0';
				strcpy(&newbuffer[1],buffer);
				apb.append(newbuffer);
			}
			else
				apb.append(buffer);
			apb.append(space);
		}
		else if((pin_size == 20))//bus
		{
			sprintf(buffer,"%x",(tempvalue>>lsb) & 0xFFFFF);

			buflen = strlen(buffer);
			if(buflen == 1)
			{
				newbuffer[0] = '0';
				newbuffer[1] = '0';
				newbuffer[2] = '0';
				newbuffer[3] = '0';

				strcpy(&newbuffer[4],buffer);
				apb.append(newbuffer);
			}
			else if(buflen == 2)
			{
				newbuffer[0] = '0';
				newbuffer[1] = '0';
				newbuffer[2] = '0';
				strcpy(&newbuffer[3],buffer);
				apb.append(newbuffer);
			}
			else if(buflen == 3)
			{
				newbuffer[0] = '0';
				newbuffer[1] = '0';
				strcpy(&newbuffer[2],buffer);
				apb.append(newbuffer);
			}
			else if(buflen == 4)
			{
				newbuffer[0] = '0';
				strcpy(&newbuffer[1],buffer);
				apb.append(newbuffer);
			}
			else
				apb.append(buffer);
			apb.append(space);
		}
		else if((pin_size == 21))//bus
		{
			sprintf(buffer,"%x",(tempvalue>>lsb) & 0x1FFFFF);

			buflen = strlen(buffer);
			if(buflen == 1)
			{
				newbuffer[0] = '0';
				newbuffer[1] = '0';
				newbuffer[2] = '0';
				newbuffer[3] = '0';
				newbuffer[4] = '0';

				strcpy(&newbuffer[5],buffer);
				apb.append(newbuffer);
			}
			else if(buflen == 2)
			{
				newbuffer[0] = '0';
				newbuffer[1] = '0';
				newbuffer[2] = '0';
				newbuffer[3] = '0';

				strcpy(&newbuffer[4],buffer);
				apb.append(newbuffer);
			}
			else if(buflen == 3)
			{
				newbuffer[0] = '0';
				newbuffer[1] = '0';
				newbuffer[2] = '0';

				strcpy(&newbuffer[3],buffer);
				apb.append(newbuffer);
			}
			else if(buflen == 4)
			{
				newbuffer[0] = '0';
				newbuffer[1] = '0';
				strcpy(&newbuffer[2],buffer);
				apb.append(newbuffer);
			}
			else if(buflen == 5)
			{
				newbuffer[0] = '0';
				strcpy(&newbuffer[1],buffer);
				apb.append(newbuffer);
			}
			else
				apb.append(buffer);
			apb.append(space);
		}

		else if((pin_size == 22))//bus
		{
			sprintf(buffer,"%x",(tempvalue>>lsb) & 0x3FFFFF);

			buflen = strlen(buffer);
			if(buflen == 1)
			{
				newbuffer[0] = '0';
				newbuffer[1] = '0';
				newbuffer[2] = '0';
				newbuffer[3] = '0';
				newbuffer[4] = '0';

				strcpy(&newbuffer[5],buffer);
				apb.append(newbuffer);
			}
			else if(buflen == 2)
			{
				newbuffer[0] = '0';
				newbuffer[1] = '0';
				newbuffer[2] = '0';
				newbuffer[3] = '0';

				strcpy(&newbuffer[4],buffer);
				apb.append(newbuffer);
			}
			else if(buflen == 3)
			{
				newbuffer[0] = '0';
				newbuffer[1] = '0';
				newbuffer[2] = '0';

				strcpy(&newbuffer[3],buffer);
				apb.append(newbuffer);
			}
			else if(buflen == 4)
			{
				newbuffer[0] = '0';
				newbuffer[1] = '0';
				strcpy(&newbuffer[2],buffer);
				apb.append(newbuffer);
			}
			else if(buflen == 5)
			{
				newbuffer[0] = '0';
				strcpy(&newbuffer[1],buffer);
				apb.append(newbuffer);
			}
			else
				apb.append(buffer);
			apb.append(space);
		}
		else if(pin_size == 23)//bus
		{
			sprintf(buffer,"%x",(tempvalue>>lsb) & 0x7FFFFF);

			buflen = strlen(buffer);
			if(buflen == 1)
			{
				newbuffer[0] = '0';
				newbuffer[1] = '0';
				newbuffer[2] = '0';
				newbuffer[3] = '0';
				newbuffer[4] = '0';

				strcpy(&newbuffer[5],buffer);
				apb.append(newbuffer);
			}
			else if(buflen == 2)
			{
				newbuffer[0] = '0';
				newbuffer[1] = '0';
				newbuffer[2] = '0';
				newbuffer[3] = '0';

				strcpy(&newbuffer[4],buffer);
				apb.append(newbuffer);
			}
			else if(buflen == 3)
			{
				newbuffer[0] = '0';
				newbuffer[1] = '0';
				newbuffer[2] = '0';

				strcpy(&newbuffer[3],buffer);
				apb.append(newbuffer);
			}
			else if(buflen == 4)
			{
				newbuffer[0] = '0';
				newbuffer[1] = '0';
				strcpy(&newbuffer[2],buffer);
				apb.append(newbuffer);
			}
			else if(buflen == 5)
			{
				newbuffer[0] = '0';
				strcpy(&newbuffer[1],buffer);
				apb.append(newbuffer);
			}
			else
				apb.append(buffer);
			apb.append(space);
		}
		else if((pin_size == 24))//bus
		{
			sprintf(buffer,"%x",(tempvalue>>lsb) & 0xFFFFFF);

			buflen = strlen(buffer);
			if(buflen == 1)
			{
				newbuffer[0] = '0';
				newbuffer[1] = '0';
				newbuffer[2] = '0';
				newbuffer[3] = '0';
				newbuffer[4] = '0';

				strcpy(&newbuffer[5],buffer);
				apb.append(newbuffer);
			}
			else if(buflen == 2)
			{
				newbuffer[0] = '0';
				newbuffer[1] = '0';
				newbuffer[2] = '0';
				newbuffer[3] = '0';

				strcpy(&newbuffer[4],buffer);
				apb.append(newbuffer);
			}
			else if(buflen == 3)
			{
				newbuffer[0] = '0';
				newbuffer[1] = '0';
				newbuffer[2] = '0';

				strcpy(&newbuffer[3],buffer);
				apb.append(newbuffer);
			}
			else if(buflen == 4)
			{
				newbuffer[0] = '0';
				newbuffer[1] = '0';
				strcpy(&newbuffer[2],buffer);
				apb.append(newbuffer);
			}
			else if(buflen == 5)
			{
				newbuffer[0] = '0';
				strcpy(&newbuffer[1],buffer);
				apb.append(newbuffer);
			}
			else
				apb.append(buffer);
			apb.append(space);
		}

		else if((pin_size == 25))//bus
		{
			sprintf(buffer,"%x",(tempvalue>>lsb) & 0x1FFFFFF);

			buflen = strlen(buffer);
			if(buflen == 1)
			{
				newbuffer[0] = '0';
				newbuffer[1] = '0';
				newbuffer[2] = '0';
				newbuffer[3] = '0';
				newbuffer[4] = '0';
				newbuffer[5] = '0';

				strcpy(&newbuffer[6],buffer);
				apb.append(newbuffer);
			}
			else if(buflen == 2)
			{
				newbuffer[0] = '0';
				newbuffer[1] = '0';
				newbuffer[2] = '0';
				newbuffer[3] = '0';
				newbuffer[4] = '0';

				strcpy(&newbuffer[5],buffer);
				apb.append(newbuffer);
			}
			else if(buflen == 3)
			{
				newbuffer[0] = '0';
				newbuffer[1] = '0';
				newbuffer[2] = '0';
				newbuffer[3] = '0';

				strcpy(&newbuffer[4],buffer);
				apb.append(newbuffer);
			}
			else if(buflen == 4)
			{
				newbuffer[0] = '0';
				newbuffer[1] = '0';
				newbuffer[2] = '0';
				strcpy(&newbuffer[3],buffer);
				apb.append(newbuffer);
			}
			else if(buflen == 5)
			{
				newbuffer[0] = '0';
				newbuffer[1] = '0';
				strcpy(&newbuffer[2],buffer);
				apb.append(newbuffer);
			}
			else if(buflen == 6)
			{
				newbuffer[0] = '0';
				strcpy(&newbuffer[1],buffer);
				apb.append(newbuffer);
			}
			else
				apb.append(buffer);
			apb.append(space);
		}
		else if((pin_size == 26))//bus
		{
			sprintf(buffer,"%x",(tempvalue>>lsb) & 0x3FFFFFF);

			buflen = strlen(buffer);
			if(buflen == 1)
			{
				newbuffer[0] = '0';
				newbuffer[1] = '0';
				newbuffer[2] = '0';
				newbuffer[3] = '0';
				newbuffer[4] = '0';
				newbuffer[5] = '0';

				strcpy(&newbuffer[6],buffer);
				apb.append(newbuffer);
			}
			else if(buflen == 2)
			{
				newbuffer[0] = '0';
				newbuffer[1] = '0';
				newbuffer[2] = '0';
				newbuffer[3] = '0';
				newbuffer[4] = '0';

				strcpy(&newbuffer[5],buffer);
				apb.append(newbuffer);
			}
			else if(buflen == 3)
			{
				newbuffer[0] = '0';
				newbuffer[1] = '0';
				newbuffer[2] = '0';
				newbuffer[3] = '0';

				strcpy(&newbuffer[4],buffer);
				apb.append(newbuffer);
			}
			else if(buflen == 4)
			{
				newbuffer[0] = '0';
				newbuffer[1] = '0';
				newbuffer[2] = '0';
				strcpy(&newbuffer[3],buffer);
				apb.append(newbuffer);
			}
			else if(buflen == 5)
			{
				newbuffer[0] = '0';
				newbuffer[1] = '0';
				strcpy(&newbuffer[2],buffer);
				apb.append(newbuffer);
			}
			else if(buflen == 6)
			{
				newbuffer[0] = '0';
				strcpy(&newbuffer[1],buffer);
				apb.append(newbuffer);
			}
			else
				apb.append(buffer);
			apb.append(space);
		}
		else if((pin_size == 27))//bus
		{
			sprintf(buffer,"%x",(tempvalue>>lsb) & 0x7FFFFFF);

			buflen = strlen(buffer);
			if(buflen == 1)
			{
				newbuffer[0] = '0';
				newbuffer[1] = '0';
				newbuffer[2] = '0';
				newbuffer[3] = '0';
				newbuffer[4] = '0';
				newbuffer[5] = '0';

				strcpy(&newbuffer[6],buffer);
				apb.append(newbuffer);
			}
			else if(buflen == 2)
			{
				newbuffer[0] = '0';
				newbuffer[1] = '0';
				newbuffer[2] = '0';
				newbuffer[3] = '0';
				newbuffer[4] = '0';

				strcpy(&newbuffer[5],buffer);
				apb.append(newbuffer);
			}
			else if(buflen == 3)
			{
				newbuffer[0] = '0';
				newbuffer[1] = '0';
				newbuffer[2] = '0';
				newbuffer[3] = '0';

				strcpy(&newbuffer[4],buffer);
				apb.append(newbuffer);
			}
			else if(buflen == 4)
			{
				newbuffer[0] = '0';
				newbuffer[1] = '0';
				newbuffer[2] = '0';
				strcpy(&newbuffer[3],buffer);
				apb.append(newbuffer);
			}
			else if(buflen == 5)
			{
				newbuffer[0] = '0';
				newbuffer[1] = '0';
				strcpy(&newbuffer[2],buffer);
				apb.append(newbuffer);
			}
			else if(buflen == 6)
			{
				newbuffer[0] = '0';
				strcpy(&newbuffer[1],buffer);
				apb.append(newbuffer);
			}
			else
				apb.append(buffer);
			apb.append(space);
		}
		else if((pin_size == 28))//bus
		{
			sprintf(buffer,"%x",(tempvalue>>lsb) & 0xFFFFFFF);

			buflen = strlen(buffer);
			if(buflen == 1)
			{
				newbuffer[0] = '0';
				newbuffer[1] = '0';
				newbuffer[2] = '0';
				newbuffer[3] = '0';
				newbuffer[4] = '0';
				newbuffer[5] = '0';

				strcpy(&newbuffer[6],buffer);
				apb.append(newbuffer);
			}
			else if(buflen == 2)
			{
				newbuffer[0] = '0';
				newbuffer[1] = '0';
				newbuffer[2] = '0';
				newbuffer[3] = '0';
				newbuffer[4] = '0';

				strcpy(&newbuffer[5],buffer);
				apb.append(newbuffer);
			}
			else if(buflen == 3)
			{
				newbuffer[0] = '0';
				newbuffer[1] = '0';
				newbuffer[2] = '0';
				newbuffer[3] = '0';

				strcpy(&newbuffer[4],buffer);
				apb.append(newbuffer);
			}
			else if(buflen == 4)
			{
				newbuffer[0] = '0';
				newbuffer[1] = '0';
				newbuffer[2] = '0';
				strcpy(&newbuffer[3],buffer);
				apb.append(newbuffer);
			}
			else if(buflen == 5)
			{
				newbuffer[0] = '0';
				newbuffer[1] = '0';
				strcpy(&newbuffer[2],buffer);
				apb.append(newbuffer);
			}
			else if(buflen == 6)
			{
				newbuffer[0] = '0';
				strcpy(&newbuffer[1],buffer);
				apb.append(newbuffer);
			}
			else
				apb.append(buffer);
			apb.append(space);
		}

		else if((pin_size == 29))//bus
		{
			sprintf(buffer,"%x",(tempvalue>>lsb) & 0x1FFFFFFF);

			buflen = strlen(buffer);
			if(buflen == 1)
			{
				newbuffer[0] = '0';
				newbuffer[1] = '0';
				newbuffer[2] = '0';
				newbuffer[3] = '0';
				newbuffer[4] = '0';
				newbuffer[5] = '0';
				newbuffer[6] = '0';

				strcpy(&newbuffer[7],buffer);
				apb.append(newbuffer);
			}
			else if(buflen == 2)
			{
				newbuffer[0] = '0';
				newbuffer[1] = '0';
				newbuffer[2] = '0';
				newbuffer[3] = '0';
				newbuffer[4] = '0';
				newbuffer[5] = '0';

				strcpy(&newbuffer[6],buffer);
				apb.append(newbuffer);
			}
			else if(buflen == 3)
			{
				newbuffer[0] = '0';
				newbuffer[1] = '0';
				newbuffer[2] = '0';
				newbuffer[3] = '0';
				newbuffer[4] = '0';

				strcpy(&newbuffer[5],buffer);
				apb.append(newbuffer);
			}
			else if(buflen == 4)
			{
				newbuffer[0] = '0';
				newbuffer[1] = '0';
				newbuffer[2] = '0';
				newbuffer[3] = '0';

				strcpy(&newbuffer[4],buffer);
				apb.append(newbuffer);
			}
			else if(buflen == 5)
			{
				newbuffer[0] = '0';
				newbuffer[1] = '0';
				newbuffer[2] = '0';

				strcpy(&newbuffer[3],buffer);
				apb.append(newbuffer);
			}
			else if(buflen == 6)
			{
				newbuffer[0] = '0';
				newbuffer[1] = '0';

				strcpy(&newbuffer[2],buffer);
				apb.append(newbuffer);
			}
			else if(buflen == 7)
			{
				newbuffer[0] = '0';

				strcpy(&newbuffer[1],buffer);
				apb.append(newbuffer);
			}
			else
				apb.append(buffer);
			apb.append(space);
		}
		else if((pin_size == 30))//bus
		{
			sprintf(buffer,"%x",(tempvalue>>lsb) & 0x3FFFFFFF);

			buflen = strlen(buffer);
			if(buflen == 1)
			{
				newbuffer[0] = '0';
				newbuffer[1] = '0';
				newbuffer[2] = '0';
				newbuffer[3] = '0';
				newbuffer[4] = '0';
				newbuffer[5] = '0';
				newbuffer[6] = '0';

				strcpy(&newbuffer[7],buffer);
				apb.append(newbuffer);
			}
			else if(buflen == 2)
			{
				newbuffer[0] = '0';
				newbuffer[1] = '0';
				newbuffer[2] = '0';
				newbuffer[3] = '0';
				newbuffer[4] = '0';
				newbuffer[5] = '0';

				strcpy(&newbuffer[6],buffer);
				apb.append(newbuffer);
			}
			else if(buflen == 3)
			{
				newbuffer[0] = '0';
				newbuffer[1] = '0';
				newbuffer[2] = '0';
				newbuffer[3] = '0';
				newbuffer[4] = '0';

				strcpy(&newbuffer[5],buffer);
				apb.append(newbuffer);
			}
			else if(buflen == 4)
			{
				newbuffer[0] = '0';
				newbuffer[1] = '0';
				newbuffer[2] = '0';
				newbuffer[3] = '0';

				strcpy(&newbuffer[4],buffer);
				apb.append(newbuffer);
			}
			else if(buflen == 5)
			{
				newbuffer[0] = '0';
				newbuffer[1] = '0';
				newbuffer[2] = '0';

				strcpy(&newbuffer[3],buffer);
				apb.append(newbuffer);
			}
			else if(buflen == 6)
			{
				newbuffer[0] = '0';
				newbuffer[1] = '0';

				strcpy(&newbuffer[2],buffer);
				apb.append(newbuffer);
			}
			else if(buflen == 7)
			{
				newbuffer[0] = '0';

				strcpy(&newbuffer[1],buffer);
				apb.append(newbuffer);
			}
			else
				apb.append(buffer);
			apb.append(space);
		}
		else if((pin_size == 31))//bus
		{
			sprintf(buffer,"%x",(tempvalue>>lsb) & 0x7FFFFFFF);

			buflen = strlen(buffer);
			if(buflen == 1)
			{
				newbuffer[0] = '0';
				newbuffer[1] = '0';
				newbuffer[2] = '0';
				newbuffer[3] = '0';
				newbuffer[4] = '0';
				newbuffer[5] = '0';
				newbuffer[6] = '0';

				strcpy(&newbuffer[7],buffer);
				apb.append(newbuffer);
			}
			else if(buflen == 2)
			{
				newbuffer[0] = '0';
				newbuffer[1] = '0';
				newbuffer[2] = '0';
				newbuffer[3] = '0';
				newbuffer[4] = '0';
				newbuffer[5] = '0';

				strcpy(&newbuffer[6],buffer);
				apb.append(newbuffer);
			}
			else if(buflen == 3)
			{
				newbuffer[0] = '0';
				newbuffer[1] = '0';
				newbuffer[2] = '0';
				newbuffer[3] = '0';
				newbuffer[4] = '0';

				strcpy(&newbuffer[5],buffer);
				apb.append(newbuffer);
			}
			else if(buflen == 4)
			{
				newbuffer[0] = '0';
				newbuffer[1] = '0';
				newbuffer[2] = '0';
				newbuffer[3] = '0';

				strcpy(&newbuffer[4],buffer);
				apb.append(newbuffer);
			}
			else if(buflen == 5)
			{
				newbuffer[0] = '0';
				newbuffer[1] = '0';
				newbuffer[2] = '0';

				strcpy(&newbuffer[3],buffer);
				apb.append(newbuffer);
			}
			else if(buflen == 6)
			{
				newbuffer[0] = '0';
				newbuffer[1] = '0';

				strcpy(&newbuffer[2],buffer);
				apb.append(newbuffer);
			}
			else if(buflen == 7)
			{
				newbuffer[0] = '0';

				strcpy(&newbuffer[1],buffer);
				apb.append(newbuffer);
			}
			else
				apb.append(buffer);
			apb.append(space);
		}
		else if((pin_size == 32))//bus
		{
			sprintf(buffer,"%x",(tempvalue>>lsb) & 0xFFFFFFFF);

			buflen = strlen(buffer);
			if(buflen == 1)
			{
				newbuffer[0] = '0';
				newbuffer[1] = '0';
				newbuffer[2] = '0';
				newbuffer[3] = '0';
				newbuffer[4] = '0';
				newbuffer[5] = '0';
				newbuffer[6] = '0';

				strcpy(&newbuffer[7],buffer);
				apb.append(newbuffer);
			}
			else if(buflen == 2)
			{
				newbuffer[0] = '0';
				newbuffer[1] = '0';
				newbuffer[2] = '0';
				newbuffer[3] = '0';
				newbuffer[4] = '0';
				newbuffer[5] = '0';

				strcpy(&newbuffer[6],buffer);
				apb.append(newbuffer);
			}
			else if(buflen == 3)
			{
				newbuffer[0] = '0';
				newbuffer[1] = '0';
				newbuffer[2] = '0';
				newbuffer[3] = '0';
				newbuffer[4] = '0';

				strcpy(&newbuffer[5],buffer);
				apb.append(newbuffer);
			}
			else if(buflen == 4)
			{
				newbuffer[0] = '0';
				newbuffer[1] = '0';
				newbuffer[2] = '0';
				newbuffer[3] = '0';

				strcpy(&newbuffer[4],buffer);
				apb.append(newbuffer);
			}
			else if(buflen == 5)
			{
				newbuffer[0] = '0';
				newbuffer[1] = '0';
				newbuffer[2] = '0';

				strcpy(&newbuffer[3],buffer);
				apb.append(newbuffer);
			}
			else if(buflen == 6)
			{
				newbuffer[0] = '0';
				newbuffer[1] = '0';

				strcpy(&newbuffer[2],buffer);
				apb.append(newbuffer);
			}
			else if(buflen == 7)
			{
				newbuffer[0] = '0';

				strcpy(&newbuffer[1],buffer);
				apb.append(newbuffer);
			}
			else
				apb.append(buffer);
			apb.append(space);
		}
	}
	//for(k = 0; k < moduledetails[me].currentValue.size();k++)
	//	std::cout<<"CONTENTS OF currentValue in evl_output ="<<moduledetails[me].currentValue[k].wirename<<std::endl;
	std::transform(apb.begin(), apb.end(), apb.begin(), ::toupper);
	//std::cout<<"CONTENTS OF apb ="<<apb<<std::endl;

	return apb;			
}

std::string CreateInputFile(std::vector<struct componentstruct> & cd,int index,int TransId)
{
	int i=0;
	unsigned int j=0;

	char buffer [9] = "";//one space for \0 character
	std::string apb = "";
	std::string space = " ";

	i = index;		
	unsigned int tempvalue;

	sprintf(buffer,"%d ",TransId);
	apb.append(buffer);//copy TransId
	apb.append(space);

	for(j = 0; j< cd[i].pinname.size();j++)
	{
		int pin_size = (cd[i].busstartindex[j]-cd[i].busendindex[j]+1);

		if(pin_size==1)
		{
			tempvalue = 0xABCD;//0x55555555;;
			sprintf(buffer,"%x",tempvalue & 0x1);

			apb.append(buffer);//copy last character of bus 
			apb.append(space);
		}
		else if(pin_size==2)//
		{
			tempvalue = 0;//0x55555555;;
			sprintf(buffer,"%x",tempvalue & 0x3);

			apb.append(buffer);//copy last character of bus 
			apb.append(space);
		}
		else if(pin_size==3)//
		{
			tempvalue = 0;//0x55555555;;

			sprintf(buffer,"%x",tempvalue & 0x7);

			apb.append(buffer);//copy last character of bus 
			apb.append(space);
		}
		else if(pin_size==4)//
		{
			tempvalue = 0xABCD;//0x55555555;;

			sprintf(buffer,"%x",tempvalue & 0xF);

			apb.append(buffer);//copy last character of bus 
			apb.append(space);
		}
		else if(pin_size==5)//
		{
			tempvalue = 0;//0x55555555;;
			sprintf(buffer,"%x",tempvalue & 0x1F);

			apb.append(buffer);//copy last character of bus 
			apb.append(space);
		}
		else if(pin_size==6)//
		{
			tempvalue = 0;//0x55555555;;

			sprintf(buffer,"%x",tempvalue & 0x3F);

			apb.append(buffer);//copy last character of bus 
			apb.append(space);
		}
		else if(pin_size==7)//
		{
			tempvalue = 0;//0x55555555;;
			sprintf(buffer,"%x",tempvalue & 0x7F);

			apb.append(buffer);//copy last character of bus 
			apb.append(space);
		}
		else if(pin_size==8)
		{
			tempvalue = 0x55;

			sprintf(buffer,"%x",tempvalue & 0xFF);

			apb.append(buffer);//copy last character of bus 
			apb.append(space);
		}
		else if(pin_size==9)
		{
			tempvalue = 0;//0x55555555;;
			sprintf(buffer,"%x",tempvalue & 0x1FF);

			apb.append(buffer);//copy last character of bus 
			apb.append(space);
		}
		else if(pin_size==10)
		{
			tempvalue = 0;//0x55555555;;

			sprintf(buffer,"%x",tempvalue & 0x3FF);

			apb.append(buffer);//copy last character of bus 
			apb.append(space);
		}
		else if(pin_size==11)
		{
			tempvalue = 0;//0x55555555;;
			sprintf(buffer,"%x",tempvalue & 0x7FF);

			apb.append(buffer);//copy last character of bus 
			apb.append(space);
		}
		else if(pin_size==12)
		{
			tempvalue = 0;//0x55555555;;
			sprintf(buffer,"%x",tempvalue & 0xFFF);

			apb.append(buffer);//copy last character of bus 
			apb.append(space);
		}
		else if(pin_size==13)
		{
			tempvalue = 0;//0x55555555;;

			sprintf(buffer,"%x",tempvalue & 0x1FFF);

			apb.append(buffer);//copy last character of bus 
			apb.append(space);
		}
		else if(pin_size==14)
		{
			tempvalue = 0;//0x55555555;;
			sprintf(buffer,"%x",tempvalue & 0x3FFF);

			apb.append(buffer);//copy last character of bus 
			apb.append(space);
		}
		else if(pin_size==15)
		{
			tempvalue = 0xABCABCAB;//0x55555555;;
			sprintf(buffer,"%x",tempvalue & 0x7FFF);

			apb.append(buffer);//copy last character of bus 
			apb.append(space);
		}
		else if(pin_size==16)
		{
			tempvalue = 0xABCABCAB;//0x55555555;;
			sprintf(buffer,"%x",tempvalue & 0xFFFF);

			apb.append(buffer);//copy last character of bus 
			apb.append(space);
		}
		else if(pin_size==17)
		{
			tempvalue = 0;//0x55555555;;
			sprintf(buffer,"%x",tempvalue & 0x1FFFF);

			apb.append(buffer);//copy last character of bus 
			apb.append(space);
		}
		else if(pin_size==18)
		{
			tempvalue = 0;//0x55555555;;
			sprintf(buffer,"%x",tempvalue & 0x3FFFF);

			apb.append(buffer);//copy last character of bus 
			apb.append(space);
		}
		else if(pin_size==19)
		{
			tempvalue = 0;//0x55555555;;
			sprintf(buffer,"%x",tempvalue & 0x7FFFF);

			apb.append(buffer);//copy last character of bus 
			apb.append(space);
		}
		else if(pin_size==20)
		{
			tempvalue = 0;//0x55555555;;
			sprintf(buffer,"%x",tempvalue & 0xFFFFF);

			apb.append(buffer);//copy last character of bus 
			apb.append(space);
		}
		else if(pin_size==21)
		{
			tempvalue = 0;//0x55555555;;
			sprintf(buffer,"%x",tempvalue & 0x1FFFFF);

			apb.append(buffer);//copy last character of bus 
			apb.append(space);
		}
		else if(pin_size==22)
		{
			tempvalue = 0;//0x55555555;;
			sprintf(buffer,"%x",tempvalue & 0x3FFFFF);

			apb.append(buffer);//copy last character of bus 
			apb.append(space);
		}
		else if(pin_size==23)
		{
			tempvalue = 0;//0x55555555;;
			sprintf(buffer,"%x",tempvalue & 0x7FFFFF);

			apb.append(buffer);//copy last character of bus 
			apb.append(space);
		}
		else if(pin_size==24)
		{
			tempvalue = 0;//0x55555555;;
			sprintf(buffer,"%x",tempvalue & 0xFFFFFF);

			apb.append(buffer);//copy last character of bus 
			apb.append(space);
		}
		else if(pin_size==25)
		{
			tempvalue = 0;//0x55555555;;
			sprintf(buffer,"%x",tempvalue & 0x1FFFFFF);

			apb.append(buffer);//copy last character of bus 
			apb.append(space);
		}
		else if(pin_size==26)
		{
			tempvalue = 0;//0x55555555;;
			sprintf(buffer,"%x",tempvalue & 0x3FFFFFF);

			apb.append(buffer);//copy last character of bus 
			apb.append(space);
		}
		else if(pin_size==27)
		{
			tempvalue = 0;//0x55555555;;
			sprintf(buffer,"%x",tempvalue & 0x7FFFFFF);

			apb.append(buffer);//copy last character of bus 
			apb.append(space);
		}
		else if(pin_size==28)
		{
			tempvalue = 0;//0x55555555;;
			sprintf(buffer,"%x",tempvalue & 0xFFFFFFF);

			apb.append(buffer);//copy last character of bus 
			apb.append(space);
		}
		else if(pin_size==29)
		{
			tempvalue = 0;//0x55555555;;
			sprintf(buffer,"%x",tempvalue & 0x1FFFFFFF);

			apb.append(buffer);//copy last character of bus 
			apb.append(space);
		}
		else if(pin_size==30)
		{
			tempvalue = 0;//0x55555555;;
			sprintf(buffer,"%x",tempvalue & 0x3FFFFFFF);

			apb.append(buffer);//copy last character of bus 
			apb.append(space);
		}
		else if(pin_size==31)
		{
			tempvalue = 0;//0x55555555;;
			sprintf(buffer,"%x",tempvalue & 0x7FFFFFFF);

			apb.append(buffer);//copy last character of bus 
			apb.append(space);
		}
		else if(pin_size==32)
		{
			tempvalue = 0;//0x55555555;;
			sprintf(buffer,"%x",tempvalue & 0xFFFFFFFF);

			apb.append(buffer);//copy last character of bus 
			apb.append(space);
		}
	}
	std::transform(apb.begin(), apb.end(), apb.begin(), ::toupper);

	return apb;	
}

unsigned int UpdateCurrentValue(unsigned OrgNumber, unsigned NewNumber, unsigned int endIndex, unsigned int startIndex)

{
	int i, shift_y, ybit;
	long int temp, t;
	if(endIndex == -1) endIndex = 0;// to care of single wire

	if(startIndex == -1) startIndex =0;

	for (i = endIndex;i <= startIndex;i++)
	{
		shift_y = NewNumber >> i;
		ybit = shift_y & 1;
		if (ybit == 1)
		{
			temp = 1 << i;
			OrgNumber = OrgNumber | temp;
		}
		if (ybit == 0)
		{
			t = 0XFFFFFFFF;
			temp = 1 << i;
			endIndex = t ^ temp;
			OrgNumber = OrgNumber & endIndex;
		}
	}
	return OrgNumber;
}
// return 0 if Component not found. Likely to be an input pin , else return 1; //bonus1
unsigned  CheckComponentExecStatus(struct componentstruct cd,unsigned pinindex, unsigned wire_no, const std::vector<struct netstruct> & net,
							  std::vector<struct netlist>&nl,unsigned *nlIndex,unsigned *clIndex)
{
	int found = 0,flag = 0;
	unsigned k,j;
	int tt1 = nl.size();
	struct netstruct comparenet; 
	int tempint1 = 0,tempint2;
	//std::cout<<"nlIndex at beginning = "<<*nlIndex<<std::endl;
	//std::cout<<"clIndex at beginning = "<<*clIndex<<std::endl;

	for(k = 0; k< nl.size(); k++)//find input pin value in currentValue vector
	{
		if((nl[k].wireName.compare(cd.pinname[pinindex]) == 0)&&(net[k].wireindex == wire_no))
		{

			//std::cout<<"wire_no = "<<wire_no<<std::endl;
			//std::cout<<"cd.pinname[pinindex] = "<<cd.pinname[pinindex]<<std::endl;

			// after matching wireName, match wireIndex 
			tempint1 = net[k].wiresize;
			tempint2 = net[k].wireindex;
			if(tempint1 == 1){
				if(tempint2 == cd.busstartindex[pinindex]) 
					flag =1;
			}
			else if (tempint1 > 1){
				if((tempint2 <= cd.busstartindex[pinindex]) && (tempint2 >= cd.busendindex[pinindex])) flag =1;
			}

			if(flag)
			{
				for(j = 0; j< nl[k].componentList.size(); j++)//find input pin value in currentValue vector
				{
					if(nl[k].componentList[j].componentType.compare("evl_input")== 0)// check if the pin was directly loaded by evl_input; one of its components should be evl_input
					{
						found = 1;
						break;
					}
					if(nl[k].componentList[j].pinLocation == 0)
					{
						if(nl[k].componentList[j].componentType.compare("evl_output")== 0)
							continue;
						else
							found = 1;
						break;
					}
				}
				if(found) break;
			}
		}
	}
	//std::cout<<"nlIndex at end = "<<*nlIndex<<std::endl;
	//std::cout<<"clIndex at end = "<<*clIndex<<std::endl;

	if (found)
	{
		*nlIndex = k; *clIndex = j;
		return 1;
	}
	else return 0;
}

unsigned  CheckModuleExecStatus(struct componentstruct cd,unsigned pinindex, unsigned wire_no, const std::vector<struct netstruct> & net,
							  std::vector<struct netlist>&nl,unsigned *nlIndex,unsigned *clIndex)
{
	int found = 0,flag = 0;
	unsigned i,k,j;
	int tt1 = nl.size();
	struct netstruct comparenet;
	int tempint1 = 0,tempint2;
	//std::cout<<"nlIndex at beginning = "<<*nlIndex<<std::endl;
	//std::cout<<"clIndex at beginning = "<<*clIndex<<std::endl;

	for(k = 0; k< nl.size(); k++)//find input pin value in currentValue vector
	{
		if((nl[k].wireName.compare(cd.pinname[pinindex]) == 0)&&(net[k].wireindex == wire_no))
		{

			//std::cout<<"wire_no = "<<wire_no<<std::endl;
			//std::cout<<"cd.pinname[pinindex] = "<<cd.pinname[pinindex]<<std::endl;

			// after matching wireName, match wireIndex
			tempint1 = net[k].wiresize;
			tempint2 = net[k].wireindex;
			if(tempint1 == 1){
				if(tempint2 == cd.busstartindex[pinindex])
					flag =1;
			}
			else if (tempint1 > 1){
				if((tempint2 <= cd.busstartindex[pinindex]) && (tempint2 >= cd.busendindex[pinindex])) flag =1;
			}

			if(flag)
			{
				for(j = 0; j< nl[k].componentList.size(); j++)//find input pin value in currentValue vector
				{
					if(nl[k].componentList[j].pinLocation == 0)
					{
						if(nl[k].componentList[j].componentType.compare("evl_output")== 0)
							continue;
						else
							found = 1;
						break;
					}
					if(nl[k].componentList[j].componentType.compare("evl_input")== 0)// check if the pin was directly loaded by evl_input; one of its components should be evl_input
					{
						found = 1;
						break;
					}
					//std::cout<<"nl[k].componentList[j].componentName = "<<nl[k].componentList[j].componentName<<std::endl;
					//std::cout<<"nl[k].componentList[j].componentType = "<<nl[k].componentList[j].componentType<<std::endl;
// add for loop
					for(i = 1; i < moduledetails.size(); i++)
					{

						if(nl[k].componentList[j].componentType.compare(moduledetails[i].modulename) == 0)
						{
							if(moduledetails[i].moduleparameters[nl[k].componentList[j].pinLocation].type == 1)// it is a output pin
							{
								found = 1;
								break;
							}
						}
					}
					if(found) break;
				}
				if(found) break;
			}
		}
	}
	//std::cout<<"nlIndex at end = "<<*nlIndex<<std::endl;
	//std::cout<<"clIndex at end = "<<*clIndex<<std::endl;

	if (found)
	{
		*nlIndex = k; *clIndex = j;
		return 1;
	}
	else return 0;
}
