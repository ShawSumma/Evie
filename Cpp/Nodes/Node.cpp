#include "../../H/Nodes/Node.h"
#include "../../H/Nodes/Token.h"
#include "../../H/Parser/Algebra.h"
#include "../../H/Docker/Mangler.h"
#include "../../H/UI/Safe.h"
#include "../../H/UI/Usr.h"

#include <vector>
#include <string>
#include <climits>

extern Usr* sys;

using namespace std;

void Node::Update_Defined_Stack_Offsets()
{
	//skip the returnin address.
	int Parameter_Offset = 8;
	int Local_Offset = 0;
	//first conmpute all the memory locaitions.
	for (auto& i : Defined) {
		if (i->is(OBJECT_NODE) || i->is(OBJECT_DEFINTION_NODE)) {
			//every local variable is defined default as a value in a register.
			if (i->Requires_Address) {
				i->Memory_Offset = Local_Offset/* + Size_of_Call_Space*/;
				Local_Offset += i->Get_Size();
			}
		}
		if (i->is(PARAMETER_NODE)) {
			if (Token(i, true).is(TOKEN::REGISTER)) {
				//This happends when the parameter would be in a register but its in debug, so it uses stack
				//we need to tell the system that it CAN use the Local_Offset as storing the parameter
				i->Memory_Offset = Local_Offset/* + Size_of_Call_Space*/;
				Local_Offset += i->Get_Size();
			}
			else if (!Token(i).is(TOKEN::REGISTER)) {
				//the parameters locate below the local variable space and the returning address 
				//also, do remeber that, the pushes that the code needs for the nonvolatiles
				//those come before the parameter space.
				i->Memory_Offset = Parameter_Offset/* + Size_of_Call_Space*/;
				Parameter_Offset += i->Get_Size();
			}
		}
	}
}

Variable_Descriptor::Variable_Descriptor(Node* v, int i, vector<Node*> source) {
	Define_Index = i;
	Var = v;

	vector<Node*> Linear_Ast = v->Get_all({OBJECT_DEFINTION_NODE, OBJECT_NODE, PARAMETER_NODE});
	//for complex inlinings
	for (int n = i; n < source.size(); n++) {
		if (source[n]->is(ASSIGN_OPERATOR_NODE) || source[n]->is(POSTFIX_NODE)) {
			for (auto c : Linear_Ast) {
				if (source[n]->Left->Name == c->Name) {
					Expiring_Index = n;
					goto Stop;
				}
			}
		}
		if (source[n]->is(PREFIX_NODE)) {
			for (auto c : Linear_Ast) {
				if (source[n]->Right->Name == c->Name) {
					Expiring_Index = n;
					goto Stop;
				}
			}
		}
		if (source[n]->is(CALL_NODE)) {
			for (auto c : Linear_Ast) {
				for (auto p : source[n]->Function_Implementation->Parameters) {
					if (p->is("ptr") != -1)
						if (p->Name == c->Name) {
							Expiring_Index = n;
							goto Stop;
						}
				}
			}
		}
		Expiring_Index = n;
	}
Stop:;
}

vector<string> Node::Get_Inheritted(bool Skip_Prefixes, bool Get_Name, bool Skip_Keywords) {
	vector<string> Result;
	if (MANGLER::Is_Base_Type(this) || Get_Name) {
		return { Name };
	}
	else if (is(NUMBER_NODE)) {
		//1.29348
		if (find(Name.begin(), Name.end(), '.') != Name.end()) {
			if ((Name.end() - find(Name.begin(), Name.end(), '.')) <= 7)
				return Find(4, Global_Scope, "decimal")->Get_Inheritted(Skip_Prefixes, true);
			else
				return Find(8, Global_Scope, "decimal")->Get_Inheritted(Skip_Prefixes, true);
		}
		else {
			if (atoll(Name.c_str()) > INT_MAX) {
				return Find(8, Global_Scope, "integer")->Get_Inheritted(Skip_Prefixes, true);
			}
			return Find(4, Global_Scope, "integer")->Get_Inheritted(Skip_Prefixes, true);
		}
	}
	else {
		for (int i = 0; i < Inheritted.size(); i++) {
			if (Skip_Prefixes && ((Inheritted[i] == "ptr") || (Inheritted[i] == "ref")))
				continue;
			if (Skip_Keywords && Lexer::GetComponent(Inheritted[i]).is(::Flags::KEYWORD_COMPONENT) && Inheritted[i] != "ptr")
				continue;
			Result.push_back(Inheritted[i]);
		}
		return Result;
	}
}

string Node::Get_Inheritted(string seperator, bool Skip_Prefixes, bool Get_Name, bool skip_keywords) {
	if (MANGLER::Is_Base_Type(this) || Get_Name) {
		return seperator + Name;
	}
	else if (is(NUMBER_NODE) && Cast_Type == nullptr) {
		//1.29348
		if (find(Name.begin(), Name.end(), '.') != Name.end()) {
			if ((Name.end() - find(Name.begin(), Name.end(), '.')) <= 7)
				return Find(4, Global_Scope, "decimal")->Get_Inheritted(seperator, Skip_Prefixes, true);
			else
				return Find(8, Global_Scope, "decimal")->Get_Inheritted(seperator,  Skip_Prefixes, true);
		}
		else {
			if (atoll(Name.c_str()) > INT_MAX) {
				return Find(8, Global_Scope, "integer")->Get_Inheritted(seperator, Skip_Prefixes, true);
			}
			return Find(4, Global_Scope, "integer")->Get_Inheritted(seperator, Skip_Prefixes, true);
		}
	}
	else if (is(NUMBER_NODE) && Cast_Type != nullptr) {
		if (Cast_Type != nullptr)
			return seperator + Cast_Type->Name;
	}
	else {
		string result = "";
		for (int i = 0; i < Inheritted.size(); i++) {
			if (Skip_Prefixes && ((Inheritted[i] == "ptr") || (Inheritted[i] == "ref")))
				continue;
			else if (skip_keywords && Lexer::GetComponents(Inheritted[i])[0].is(Flags::KEYWORD_COMPONENT))
				continue;
			result += seperator + Inheritted[i];
		}
		if (Cast_Type != nullptr)
			result = seperator + Cast_Type->Name;
		return result;
	}
}

int Node::Calculate_Inheritted_Distance(Node* Val, Node* Loader, string type)
{
	int Val_Ptr_Count = Val->Get_All(type);
	int Loader_Ptr_Count = Loader->Get_All(type);

	//if there is ptr involved					&& if the ptr distance is more than 1 
	if ((Val_Ptr_Count + Loader_Ptr_Count > 0) && (abs(Val_Ptr_Count - Loader_Ptr_Count) > 2)) {
		if (Loader_Ptr_Count > Val_Ptr_Count && type == "ptr") {
			//this means that the Loader ptr count is greter than the Val ptr by *2
			Report(Observation(ERROR, "Cannot wrap " + Val->Name + to_string(abs(Val_Ptr_Count - Loader_Ptr_Count)) + " times into " + Loader->Name, *Val->Location));
		}
	}

	return abs(Val_Ptr_Count - Loader_Ptr_Count);
}

Node* Node::Find_Scope(Node* n)
{
	Node* Current_Scope = n->Scope;
	vector<Node*> Fetchers = n->Get_All_Fetchers();

	reverse(Fetchers.begin(), Fetchers.end());

	while (true) {
		for (auto& i : Current_Scope->Defined) {
			if (Fetchers.back()->Name == i->Name) {
				Current_Scope = i;
				Fetchers.pop_back();
			}
			if (Fetchers.size() == 0)
				break;
		}

		if (Fetchers.size() == 0)
			break;
		if (Current_Scope->Scope == nullptr)
			Report(Observation(ERROR, "Scope not found", *n->Location));

		Current_Scope = Current_Scope->Scope;
	}

	return Current_Scope;
}

Node* is(Node* Result) {
	if (Result) {
		return Result;
	}

	//out of bounds
	else if (Result == nullptr) {
		return 0;
	}
}

#define TOO_BIG (Node*)1

#define TOO_SMOLL nullptr

//this algorithm is optimized by starting from low-hi.
//because this system ingnores automatically if the node is already beoynd the line and character number.
//Returns 1 if not found and returns 0 if the node is beyond the location.
vector<Node*> Find_Trace;
Node* Node::Find(Position& location)
{
	for (int j = 0; j < Find_Trace.size(); j++) {
		if (this == Find_Trace[j]) {
			return TOO_BIG;
		}
	}

	Find_Trace.push_back(this);

	if (Location->GetAbsolute() > location.GetAbsolute()) {
		Find_Trace.pop_back();
		return TOO_BIG;
	}
	else if (Location->GetAbsolute() < location.GetAbsolute()) {

		vector<Node*> Defined_Reversed = Defined;

		::sort(Defined_Reversed.begin(), Defined_Reversed.end(), [](Node* a, Node* b) { 
			return ((a->Location->GetFileIndex() > b->Location->GetFileIndex())
				|| (a->Location->GetFileIndex() == b->Location->GetFileIndex()
					&& a->Location->GetLine() > b->Location->GetLine()
					));

			//!((file1 > file2) || (file1 == file2 && line1 > line))
		});

		/*return += { 
			reverse(Defined.begin(), Defined.end());
		}*/
		for (auto& i : Defined_Reversed) {
			Node* Result = i->Find(location);

			if (Result == TOO_BIG)
				continue;

			else if (Result == TOO_SMOLL)
				break;	//we have gone over the result

			else {
				Find_Trace.pop_back();
				return Result;
			}
		}

		vector<Node*> Childs_Reversed = Childs; 
		
		::sort(Childs_Reversed.begin(), Childs_Reversed.end(), [](Node* a, Node* b) {
			return ((a->Location->GetFileIndex() > b->Location->GetFileIndex())
				|| (a->Location->GetFileIndex() == b->Location->GetFileIndex()
					&& a->Location->GetLine() > b->Location->GetLine()
					));

			//!((file1 > file2) || (file1 == file2 && line1 > line))
			});

		for (auto& i : Childs_Reversed) {
			Node* Result = i->Find(location);

			if (Result == TOO_BIG)
				continue;

			else if (Result == TOO_SMOLL)
				break;	//we have gone over the result

			else {
				Find_Trace.pop_back();
				return Result;
			}
		}

		vector<Node*> Parameter_Reversed = Parameters;
		reverse(Parameter_Reversed.begin(), Parameter_Reversed.end());

		for (auto& i : Parameter_Reversed) {
			Node* Result = i->Find(location);

			if (Result == TOO_BIG)
				continue;

			else if (Result == TOO_SMOLL)
				break;	//we have gone over the result

			else {
				Find_Trace.pop_back();
				return Result;
			}
		}

		if (Left) {
			Node* Result = Left->Find(location);

			if (Result != TOO_BIG && Result != TOO_SMOLL) {
				Find_Trace.pop_back();
				return Result;
			}
		}
		if (Right) {
			Node* Result = Right->Find(location);

			if (Result != TOO_BIG && Result != TOO_SMOLL) {
				Find_Trace.pop_back();
				return Result;
			}
		}
		if (Fetcher) {
			Node* Result = Fetcher->Find(location);

			if (Result != TOO_BIG && Result != TOO_SMOLL) {
				Find_Trace.pop_back();
				return Result;
			}
		}
		if (Cast_Type) {
			Node* Result = Cast_Type->Find(location);

			if (Result != TOO_BIG && Result != TOO_SMOLL) {
				Find_Trace.pop_back();
				return Result;
			}
		}
	}
	else {
		Find_Trace.pop_back();
		return this;
	}

	Find_Trace.pop_back();
	return TOO_SMOLL;
}

bool Node::Compare_Fetchers(Node* other)
{
	string This_Fethcers = "";
	for (auto i : Get_All_Fetchers()) {
		if (Find(i, Scope, CLASS_NODE) || (Scope->Name == i->Name && Scope->is(CLASS_NODE)))
			This_Fethcers += i->Name;
		else
			This_Fethcers += i->Get_Inheritted("", false, false, false);

		//multi dimensional templates are disabled
		for (auto j : i->Templates)
			This_Fethcers += j->Name;
	}
	if (this->Fetcher == nullptr)
		if (this->Has({ FUNCTION_NODE, CALL_NODE }))
			This_Fethcers += this->Name;

	string Other_Fethcers = "";
	for (auto i : other->Get_All_Fetchers()) {
		if (Find(i, Scope, CLASS_NODE) || (Scope->Name == i->Name && Scope->is(CLASS_NODE)))
			Other_Fethcers += i->Name;
		else
			Other_Fethcers += i->Get_Inheritted("", false, false, false);

		for (auto j : i->Templates)
			Other_Fethcers += j->Name;
	}
	if (other->Fetcher == nullptr)
		if (other->Has({ FUNCTION_NODE, CALL_NODE }))
			Other_Fethcers += other->Name;

	if (This_Fethcers == Other_Fethcers)
		return true;
	return false;
}

vector<Node*> Node::Get_All_Fetchers()
{
	vector<Node*> Result;
	if (Fetcher != nullptr) {
		vector<Node*> tmp = Fetcher->Get_All_Fetchers();
		Result.insert(Result.end(), tmp.begin(), tmp.end());
		Result.push_back(Fetcher);
	}
	//Result.push_back(this);
	return Result;
}

Node* Node::Get_Scope_As(int F, Node* scope) {
	if (scope->is(F))
		return scope;
	if (scope->Scope != nullptr)
		return Get_Scope_As(F, scope->Scope);
	Report(Observation(ERROR, "Parent NULL!!", *Location));
	throw::runtime_error("ERROR!");
}

Node* Node::Get_Scope_As(int F, vector<string> inheritted, Node* scope)
{
	if (!scope->is(F))
		return Get_Scope_As(F, inheritted, scope->Scope);

	for (auto i : inheritted)
		if (scope->is(i) == -1)
			goto Not_Right_Scope;

	return scope;
	
	Not_Right_Scope:;
	return Get_Scope_As(F, inheritted, scope->Scope);
}

Node* Node::Get_Context_As(int F, Node* Context)
{
	if (Context->is(F))
		return Context;
	if (Context->Context)
		return Get_Context_As(F, Context->Context);
	return nullptr;
}

Node* Node::Get_Context_As(string n, Node* Context)
{
	if (Context->Name == n)
		return Context;
	if (Context->Context)
		return Get_Context_As(n, Context->Context);
	return nullptr;
}

/// <summary>
/// Gets a list of all the upper parents that this is defined in, up to global scope :D
/// </summary>
/// <returns></returns>
vector<Node*> Node::Get_Scope_Path()
{
	vector<Node*> Result;
	Node* Current_Scope = Scope;
	while (Current_Scope->Name != "GLOBAL_SCOPE") {

		Result.push_back(Current_Scope);
		Current_Scope = Current_Scope->Scope;

		if (Current_Scope == nullptr)
			Report(Observation(ERROR, "Parental Scope was not found.", *this->Location));
	}
	return Result;
}

Node* Node::Find(Node* n, Node* s)
{
	//some criteria
	if (n->Name == "\n")
		return nullptr;
	if (n->is(NUMBER_NODE) || n->is(STRING_NODE) || n->is(LABEL_NODE))
		return n;
	if (s == nullptr) {
		Report(Observation(ERROR, "Critical Error: parent is null!", *Location));
		throw::runtime_error("ERROR!");
	}

	if (n->is(TEMPLATE_NODE))
		for (auto& i : s->Templates)
			if (i->Templates.size() == n->Templates.size())
				if (i->Name == n->Name)
					return i;

	//The feching find that finds the Scope_Path, algorithm will be start before normal search-
	//because of same named objects in the current scope.
	//Normal current and above going scope search algorithm
	for (Node* i : s->Defined)
		if (i->Templates.size() == n->Templates.size())
			if (i->Name == n->Name) {
				if (n->Cast_Type != nullptr) {
					Node* tmp = i->Copy_Node(i, i->Scope);
					tmp->Cast_Type = n->Cast_Type;
					return tmp;
				}
				return i;
			}

	for (Node* i : s->Inlined_Items) {
		if (i->Templates.size() == n->Templates.size())
			if (i->Name == n->Name) {
				if (n->Cast_Type != nullptr) {
					Node* tmp = i->Copy_Node(i, i->Scope);
					tmp->Cast_Type = n->Cast_Type;
					return tmp;
				}
				return i;
			}
	}

	if (n->Fetcher != nullptr)
		if (n->Fetcher != s)
			for (auto& i : Find_Scope(n)->Defined)
				if (i->Templates.size() == n->Templates.size())
					if (i->Name == n->Name)
						return i;

	if (s->Fetcher != nullptr)
		if (s->Fetcher != s->Scope)
			for (auto& i : Find_Scope(s)->Defined)
				if (i->Templates.size() == n->Templates.size())
					if (i->Name == n->Name)
						return i;

	//If the current scope doesn't have the wanted object, then try at one spet higher scope.
	if (s->Scope != nullptr)
		if (Find(n, s->Scope) != nullptr)
			return Find(n, s->Scope);

	//IDK what this does, please explain!
	if (s->Cast_Type != nullptr)
		for (auto& i : s->Find(s->Cast_Type, s, CLASS_NODE)->Defined)
			if (i->Templates.size() == n->Templates.size())
				if (i->Name == n->Name)
					return i;

	return nullptr;
}

Node* Node::Find(Node* n, Node* s, int f)
{
	//some criteria
	if (n->Name == "\n")
		return nullptr;
	if (n->is(NUMBER_NODE) || n->is(STRING_NODE) || n->is(LABEL_NODE))
		return n;
	if (s == nullptr) {
		Report(Observation(ERROR, "Critical Error: parent is null!", *Location));
		throw::runtime_error("ERROR!");
	}

	if (n->is(TEMPLATE_NODE))
		for (auto& i : s->Templates)
			if (i->is(n->Type))
				if (i->Templates.size() == n->Templates.size())
					if (i->Name == n->Name)
						return i;

	//The feching find that finds the Scope_Path, algorithm will be start before normal search-
	//because of same named objects in the current scope.
	//Normal current and above going scope search algorithm
	for (Node* i : s->Defined)
		if (i->is(f))
			if (i->Templates.size() == n->Templates.size())
				if (i->Name == n->Name) {
					if (n->Cast_Type != nullptr) {
						Node* tmp = i->Copy_Node(i, i->Scope);
						tmp->Cast_Type = n->Cast_Type;
						return tmp;
					}
					return i;
				}

	for (Node* i : s->Inlined_Items) {
		if (i->is(f))
			if (i->Templates.size() == n->Templates.size())
				if (i->Name == n->Name) {
					if (n->Cast_Type != nullptr) {
						Node* tmp = i->Copy_Node(i, i->Scope);
						tmp->Cast_Type = n->Cast_Type;
						return tmp;
					}
					return i;
				}
	}

	if (n->Fetcher != nullptr)
		for (auto& i : Find_Scope(n)->Defined)
			if (i->is(f))
				if (i->Templates.size() == n->Templates.size())
					if (i->Name == n->Name)
						return i;

	//If the current scope doesn't have the wanted object, then try at one spet higher scope.
	if (s->Scope != nullptr)
		if (Find(n, s->Scope, f) != nullptr)
			return Find(n, s->Scope, f);

	//IDK what this does, please explain!
	if (s->Cast_Type != nullptr && s->Cast_Type->Name != n->Name)
			for (auto& i : s->Find(s->Cast_Type, s, CLASS_NODE)->Defined)
				if (i->is(f))
					if (i->Templates.size() == n->Templates.size())
						if (i->Name == n->Name)
							return i;
	
	return nullptr;
}

Node* Node::Find(string name, Node* s, int flags) {
	if (name == "\n")
		return nullptr;
	if (s == nullptr) {
		Report(Observation(ERROR, "Critical Error: parent is null!", *Location));
		throw::runtime_error("ERROR!");
		return nullptr;
	}

	for (auto& i : s->Templates)
		if (i->is(flags))
			if (i->Name == name)
				return i;

	for (Node* i : s->Defined)
		if (i->is(flags))
			if (i->Name == name)
				return i;

	for (Node* i : s->Inlined_Items)
		if (i->is(flags))
			if (i->Name == name)
				return i;

	if (s->Scope != nullptr)
		if (Find(name, s->Scope, flags) != nullptr)
			return Find(name, s->Scope, flags);

	if (s->Cast_Type != nullptr && s->Cast_Type->Name != name && s->Find(s->Cast_Type, s, flags) != nullptr)
		for (auto& i : s->Find(s->Cast_Type, s, { CLASS_NODE, OBJECT_DEFINTION_NODE, OBJECT_NODE })->Defined)
				if (i->Name == name)
					return i;

	if (s->Fetcher != nullptr) {
		Node* F = Find_Scope(s);
		if (F != nullptr)
			if (Find(name, F, flags) != nullptr)
				return Find(name, F, flags);
	}
	return nullptr;
}

Node* Node::Find(string name, Node* s, bool Need_Parent_existence) {
	if (name == "\n")
		return nullptr;
	if (s == nullptr /*&& Need_Parent_existance*/) {
		Report(Observation(ERROR, "Critical Error: parent is null!", *Location));
		throw::runtime_error("ERROR!");
		return nullptr;
	}

	for (auto& i : s->Templates)
		if (i->Name == name)
			return i;

	for (Node* i : s->Defined)
		if (i->Name == name)
			return i;

	for (Node* i : s->Inlined_Items)
		if (i->Name == name)
			return i;

	if (s->Scope != nullptr)
		if (Find(name, s->Scope, Need_Parent_existence) != nullptr)
			return Find(name, s->Scope, Need_Parent_existence);

	if (s->Cast_Type != nullptr && s->Cast_Type->Name != name )
		for (auto& i : s->Find(s->Cast_Type, s, { CLASS_NODE, OBJECT_DEFINTION_NODE, OBJECT_NODE })->Defined)
				if (i->Name == name)
					return i;

	if (s->Fetcher != nullptr) {
		Node* F = Find_Scope(s);
		if (F != nullptr)
			if (Find(name, F, Need_Parent_existence) != nullptr)
				return Find(name, F, Need_Parent_existence);
	}
	return nullptr;
}

void Node::Get_Inheritted_Class_Members() {
	for (auto Inherit : Inheritted) {
		if (Lexer::GetComponent(Inherit).is(::Flags::KEYWORD_COMPONENT))
			continue;
		vector<Node*> Inheritted_Members = Find(Inherit, Scope)->Defined;

		reverse(Inheritted_Members.begin(), Inheritted_Members.end());

		for (auto Member : Inheritted_Members)
			if (Member->is("const") == -1)
				if (Locate(Member->Name, Defined) == false)
					Defined.insert(Defined.begin(), Member);
	}
}

void Node::Update_Inheritance() {
	if (Cast_Type != nullptr)
		return;
	//save all keyword inheritances.
	vector<string> Keyword_Inheritance;
	for (auto i : Inheritted)
		if (Lexer::GetComponents(i)[0].is(Flags::KEYWORD_COMPONENT))
			Keyword_Inheritance.push_back(i);
	//remove all inheritances
	Inheritted.clear();

	Inheritted.push_back(Cast_Type->Name);
	Inheritted.insert(Inheritted.end(), Keyword_Inheritance.begin(), Keyword_Inheritance.end());
}

void Node::Transform_Dot_To_Fechering(Node* To)
{
	//(((A.B).C).D).Banana()
	//we start at the Dot that is left side of Banana()
	//((A.B).C).D | this == (..).D
	if (Name == ".") {
		//set the left side to To
		To->Fetcher = Right;
		if (Left->Name == ".") {
			Left->Transform_Dot_To_Fechering(Right);
		}
	}
	else
		To->Fetcher = this;
}

Node* Node::Get_Closest_Context(int Flags)
{
	if (!this)
		return nullptr;

	if (is(Flags))
		return this;

	Node* Closest_Context = Context->Get_Closest_Context(Flags);

	if (Closest_Context)
		return Closest_Context;

	return nullptr;
}

void Node::Clean()
{
	Header.clear();
	Defined.clear();
	Parameters.clear();
	Childs.clear();
	Inlined_Items.clear();
	Member_Functions.clear();
	Operator_Overloads.clear();
	Templates.clear();
	Inheritable_templates.clear();
	Template_Children.clear();
	Numerical_Return_Types.clear();
}

vector<Node*> Trace_Update_Size;
int Node::Update_Size() {
	if (is("const") != -1 && Size != 0 || Is_Template_Object)
		return Size;

	for (int j = 0; j < Trace_Update_Size.size(); j++)
		if (this == Trace_Update_Size[j]) {
			Trace_Update_Size.pop_back();
			if (is("ptr") != -1 || is("func") != -1)
				return _SYSTEM_BIT_SIZE_;
			return Size;
		}

	Trace_Update_Size.push_back(this);

	Size = 0;
	if (!is(FUNCTION_NODE))
		for (auto Member : Defined) {
			if (Member->Has({ FUNCTION_NODE, PROTOTYPE, IMPORT, EXPORT }))
				Member->Update_Size();
			else
				Size += Member->Update_Size();
		}

	//this must be done after the members size are all set because the ptr will override them size.
	for (auto Inherit : Inheritted) {
		if (Lexer::GetComponent(Inherit).is(Flags::KEYWORD_COMPONENT)) {
			if (Inherit == "ptr" || Inherit == "func") {
				Size = _SYSTEM_BIT_SIZE_;
				break;
			}
			continue;
		}
		else if (MANGLER::Is_Base_Type(Find(Inherit)))
			Size += Find(Inherit)->Update_Size();
	}
	Trace_Update_Size.pop_back();
	return Size;
}

vector<Node*> Node::Get_all(int f, vector<Node*> Trace)
{

	if (this->is(FUNCTION_NODE))
		if (this->is(f) || f == -1)
			return { this };
		else
			return {};


	for (int j = 0; j < Trace.size(); j++)
		if (this == Trace[j]) {
			Trace.pop_back();
			if (this->is(f))
				return { new Node(*this) };
			return {};
		}

	Trace.push_back(this);

	vector<Node*> Result;
	if (Left != nullptr) {
		vector<Node*> left = Left->Get_all(f, Trace);
		Result.insert(Result.end(), left.begin(), left.end());
	}
	if (Right != nullptr) {
		vector<Node*> right = Right->Get_all(f, Trace);
		Result.insert(Result.end(), right.begin(), right.end());
	}
	if (Succsessor != nullptr) {
		vector<Node*> Succsessors = Succsessor->Get_all(f, Trace);
		Result.insert(Result.end(), Succsessors.begin(), Succsessors.end());
	}
	if (Predecessor != nullptr) {
		vector<Node*> Predecessors = Predecessor->Get_all(f, Trace);
		Result.insert(Result.end(), Predecessors.begin(), Predecessors.end());
	}
	if (Fetcher != nullptr) {
		//vector<Node*> Fetchers = Fetcher->Get_all(f, Trace);
		//Result.insert(Result.end(), Fetchers.begin(), Fetchers.end());
		if (Fetcher->is(f))
			Result.push_back(Fetcher);
	}
	for (Node* i : Header) {
		vector<Node*> Headers = i->Get_all(f, Trace);
		Result.insert(Result.end(), Headers.begin(), Headers.end());
	}
	for (Node* i : Childs) {
		vector<Node*> childs = i->Get_all(f, Trace);
		Result.insert(Result.end(), childs.begin(), childs.end());
	}
	for (Node* i : Parameters) {
		vector<Node*> childs = i->Get_all(f, Trace);
		Result.insert(Result.end(), childs.begin(), childs.end());
	}
	for (Node* i : Defined) {
		vector<Node*> childs = i->Get_all(f, Trace);
		Result.insert(Result.end(), childs.begin(), childs.end());
	}
	if (Cast_Type) {
		//vector<Node*> childs = Cast_Type->Get_all(f, Trace);
		//Result.insert(Result.end(), childs.begin(), childs.end());
		if (Cast_Type->is(f))
			Result.push_back(Cast_Type);
	}

	if (is(f) || f == -1)
		Result.push_back(this);

	for (int i = 0; i < Result.size(); i++)
		for (int j = 0; j < Result.size(); j++)
			if (Result[i] == Result[j] && i != j)
				Result.erase(Result.begin() + j--);

	Trace.pop_back();

	return Result;
}

//			old  , new
vector<pair<Node*, Node*>> Trace;
Node* Node::Copy_Node(Node* What_Node, Node* p)
{
	if (What_Node == nullptr)
		return nullptr;

	if (What_Node->is(FUNCTION_NODE))
		return What_Node;

	for (int j = 0; j < Trace.size(); j++) {
		if (What_Node == Trace[j].first) {
			return Trace[j].second;
		}
	}

	//this will only copy the ptrs in list but we want to also copy what those ptr point to.
	Node* Result = new Node(*What_Node);
	Result->Scope = p;

	Trace.push_back({ What_Node, Result });

	//lets start from defined
	for (int i = 0; i < Result->Defined.size(); i++)
		Result->Defined[i] = Copy_Node(Result->Defined[i], Result);

	for (int i = 0; i < Result->Templates.size(); i++)
		Result->Templates[i] = Copy_Node(Result->Templates[i], Result);

	for (int i = 0; i < Result->Childs.size(); i++)
		if (Result->is(CONTENT_NODE))
			Result->Childs[i] = Copy_Node(Result->Childs[i], p);
		else
			Result->Childs[i] = Copy_Node(Result->Childs[i], Result);

	for (int i = 0; i < Result->Member_Functions.size(); i++)
		Result->Member_Functions[i] = Copy_Node(Result->Member_Functions[i], Result);

	for (int i = 0; i < Result->Operator_Overloads.size(); i++)
		Result->Operator_Overloads[i] = Copy_Node(Result->Operator_Overloads[i], Result);

	for (int i = 0; i < Result->Parameters.size(); i++) {
		Node* scope = Result;
		if (Result->is(CALL_NODE))
			scope = Result->Scope;

		Result->Parameters[i] = Copy_Node(Result->Parameters[i], scope);

		if (Result->is(CALL_NODE))
			Result->Parameters[i]->Context = Result;
	}

	for (int i = 0; i < Result->Header.size(); i++)
		Result->Header[i] = Copy_Node(Result->Header[i], p);


	for (int i = 0; i < Result->Numerical_Return_Types.size(); i++)
		Result->Numerical_Return_Types[i] = Copy_Node(Result->Numerical_Return_Types[i], p);

	if (Result->Left) {
		Result->Left = Copy_Node(Result->Left, p);
		Result->Left->Context = Result;
	}
	if (Result->Right) {
		Result->Right = Copy_Node(Result->Right, p);
		Result->Right->Context = Result;
	}

	Result->Succsessor = Copy_Node(Result->Succsessor, p);
	Result->Predecessor = Copy_Node(Result->Predecessor, p);

	if (Result->Fetcher)
		Result->Fetcher = Copy_Node(Result->Fetcher, p);


	if (Result->Succsessor) {
		Result->Succsessor->Predecessor = Result;
	}
	if (Result->Predecessor) {
		Result->Predecessor->Succsessor = Result;
	}

	if (Result->Location)
		Result->Location = new Position(*Result->Location);

	if (Result->Cast_Type)
		Result->Cast_Type = Copy_Node(Result->Cast_Type, p);

	//The copying prosess must go downwards not upwards, otherwise it will loop forever!
	//Result->Holder = Copy_Node(Result->Holder, p);

	Trace.pop_back();
	//now we have copyed every ptr into a new base to point.
	return Result;
}