#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#include <functional>
#include <cctype>

#include <string.h>

#define ERROR -1
#define INIT 0
#define MODULE 1
#define ENDMODULE 2
#define WIRE 3
#define COMPONENT 4
#define LPAREN 5

struct modulestruct {
	std::string modulename;
	std::vector<struct wirestruct> modulewires;
	std::vector<struct componentstruct> componentwires;
};

struct wirestruct {
	std::string wirename;
	int wiresize;
};

struct componentstruct {
	std::string componentname;
	std::vector<std::string> pinname;
	std::vector<int> busstartindex;
	std::vector<int> busendindex;
	int componentsize;
};

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
	std::string buscomp;
	std::vector<std::string> modulenamevector;

	std::vector<struct modulestruct> moduledetails;
	std::vector<struct wirestruct> wiredetails;
	std::vector<struct componentstruct> componentdetails;

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
					struct wirestruct tempwire;
					tempwire.wirename.assign(tokenvector[i]);
					if(wiresize==0)//no [m:n] at beginning of wire token
						tempwire.wiresize = 1;
					else tempwire.wiresize = wiresize;
					wiredetails.push_back(tempwire);
					if(tokenvector[i].compare("[")==0){
						wiredetails.pop_back();// pop vector cause it now has name=[ and size=0 in it
						wireupper = atoi(tokenvector[i+1].c_str());
						wirelower = atoi(tokenvector[i+3].c_str());
						wiresize = wireupper- wirelower+1;//wire size = upperlimit-lowerlimit+1
						if(wiresize<1)
							std::cout<<"Error in bus range."<<std::endl;
						tempwire.wiresize = wiresize;
						i = i+5;//skip [m:n] = 5 tokens
						tempwire.wirename = tokenvector[i];//get name token
						wiredetails.push_back(tempwire);
					}
					i++;
				}
			}//end of while
			wiresize = 0;
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
					if(state==LPAREN){
						tempcomp.pinname.push_back(tokenvector[i]);//any name tokens after first will be names of pins
						buscomp.assign(tokenvector[i]);
						tempcomp.busstartindex.push_back(-1);//default value of index = -1
						tempcomp.busendindex.push_back(-1);//default value of index = -1
						componentsize++;
						i++;//get opening paren [ or next token
						if(tokenvector[i].compare("[")==0){
							if((tokenvector[i+2].compare("]")==0)){//single bus element is used
								tempcomp.busstartindex.pop_back();//token is a bus, and will have valid index
								i++;//get array index; discard the opening [
								tempcomp.busstartindex.push_back(atoi(tokenvector[i].c_str()));//save index
								i++;//get closing paren ]
								i++;//go to next token
							}
							if((tokenvector[i+2].compare(":")==0)){//range of bus element is used [m:n]
								tempcomp.busstartindex.pop_back();//token is a bus, and will have both indices
								tempcomp.busendindex.pop_back();//token is a bus, and will have both indices
								buscomp.append(tokenvector[i].c_str(),1); //store[
								i++;//get array starting index;
								tempcomp.busstartindex.push_back(atoi(tokenvector[i].c_str()));//save starting index
								i++; //get colon :
								i++;//get array ending index;
								tempcomp.busendindex.push_back(atoi(tokenvector[i].c_str()));//save index
								i++;//get closing paren ]
								i++;//go to next token
							}
						}
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
		}
	}
	return 0;
}
