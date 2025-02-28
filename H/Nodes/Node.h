#ifndef _NODE_H_
#define _NODE_H_

extern int _SYSTEM_BIT_SIZE_;

#include <string>
#include <vector>
#include <iostream>
#include <regex>

#include "../Lexer/Lexer.h"
#include "../Lexer/Position.h"

using namespace std;
class Node;

extern Node* Global_Scope;

class Variable_Descriptor {
public:
	int Define_Index = 0;
	int Expiring_Index = 0;
	Node* Var = nullptr;
	Variable_Descriptor(Node* v, int i, vector<Node*> source);
};

namespace PARSED_BY {
	constexpr long long NONE							= 1 << 0;
	constexpr long long PREPROSESSOR					= 1 << 1;
	constexpr long long PARSER							= 1 << 2;
	constexpr long long POSTPROSESSOR					= 1 << 3;
	constexpr long long ALGEBRA							= 1 << 4;
	constexpr long long SAFE							= 1 << 5;
	constexpr long long IRGENERATOR						= 1 << 6;
	constexpr long long IRPOSTPROSESSOR					= 1 << 7;
	constexpr long long BACKEND							= 1 << 8;

	constexpr long long DESTRUCTOR_CALLER				= 1 << 9;
	constexpr long long REFERENCE_COUNT_INCREASE		= 1 << 10;
	constexpr long long MEMBER_FUNCTION_DEFINED_INSIDE	= 1 << 11;
	constexpr long long FUNCTION_PROSESSOR				= 1 << 12;
	constexpr long long THIS_AND_DOT_INSERTER			= 1 << 13;

}

enum class LABEL_TYPE {
	NON,
	RETURN_LABEL,
	CAN_MODIFY_ID,
};

class Node {
public:
	Node(int flag, Position* p) : Type(flag), Location(p) {}
	Node(int flag, Position* p, string f) : Type(flag), Location(p), Format(f) {}
	Node(string n, Position* p) : Name(n), Location(p) {}
	Node(int flag, string n, Position* p) : Type(flag), Name(n), Location(p) {}
	Node(Node* n, int f) { *this = *Copy_Node(n, n->Scope); Type = f; }
	//Node(){}
	//Normal features
	Position* Location = nullptr;
	string Name = "";
	string Comment = "";
	//for string or char lists
	string String = "";
	int Type = 0;
	int Size = 0;
	bool Requires_Address = false;	//for optimisation pusrposes.
	int Memory_Offset = 0;
	vector<string> Inheritted;
	vector<Node*> Templates;
	vector<Node*> Inheritable_templates;
	vector<Component> Template_Children;
	Node* Scope = nullptr;
	//funciton inlining features
	vector<Node*> Header;
	//Scope features
	vector<Node*> Defined;
	vector<Node*> Childs;
	vector<Node*> Member_Functions;
	vector<Node*> Operator_Overloads;
	int Call_Space_Start_Address = 0;
	//namespace inlining features
	vector<Node*> Inlined_Items;
	//for maximus parametrus usagus.
	int Size_of_Call_Space = 0;
	//for local variables.
	int Local_Allocation_Space = 0;
	//function features
	vector<Node*> Parameters;
	string Mangled_Name = "";
	//Import features
	vector<Node*> Numerical_Return_Types;
	//operator features
	Node* Context = nullptr;
	Node* Left = nullptr;
	Node* Right = nullptr;
	//pointter features
	int Scaler = 0;
	//content features
	char Paranthesis_Type = 0;
	//condition features
	Node* Succsessor = nullptr;
	Node* Predecessor = nullptr;
	//algebra optimizer features
	int Order = 1;
	int Coefficient = 1;
	Variable_Descriptor* Current_Value = nullptr;
	bool Inlined = false;
	bool Cant_Inline = false;
	//fetching features
	Node* Fetcher = nullptr;
	//calling features
	Node* Function_Implementation = nullptr;
	int Calling_Count = 0;
	bool Function_Ptr = false;
	bool Function_Address_Giver = false;
	//calling convension is stored in the inheritted list
	//function prototype features
	//the import has the flag to prototyping
	//and the paramters are Named as the size needed. thx!
	//Float features
	string Format = "integer";	//integer | decimal
	//Template object features.
	bool Is_Template_Object = false;
	//casting features
	Node* Cast_Type = nullptr;
	//IR safe features
	//bool Generated = false;
	long long Parsed_By = PARSED_BY::NONE;
	LABEL_TYPE Inline_Return_Label = LABEL_TYPE::NON;

	bool is(long long F) {
		return (Parsed_By & F) == F;
	}

	bool is(int F) {
		return Type == F;
	}
	
	int is(string t) {
		for (int i = 0; i < Inheritted.size(); i++)
			if (t == Inheritted[i])
				return i;
		return -1;
	}

	bool is(vector<string> s) {
		bool Is = true;
		for (auto i : s)
			if (is(i) == -1) {
				Is = false;
				break;
			}

		return Is;
	}

	bool is(LABEL_TYPE F) { return (F == Inline_Return_Label); }

	int Calculate_Inheritted_Distance(Node* Val, Node* Loader, string type);

	int Calculate_Inheritted_Distance(Node* Loader, string type) {
		return Calculate_Inheritted_Distance(this, Loader, type);
	}

	bool Inherits_Template_Type() {
		if (Is_Template_Object)
			return true;
		for (auto i : Inheritted) {
			if (Lexer::GetComponent(i).is(::Flags::KEYWORD_COMPONENT))
				continue;
			bool Inheritted_templation = Find(i)->Inherits_Template_Type();
			if (Inheritted_templation) {
				Is_Template_Object = true;	//this speeds the prosess up if this is checked again
				return Inheritted_templation;
			}
		}
		return false;
	}
	
	string Get_Inheritted(string seperator, bool Skip_Prefixes = false, bool Get_Name = false, bool skip_keywords = false);
	
	vector<string> Get_Inheritted(bool Skip_Prefixes = false, bool Get_Name = false, bool Skip_Keywords = false);

	string Get_Mangled_Name(bool Skip_Prefixes = false, bool Skip_Return_Type = false) {
		//_int_ptr_Z6banana_int_int_short
		string mname = "";
		//add the returning type
		if (!Skip_Return_Type)
			for (auto& i : Inheritted)
				mname += "_" + i;
		mname += "_" + Name;
		for (auto i : Parameters)
			mname += "_" + i->Get_Inheritted((string)"_", is(IMPORT), Skip_Prefixes);
		return mname;
	}

	string Un_Mangle(Node* n) {
		string Result;
		for (string s : n->Inheritted)
			Result += s + " ";
		Result += n->Name + "(";
		for (Node* p : n->Parameters)
			for (string s : p->Inheritted)
				Result += s + ", ";
		Result += ")\n";
		return Result;
	}
	
	//returns atm: Cpp, Evie
	string Get_Calling_Convention_Type(string raw) {
		if (raw[0] == '_') {
			if (raw[1] == 'Z')
				return "Cpp";
			if (raw[1] == 'E')
				return "Evie";
		}
		return "UNKNOWN";
	}

	vector<Component> Un_Mangle(string raw) {
		Component Function = Component("", Flags::TEXT_COMPONENT);
		Component Parenthesis = Component("()", Flags::PAREHTHESIS_COMPONENT);
		bool Func_Name = true;
		string Current;
		vector<Component> Current_Parameter_Inheritted;
		//type ptr new  type
		if (raw[0] == '_' && raw[1] == 'Z') {
			//C++ unmangler
			//_Z3NEWi3ABC
			for (int i = 2; i < raw.size(); i++) {
				if		(raw[i] == 'P') {
					Component ptr = Component("ptr", Flags::KEYWORD_COMPONENT);
					Current_Parameter_Inheritted.push_back(ptr);
					continue;
				}
				else if (raw[i] == 'R') {
					Component ref = Component("ref", Flags::KEYWORD_COMPONENT);
					Current_Parameter_Inheritted.push_back(ref);
					continue;
				}
				else if (raw[i] == 'c') {
					//because there is nothign defined yet we want to preserve these datas for later definition.
					Component p = Component("1", Flags::NUMBER_COMPONENT);
					p.Components = Current_Parameter_Inheritted;
					Current_Parameter_Inheritted.clear();
					Parenthesis.Components.push_back(p);
				}
				else if (raw[i] == 's') {
					Component p = Component("2", Flags::NUMBER_COMPONENT);
					p.Components = Current_Parameter_Inheritted;
					Current_Parameter_Inheritted.clear();
					Parenthesis.Components.push_back(p);
				}
				else if (raw[i] == 'f') {
					Component p = Component("4", Flags::NUMBER_COMPONENT);
					p.Components = Current_Parameter_Inheritted;
					Current_Parameter_Inheritted.clear();
					Parenthesis.Components.push_back(p);
				}
				else if (raw[i] == 'i') {
					Component p = Component("4", Flags::NUMBER_COMPONENT);
					p.Components = Current_Parameter_Inheritted;
					Current_Parameter_Inheritted.clear();
					Parenthesis.Components.push_back(p);
				}
				else if (raw[i] == 'd') {
					Component p = Component("8", Flags::NUMBER_COMPONENT);
					p.Components = Current_Parameter_Inheritted;
					Current_Parameter_Inheritted.clear();
					Parenthesis.Components.push_back(p);
				}

				else if (((raw[i] >= 48) && (raw[i] <= 57))) {
					string tmp = "";
					tmp += raw[i];
					for (int j = i + 1; j < raw.size(); j++) {
						if (((raw[j] >= 48) && (raw[j] <= 57)))
							tmp += (char)raw[j];
						else
							break;
					}
					int size = atoi(tmp.c_str());
					string name = "";
					for (int j = i + (int)tmp.size(); (j < (size + i + 1)) && j < (int)raw.size(); j++) {
						name += (char)raw[j];
					}
					if (Func_Name) {
						Function.Value = name;
						Func_Name = false;
					}
					else {
						//class based parameters.
						Component p = Component(name, Flags::TEXT_COMPONENT);
						p.Components = Current_Parameter_Inheritted;
						Current_Parameter_Inheritted.clear();
						Parenthesis.Components.push_back(p);
					}
					i += size;
				}
			}
		}
		//else if (raw[0] == '_' && raw[1] == 'E') {

		//}
		else {
			//this lauches when no call type is identifyed.
			Function.Value = raw;
		}
		vector<Component> Result = { Function, Parenthesis };
		return Result;
	}
	
	Node* Find(string name, Node* parent, bool Need_Parent_existance = true);
	
	Node* Find(string name, Node* parent, int flags);

	Node* Find(string name, Node* parent, vector<int> flags) {
		for (auto flag : flags)
			if (Find(name, parent, flag))
				return Find(name, parent, flag);
		return nullptr;
	}
	
	Node* Find(int size, Node* parent, string f) {

		for (Node* i : parent->Defined)
			if (i->Size == size)
				if (i->Format == f)
					return i;

		for (Node* i : parent->Inlined_Items)
			if (i->Size == size)
				if (i->Format == f)
					return i;

		if (parent->Scope != nullptr)
			return Find(size, parent->Scope, f);
		return nullptr;
	}	

	Node* Find(int size, Node* parent, int flags, string f) {

		for (Node* i : parent->Defined)
			if (i->is(flags) && (i->Size == size))
				if (i->Format == f)
					return i;

		for (Node* i : parent->Inlined_Items)
			if (i->is(flags) && (i->Size == size))
				if (i->Format == f)
					return i;

		if (parent->Scope != nullptr)
			return Find(size, parent->Scope, flags, f);
		return nullptr;
	}

	Node* Find(Node* n, Node* p);

	Node* Find(Node* n, Node* p, int f);

	Node* Find(Node* n, Node* p, vector<int> f) {
		for (auto flag : f)
			if (Find(n, p, flag))
				return Find(n, p, flag);
		return nullptr;
	}

	Node* Find(string n) {
		return Find(n, this);
	}

	Node* Find_Scope(Node* n);

	Node* Find(Position& location);

	bool Compare_Fetchers(Node* other);

	vector<Node*> Get_All_Fetchers();

	Node* Get_Scope_As(int F, Node* Parent);

	Node* Get_Scope_As(int F, vector<string> Inhritted, Node* Parent);

	Node* Get_Context_As(int F, Node* Context);

	Node* Get_Context_As(string n, Node* Context);

	vector<Node*> Get_Scope_Path();

	Node* Get_Right_Parent() {
		if (Fetcher != nullptr) {
			return Get_Final_Fetcher(this , 1);
		}
		else {
			return Scope;
		}
	}

	vector<string> Tree;
	//a.x.b.y
	Node* Get_Final_Fetcher(Node* n, int offset) {
		Tree.push_back(n->Name);
		if (n->Fetcher != nullptr) {
			return Get_Final_Fetcher(n->Fetcher, offset);
		}

		//now got though the tree and find the right defined in the last that is inside of node* n.
		reverse(Tree.begin(), Tree.end());
		//a.x.b.y
		Node* Result = Find(Tree[0], n->Scope);

		for (int i = 1; i < Tree.size() - offset; i++) {
			Result = Find(Tree[i], Result);
		}
		Tree.clear();
		return Result;
	}

	Node* Get_Most_Left(Node* n) {
		if (n->Has({ OPERATOR_NODE, ASSIGN_OPERATOR_NODE, BIT_OPERATOR_NODE, CONDITION_OPERATOR_NODE, ARRAY_NODE }))
			return Get_Most_Left(n->Left);
		return n;
	}	
	Node* Get_Most_Left() {
		if (this->Has({ OPERATOR_NODE, ASSIGN_OPERATOR_NODE, BIT_OPERATOR_NODE, CONDITION_OPERATOR_NODE, ARRAY_NODE }))
			return this->Left->Get_Most_Left();
		return this;
	}

	Node* Get_Most_Right(Node* n) {
		if (n->Has({ OPERATOR_NODE, ASSIGN_OPERATOR_NODE, BIT_OPERATOR_NODE, CONDITION_OPERATOR_NODE, ARRAY_NODE }))
			return Get_Most_Right(n->Right);
		return n;
	}
	Node* Get_Most_Right() {
		if (this->Has({ OPERATOR_NODE, ASSIGN_OPERATOR_NODE, BIT_OPERATOR_NODE, CONDITION_OPERATOR_NODE, ARRAY_NODE }))
			return this->Right->Get_Most_Right();
		return this;
	}

	bool Locate(string name, vector<Node*> list) {
		for (Node* i : list)
			if (i->Name == name)
				return true;
		return false;
	}
	
	void Get_Inheritted_Class_Members();
	
	void Get_Inheritted_Class_Members(string s) {
			//if (s == ".")
			//	return;
			Node* inheritted = Find(s, Scope);
			for (auto i : inheritted->Defined) {
				//now insert the inheritted classes members
				if (Locate(i->Name, Defined) != true)
					//if this is already defined no luck trying to re defining the same variable twice :D
					Defined.push_back(i);
			}
	}
	
	//this reqiers that the other local variables inside this object are already having theyre own size!
	int Update_Size();

	/*void Update_Size_By_Inheritted() {
		if (Name == "size" && (is("const") != -1))
			return;
		if (is(NUMBER_NODE))
			return;
		Size = 0;
		for (string s : Inheritted) {
			//there is no inheritable type that doesnt have enything init.
			if (Lexer::GetComponent(s).is(Flags::KEYWORD_COMPONENT)) {
				if (s == "ptr") {
					//this is for function pointters.
					//Size = _SYSTEM_BIT_SIZE_;
					Scaler = Size;
					Size = _SYSTEM_BIT_SIZE_;
				}
				continue;
			}
			Size += Find(s, Scope, true)->Size;
		}
	}

	void Update_Members_Size() {
		if (Name == "size" && (is("const") != -1))
			return;
		Size = 0;
		//this needs maybe revamping?
		//decide between this forloop and inheritting the members that we inherit
		for (string s : Inheritted) {
			//there is no inheritable type that doesnt have enything init.
			if (Lexer::GetComponent(s).is(Flags::KEYWORD_COMPONENT)) {
				if (s == "func" || s == "ptr")
					//this is for function pointters.
					Size += _SYSTEM_BIT_SIZE_;
				continue;
			}
			if (Find(s, Scope)->Defined[0]->Name == "size" && (Find(s, Scope)->Defined[0]->is("const") != -1)) {
				//this is a preprossed size, take it!
				Node* Inherit = Find(s, Scope, true);
				Inherit->Update_Members_Size();
				Size += Inherit->Size;
			}
				//if this happends we this class will inherit the members of the inheritted.
			else
				//there we handle more complex inheritance instances.
				Get_Inheritted_Class_Members(s);
		}
		for (Node* i : Defined) {
			if (i->is(FUNCTION_NODE))
				continue;
			//now revaluate the all new and old defined variables.
			i->Update_Members_Size();
		}
		//now apply those revaluated values into us.
		Update_Size();
	}*/

	void Update_Local_Variable_Mem_Offsets() {
		Local_Allocation_Space = 0;
		for (auto i : Defined) {
			if (i->is(FUNCTION_NODE) || !i->Requires_Address || i->Is_Template_Object)
				continue;
			i->Memory_Offset = Local_Allocation_Space;
			Local_Allocation_Space += i->Size;
		}

		for (auto i : Childs) {
			for (auto j : i->Get_all({ IF_NODE, ELSE_IF_NODE, ELSE_NODE, WHILE_NODE })) {
				j->Update_Local_Variable_Mem_Offsets(&Local_Allocation_Space);
			}
		}
		return;
	}

	void Update_Local_Variable_Mem_Offsets(int* Current_Allocation_Space) {
		for (auto i : Defined) {
			if (i->is(FUNCTION_NODE) || (!i->Requires_Address && !i->Scope->is(OBJECT_DEFINTION_NODE)))
				continue;
			i->Memory_Offset = *Current_Allocation_Space;
			*Current_Allocation_Space += i->Size;
		}

		for (auto i : Childs) {
			for (auto j : i->Get_all({ IF_NODE, ELSE_IF_NODE, ELSE_NODE, WHILE_NODE })) {
				j->Update_Local_Variable_Mem_Offsets(Current_Allocation_Space);
			}
		}
		return;
	}

	void Update_Member_Variable_Offsets(Node* obj) {
		int Current_Offset = 0;
		for (auto i : obj->Defined) {
			if (i->is(FUNCTION_NODE))
				continue;

			if (!obj->is(FUNCTION_NODE)) {
				i->Memory_Offset = Current_Offset;
				Current_Offset += i->Get_Size();
			}

			if (i->Defined.size() > 0) {
				Update_Member_Variable_Offsets(i);
			}
		}
	}

	void Update_Defined_Stack_Offsets();

	void Update_Format() {
		if (this->is(NUMBER_NODE))
			return;
		if (this->is("const") != -1 && this->Name == "format")
			return;
		if (Is_Template_Object)
			return;
		Format = "integer";
		for (string s : Inheritted) {
			//there is no inheritable type that doesnt have enything init.
			if (Lexer::GetComponents(s)[0].is(Flags::KEYWORD_COMPONENT))
				continue;
			Node* t = Find(s, Scope, CLASS_NODE);
			t->Update_Format();
			if (t->Format == "integer")
				Format = t->Get_Format();
			if (t->Format != "integer")
				Format = t->Format;
		}
		for (auto i : Defined) {
			if (i->Name == "format" && i->is("const") != -1)
				Format = i->Format;
		}
	}

	void Update_Inheritance();

	Node* Copy_Node(Node* What_Node, Node* p);

	vector<Node*> Has(Node* n, int f)
	{
		vector<Node*> Result;
		if (n->is(OPERATOR_NODE)) {
			vector<Node*> left = Has(n->Left, f);
			Result.insert(Result.end(), left.begin(), left.end());

			vector<Node*> right = Has(n->Right, f);
			Result.insert(Result.end(), right.begin(), right.end());
		}

		else if (n->is(CONTENT_NODE)) {
			for (Node* i : n->Childs)
				if (i->is(f))
					Result.push_back(i);
		}

		if (n->Fetcher != nullptr) {
			vector<Node*> fetcher = Has(n->Fetcher, f);
			Result.insert(Result.end(), fetcher.begin(), fetcher.end());
		}


		/*
		//this doesnt work dont use unless must!!
		else if (n->is(CALL_NODE)) {
			for (Node* i : n->Parameters)
				if (i->is(f))
					Result.push_back(i);
		}*/

		if (n->is(f))
			Result.push_back(n);

		return Result;
	}

	vector<Node*> Has(int f) {
		return Has(this, f);
	}

	int Has(vector<string> s) {
		for (int i = 0; i < s.size(); i++)
			if (is(s[i]) != -1)
				return is(s[i]);
		return -1;
	}

	bool Has(vector<int> s) {
		for (int i = 0; i < s.size(); i++)
			if (is(s[i]))
				return is(s[i]);
		return false;
	}

	static bool Has(vector<Node*> l, Node* n) {
		for (auto i : l)
			for (auto j : i->Has(n->Type))
				if (i->Name == n->Name)
					return true;
		return false;
	}


	vector<Node*> Get_all(int f, vector<Node*> Trace);

	vector<Node*> Get_all(int f = -1) {
		return Get_all(f, vector<Node*>());
	}

	vector<Node*> Get_all(vector<int> flags) {
		vector<Node*> Result;
		for (auto i : flags) {
			vector<Node*> tmp = Get_all(i);
			Result.insert(Result.end(), tmp.begin(), tmp.end());
		}
		return Result;
	}

	/// <summary>
	/// Gets amount of specified int the parameter from inheritance
	/// </summary>
	int Get_All(string s) {
		int Result = 0;
		for (auto i : Inheritted)
			if (i == s)
				Result++;
		return Result;
	}

	//static vector<Node*> Get_all(int f, vector<Node*> l) {
	//	vector<Node*> Result;
	//	for (Node* n : l)
	//		if (n->is(f))
	//			Result.push_back(n);
	//	return Result;
	//}

	bool Is_Decimal() {
		if (find(Name.begin(), Name.end(), '.') != Name.end()) {
			return true;
		}
		else {
			return false;
		}
	}

	int Get_Size() {
		return Size;
	}

	template<typename T>
	vector<T>& Append(vector<T>& d, vector<T> s) {
		for (int i = 0; i < s.size(); i++)
			d.push_back(s[i]);
		return d;
	}

	string Get_Format() {
		for (auto i : Defined) {
			if (i->Name == "format")
				if (i->is("const") != -1)
					return i->Format;
		}
		return "integer";
	}

	//Gets other side of operator, or the callation parameter which it goes to.
	Node* Get_Pair() {
		if (Context->Has({ OPERATOR_NODE, ASSIGN_OPERATOR_NODE, BIT_OPERATOR_NODE, CONDITION_OPERATOR_NODE, ARRAY_NODE })) {
			if (Context->Left == this) {
				if (!Context->Right->Has({ OPERATOR_NODE, ASSIGN_OPERATOR_NODE, BIT_OPERATOR_NODE, CONDITION_OPERATOR_NODE, ARRAY_NODE }))
					return Context->Right;
				else
					return Context->Right->Get_Most_Left();
			}			
			else if (Context->Right == this) {
				if (!Context->Left->Has({ OPERATOR_NODE, ASSIGN_OPERATOR_NODE, BIT_OPERATOR_NODE, CONDITION_OPERATOR_NODE, ARRAY_NODE }))
					return Context->Left;
				else
					return Context->Left->Get_Most_Right();
			}
		}
		else if (Context->is(CALL_NODE)) {
			//get first the index of paramter this is in the callation
			int Parameter_Index = 0;
			for (auto i : Context->Parameters) {
				if (i == this)
					break;
				Parameter_Index++;
			}

			//return the representive Node from the Function implemetation's paramters
			return Context->Function_Implementation->Parameters[Parameter_Index];
		}
		throw::runtime_error("ERROR!");
	}

	//Transform all this A.B.C.D into D->C->B->A
	void Transform_Dot_To_Fechering(Node* To);

	string Construct_Template_Type_Name() {
		if (Templates.size() == 0)
			return Name;
		string Result = "____" + Name + "_";
		for (auto i : Templates)
			Result += i->Construct_Template_Type_Name() + "_";
		return Result;
	}

	Node* Get_Closest_Context(int Flags);

	void Clean();
};

#endif