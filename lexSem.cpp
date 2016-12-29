#include <iostream>
#include <fstream>
#include <string>
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
#define LPAREN 5
#define BUSPIN 6

struct modulestruct {
std::string modulename;
std::vector<struct wirestruct> modulewires;
std::vector<struct componentstruct> componentwires;
};

struct wirestruct {
std::string wirename;
int wiresize;
};

struct netstruct { //sorted wires
std::string wirename;
int wireindex;
};

struct componentstruct {
std::string componentname;
std::vector<std::string> pinname;
std::vector<int> busstartindex;
std::vector<int> busendindex;
int componentsize;
};

int binarysearch(const std::vector<struct netstruct>& vec, int start, int end, const struct netstruct & key)
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
        return binarysearch(vec, start, middle - 1, key);
    }
    // else if (vec[middle]wirename.compare(key.wirename) < 0)
    return binarysearch(vec, middle + 1, end, key);
}

//! \brief A helper function for the binary search
template<typename T>
int search(const std::vector<T>& vec, const T& key)
{
    return binarysearch(vec, 0, vec.size()-1, key);
}

int netlistadd(struct netstruct tempnet, std::vector<struct netstruct> &net){
int idx = 0;
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
	while((idx < net.size())&&(tempnet.wireindex > net[idx].wireindex)){//insert after other elements of bus
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

int main(int argc, char *argv[])
{
if (argc < 2)
{
	std::cerr << "You should provide a file name." << std::endl;
	return -1;
}

std::ifstream input_file(argv[1]);
if (!input_file)
{
	std::cerr << "I can't read " << argv[1] << "." << std::endl;
	return -1;
}

std::string output_file_name = std::string(argv[1])+".syntax";
std::ofstream output_file(output_file_name.c_str());
if (!output_file)
{
	std::cerr << "I can't write " << argv[1] << ".syntax ." << std::endl;
	return -1;
}

std::string line;
int state = INIT;//initial state
std::vector<std::string> tokenvector;
for (int line_no = 1; std::getline(input_file, line); ++line_no)
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
				|| (line[i] == ','))
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

int vecsize = tokenvector.size();
int wiresize = 0;
int wireupper = 0;
int wirelower = 0;
int pinsize = 0;
int componentsize = 0;
int comparelower = 0;
int compareupper = 0;
int wireexists = -1;
std::string buscomp;
std::vector<std::string> modulenamevector;

std::vector<struct modulestruct> moduledetails;
std::vector<struct wirestruct> wiredetails;
std::vector<struct componentstruct> componentdetails;
std::vector<struct netstruct> net;

struct wirestruct tempwire;
struct netstruct tempnet, comparenet;
//struct mycomparator comparefunc;


for(int i=0;i<vecsize; i++){ //operations on tokenvector[i]
	std::transform(tokenvector[i].begin(),
			tokenvector[i].end(),
			tokenvector[i].begin(),
			std::ptr_fun<int,
			int>(std::tolower));
	if(tokenvector[i].compare("module")==0){ //if state is MODULE, next state should be module name, next should be semicolon
		state = MODULE;
		i++;
		modulenamevector.push_back(tokenvector[i]); //store module name
		i++;
	}
	else if(tokenvector[i].compare("wire")==0){
		state = WIRE;
		i++; //remove token name = WIRE and store only the wire names into wirenamevector
		while(tokenvector[i].compare(";")!=0){ //till end of Wire list; bus[2:0]
			if(tokenvector[i].compare(",")==0){
				i++;
			}
			else{
				tempwire.wirename.assign(tokenvector[i]);
				if(wiresize==0)//no [m:n] at beginning of wire token
					tempwire.wiresize = 1;
				else tempwire.wiresize = wiresize;

				wiredetails.push_back(tempwire);
				if(tokenvector[i].compare("[")==0){
					wiredetails.pop_back();// pop vector cause it now has name=[ and size=0 in it
					wireupper = atoi(tokenvector[i+1].c_str());
					wirelower = atoi(tokenvector[i+3].c_str());
					wiresize = wireupper - wirelower+1;//wire size = upperlimit-lowerlimit+1
					if(wiresize<1)
						std::cout<<"Error in bus range."<<std::endl;
					tempwire.wiresize = wiresize;
					i = i+5;//skip [m:n] = 5 tokens
					tempwire.wirename = tokenvector[i];//get name token
					wiredetails.push_back(tempwire);
					for(int wireindex=wirelower; wireindex<(wireupper+1); wireindex++){
						tempnet.wirename.assign(tokenvector[i]);
						tempnet.wireindex = wireindex;
						std::cout<<tempnet.wirename<<"["<<tempnet.wireindex<<"]"<<std::endl;
						netlistadd(tempnet, net);
					}
				}
				else{//single wire
					wireupper = 0;
					wirelower = 0;
					tempnet.wirename.assign(tokenvector[i]);
					tempnet.wireindex = -1;
					std::cout<<tempnet.wirename<<"["<<tempnet.wireindex<<"]"<<std::endl;
					netlistadd(tempnet, net);
				}
				i++;
			}
		}//end of while
		wiresize = 0;
		wireupper = 0;
		wirelower = 0;
	}//end of else if for WIRE
	else if(tokenvector[i].compare("endmodule")==0){
		//if current state is ENDMODULE, next state should be EOF
		state = ENDMODULE;
	}
	else {
		state = COMPONENT;
		struct componentstruct tempcomp;
		if(tokenvector[i].compare("(")==0)
			tempcomp.componentname.assign("");//null if no component name is given
		else
			tempcomp.componentname.assign(tokenvector[i]);
		i++;//first token will be name of component
		while(tokenvector[i].compare(";")!=0){
			if((tokenvector[i].compare("(")==0)
					||(tokenvector[i].compare(")")==0)
					||(tokenvector[i].compare(",")==0)){
				if(tokenvector[i].compare("(")==0)
					state = LPAREN;
				i++;//skip parentheses and commas
			}
			else{
				if(state==LPAREN){//start storing component pins
					tempcomp.pinname.push_back(tokenvector[i]);//any name tokens after first will be names of pins
					buscomp.assign(tokenvector[i]);
					comparenet.wirename.assign(tokenvector[i]);
					std::cout<<comparenet.wirename<<std::endl;
					tempcomp.busstartindex.push_back(-1);//default value of index = -1
					tempcomp.busendindex.push_back(-1);//default value of index = -1
					comparenet.wireindex = -1;
					componentsize++;
					i++;//get opening paren [ or next token
					if(tokenvector[i].compare("[")==0){
						if((tokenvector[i+2].compare("]")==0)){//single bus element is used
							tempcomp.busstartindex.pop_back();//token is a bus, and will have valid index
							i++;//get array index; discard the opening [
							tempcomp.busstartindex.push_back(atoi(tokenvector[i].c_str()));//save index
							comparenet.wireindex = atoi(tokenvector[i].c_str());
							i++;//get closing paren ]
							i++;//go to next token
						//	std::cout<<"checking wire"<<std::endl; //if single element of bus is being used eg s0[2]
							wireexists = search(net, comparenet);
							if(wireexists != -1){//wire is found
								std::cout<<"bus wire exists!"<<std::endl;
							}
							else std::cout<<"bus wire doesn't exist"<<std::endl;
						}
						else if((tokenvector[i+2].compare(":")==0)){//range of bus element is used [m:n]
							tempcomp.busstartindex.pop_back();//token is a bus, and will have both indices
							tempcomp.busendindex.pop_back();//token is a bus, and will have both indices
							buscomp.append(tokenvector[i].c_str(),1); //store[
							i++;//get array starting index;
							tempcomp.busstartindex.push_back(atoi(tokenvector[i].c_str()));//save starting index
							compareupper = atoi(tokenvector[i].c_str());
							i++; //get colon :
							i++;//get array ending index;
							tempcomp.busendindex.push_back(atoi(tokenvector[i].c_str()));//save index
							comparelower = atoi(tokenvector[i].c_str());
							i++;//get closing paren ]
							i++;//go to next token
					//		std::cout<<"comparelowerlimit = "<<comparelower<<" compareupperlimit = "<<compareupper<<std::endl;
							for(int compareidx = comparelower; compareidx<=compareupper; compareidx++){
								comparenet.wireindex = compareidx;
								std::cout<<"wirename = "<<comparenet.wirename<<" wireindex = "<<comparenet.wireindex<<std::endl;
								wireexists = search(net, comparenet);
								if(wireexists != -1){//wire is found
						//			std::cout<<"bus exists!"<<std::endl;
									state = BUSPIN;
								}
								else{
						//			std::cout<<"bus doesn't exist"<<std::endl;
									state = ERROR;
									break;
								}
							}
							if(state == ERROR){ //if a bus wire was found to not exist
								std::cout<<"ERROR in bus"<<std::endl;
							}
							else if(state == BUSPIN){
								std::cout<<"Bus exists"<<std::endl;
							}
						}
					}//end of LPAREN if
					else{//single wire ie wireindex = -1
						wireexists = search(net, comparenet);
						if(wireexists != -1){//wire is found
							std::cout<<wireexists<<std::endl;
							std::cout<<"single wire exists!"<<std::endl;
						}
						else std::cout<<"single wire doesn't exist"<<std::endl;
					}
			/*		bool wireexists = binary_search(net.begin(), net.end(), comparenet, comparefunc);
					if(wireexists){//true
						std::cout<<"wire exists!"<<std::endl;
					}
					else std::cout<<"wire doesn't exist"<<std::endl;*/
				}//end of if
				else{
					tempcomp.componentname.append(" ");
					tempcomp.componentname.append(tokenvector[i].c_str(),tokenvector[i].size());
					//if token is not within parentheses, then its part of component name and not an input
					i++;
				}//end of else
			}//end of else
		}
		tempcomp.componentsize = componentsize;
		componentdetails.push_back(tempcomp);
		componentsize = 0;//reset size to be used for next component
	}
}

for(int mvs = 0; mvs<modulenamevector.size();mvs++){ //for every module in the code
	output_file<< "module" << " "<< modulenamevector[mvs] << std::endl;
	output_file<<"wires "<<wiredetails.size()<<std::endl;
	for(int n = 0; n< wiredetails.size();n++){
		output_file<<"\t wire "<< wiredetails[n].wirename<<" "
				<<wiredetails[n].wiresize <<std::endl;
	}
	output_file<<"components "<<componentdetails.size()<<std::endl;
	for(int n = 0; n< componentdetails.size();n++){
		//if componentname="", code still prints two spaces between
		//"component" and componentsize expecting a component name
		output_file<<"\t component "<< componentdetails[n].componentname<<" "
				<<componentdetails[n].componentsize<<std::endl;
		pinsize = componentdetails[n].componentsize;//get index
		int temp = 0;
		while((temp<pinsize)){ //while there are pins in the component
			if(componentdetails[n].busstartindex[temp]!=-1){
				//input is a range of a bus, print both start and end index
				if(componentdetails[n].busendindex[temp]!=-1){
					//input is a range of a bus, print start and end indices
					output_file<<"\t \t pin "<< componentdetails[n].pinname[temp]<<" "
							<<componentdetails[n].busstartindex[temp]<<" "
							<<componentdetails[n].busendindex[temp]<<std::endl;
				}
				else {//input is a single element of a bus, print startindex
					output_file<<"\t \t pin "<< componentdetails[n].pinname[temp]<<" "
							<<componentdetails[n].busstartindex[temp]<<std::endl;
				}

			}
			else //not a bus, don't print busstartindex or busendindex
				output_file<<"\t \t pin "<<componentdetails[n].pinname[temp]<<std::endl;
			temp++;
		}

	}//end of component for
	output_file<<"Netlist: "<<net.size()<<std::endl;
	int temp2=0;
	while(temp2<net.size()){
		output_file<<"net: "<< net[temp2].wirename<<" "<<net[temp2].wireindex<<std::endl;
		temp2++;
	}
}
return 0;
}
