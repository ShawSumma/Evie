#ifndef _PREPROSESSOR_H_
#define _PREPROSESSOR_H_

#include "../Flags.h"
#include "../Lexer/Component.h"
#include "../Docker/Docker.h"
#include "../UI/Usr.h"

#include <vector>

using namespace std;

class PreProsessor
{
public:
	PreProsessor(vector<Component> &list) : Input(list){}
	~PreProsessor(){}
	void Factory();
	void Include(int i);
	void If(int i);
	bool Calculate(vector<Component>);
	void Define_Const_Value(int i);
	void Replace_Const_Name_With_Value(vector<Component> &in);

	string Update_Working_Dir(string file_and_dir);
	vector<string> Included_Files;
	//std::map<string::const_name, Component::value>
	map<string, Component> Defined_Constants;
	vector<Component>& Input;
	string Working_Dir = "";
private:
};

#endif