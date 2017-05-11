#ifndef _FUNCPROTO_H_
#define _FUNCPROTO_H_

void WriteToFile(std::ofstream &netlist_file);
int NetAdd(struct netstruct tempnet, std::vector<struct netstruct> &net);
template<typename T> int SearchNet(const std::vector<T> vec, const T& key);
template<typename T> int SearchString(const std::vector<T> vec, const T& key);
int CheckComponent(struct componentstruct cs);
void CheckComponentPinValidity(std::vector<struct componentstruct> & cd,std::vector<struct netstruct> & net);
void Simulate(std::string argv,std::vector<struct componentstruct> & cd,unsigned cd_index,const std::vector<struct netstruct> & net, std::vector<struct netlist>&nl,int TransId,int flag);
std::string  CreatePinValueString(std::vector<struct componentstruct> & cd, unsigned int index);
std::string CreateInputFile(std::vector<struct componentstruct> & cd,int index,int TransId);
unsigned UpdateCurrentValue(unsigned OrgNumber, unsigned NewNumber, unsigned int endIndex, unsigned int startIndex);
unsigned  CheckComponentExecStatus(struct componentstruct cd,unsigned pinindex,unsigned wire_no,
		const std::vector<struct netstruct> & net,std::vector<struct netlist>&nl,unsigned *nlIndex,unsigned *clIndex);
unsigned  CheckModuleExecStatus(struct componentstruct cd,unsigned pinindex, unsigned wire_no, const std::vector<struct netstruct> & net,
							  std::vector<struct netlist>&nl,unsigned *nlIndex,unsigned *clIndex);
int Lex(std::ifstream &input_file,std::vector<std::string>&tokenvector);
void CreatenetList(std::ofstream &netlist_file);
void Parse(std::vector<std::string>&tokenvector);
void TokenizeAssign(std::vector<std::string> &tokenvector, unsigned *index, std::vector<struct componentstruct> &componentdetails,std::vector<struct netstruct> &net);
int TokenizeModule(std::vector<std::string> &tokenvector, unsigned *index, modulestruct *moduledetails);
int ComputeTrisValue();
int TrisValueAdd(struct trisvalue tempnet, std::vector<struct trisvalue> &net);


#endif /* _FUNCPROTO_H_ */
