#include "../../H/Nodes/Node.h"
#include "../../H/Nodes/Token.h"
#include "../../H/Parser/Algebra.h"
#include "../../H/Docker/Mangler.h"
#include "../../H/UI/Safe.h"

#include <vector>
#include <string>
#include <climits>

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
				i->Memory_Offset = Local_Offset + Size_of_Call_Space;
				Local_Offset += i->Get_Size();
			}
		}
		if (i->is(PARAMETER_NODE)) {
			if (Token(i, true).is(TOKEN::REGISTER)) {
				//This happends when the parameter would be in a register but its in debug, so it uses stack
				//we need to tell the system that it CAN use the Local_Offset as storing the parameter
				i->Memory_Offset = Local_Offset + Size_of_Call_Space;
				Local_Offset += i->Get_Size();
			}
			else if (!Token(i).is(TOKEN::REGISTER)) {
				//the parameters locate below the local variable space and the returning address 
				//also, do remeber that, the pushes that the code needs for the nonvolatiles
				//those come before the parameter space.
				i->Memory_Offset = Parameter_Offset + Size_of_Call_Space;
				Parameter_Offset += i->Get_Size();
			}
		}
	}
}

Variable_Descriptor::Variable_Descriptor(Node* v, int i, vector<Node*> source) {
	Define_Index = i;
	Var = v;

	vector<Node*> Linear_Ast = Linearise(v, false);
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

vector<string> Node::Get_Inheritted(bool Skip_Prefixes, bool Get_Name) {
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
			Result.push_back(Inheritted[i]);
		}
		return Result;
	}
}

string Node::Get_Inheritted(string seperator, bool Skip_Prefixes, bool Get_Name, bool skip_keywords) {
	if (MANGLER::Is_Base_Type(this) || Get_Name) {
		return seperator + Name;
	}
	else if (is(NUMBER_NODE) && Cast_Type == "") {
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
	else if (is(NUMBER_NODE) && Cast_Type != "") {
		if (Cast_Type != "")
			return seperator + Cast_Type;
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
		if (Cast_Type != "")
			result += seperator + Cast_Type;
		return result;
	}
}

Node* Node::Find_Scope(Node* n)
{
	Node* Current_Scope = n->Scope;
	vector<Node*> Fetchers = n->Get_All_Fetchers();

	reverse(Fetchers.begin(), Fetchers.end());

	while (true) {
		for (auto i : Current_Scope->Defined) {
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

Node* Node::Get_Parent_As(int F, Node* parent) {
	if (parent->is(F))
		return parent;
	if (parent->Scope != nullptr)
		return Get_Parent_As(F, parent->Scope);
	Report(Observation(ERROR, "Parent NULL!!", *Location));
	throw::runtime_error("ERROR!");
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
		for (auto i : s->Templates)
			if (i->Templates.size() == n->Templates.size())
				if (i->Name == n->Name)
					return i;

	//The feching find that finds the Scope_Path, algorithm will be start before normal search-
	//because of same named objects in the current scope.
	if (n->Fetcher != nullptr)
		if (n->Fetcher != s)
			for (auto i : Find_Scope(n)->Defined)
				if (i->Templates.size() == n->Templates.size())
					if (i->Name == n->Name)
						return n;

	if (s->Fetcher != nullptr)
		if (s->Fetcher != s->Scope)
			for (auto i : Find_Scope(s)->Defined)
				if (i->Templates.size() == n->Templates.size())
					if (i->Name == n->Name)
						return n;

	//Normal current and above going scope search algorithm
	for (Node* i : s->Defined)
		if (i->Templates.size() == n->Templates.size())
			if (i->Name == n->Name) {
				if (n->Cast_Type != "") {
					Node* tmp = i->Copy_Node(i, i->Scope);
					tmp->Cast_Type = n->Cast_Type;
					return tmp;
				}
				return i;
			}
	//If the current scope doesn't have the wanted object, then try at one spet higher scope.
	if (s->Scope != nullptr)
		if (Find(n, s->Scope) != nullptr)
			return Find(n, s->Scope);

	//IDK what this does, please explain!
	if (s->Cast_Type != "")
		for (auto i : s->Find(s->Cast_Type, s, CLASS_NODE)->Defined)
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
		for (auto i : s->Templates)
			if (i->is(n->Type))
				if (i->Templates.size() == n->Templates.size())
					if (i->Name == n->Name)
						return i;

	//The feching find that finds the Scope_Path, algorithm will be start before normal search-
	//because of same named objects in the current scope.
	if (n->Fetcher != nullptr)
		for (auto i : Find_Scope(n)->Defined)
			if (i->is(f))
				if (i->Templates.size() == n->Templates.size())
					if (i->Name == n->Name)
						return n;

	//Normal current and above going scope search algorithm
	for (Node* i : s->Defined)
		if (i->is(f))
			if (i->Templates.size() == n->Templates.size())
				if (i->Name == n->Name) {
					if (n->Cast_Type != "") {
						Node* tmp = i->Copy_Node(i, i->Scope);
						tmp->Cast_Type = n->Cast_Type;
						return tmp;
					}
					return i;
				}
	//If the current scope doesn't have the wanted object, then try at one spet higher scope.
	if (s->Scope != nullptr)
		if (Find(n, s->Scope, f) != nullptr)
			return Find(n, s->Scope, f);

	//IDK what this does, please explain!
	if (s->Cast_Type != "" && s->Cast_Type != n->Name)
			for (auto i : s->Find(s->Cast_Type, s, CLASS_NODE)->Defined)
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

	for (auto i : s->Templates)
		if (i->is(flags))
			if (i->Name == name)
				return i;

	for (Node* i : s->Defined)
		if (i->Name == name && i->is(flags))
			return i;

	if (s->Scope != nullptr)
		if (Find(name, s->Scope, flags) != nullptr)
			return Find(name, s->Scope, flags);

	if (s->Cast_Type != "" && s->Cast_Type != name)
			for (auto i : s->Find(s->Cast_Type, s, CLASS_NODE)->Defined)
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

Node* Node::Find(string name, Node* s, bool Need_Parent_existance) {
	if (name == "\n")
		return nullptr;
	if (s == nullptr /*&& Need_Parent_existance*/) {
		Report(Observation(ERROR, "Critical Error: parent is null!", *Location));
		throw::runtime_error("ERROR!");
		return nullptr;
	}

	for (auto i : s->Templates)
		if (i->Name == Name)
			return i;

	for (Node* i : s->Defined)
		if (i->Name == name)
			return i;

	if (s->Scope != nullptr)
		if (Find(name, s->Scope, Need_Parent_existance) != nullptr)
			return Find(name, s->Scope, Need_Parent_existance);

	if (s->Cast_Type != "" && s->Cast_Type != name)
			for (auto i : s->Find(s->Cast_Type, s, CLASS_NODE)->Defined)
				if (i->Name == name)
					return i;

	if (s->Fetcher != nullptr) {
		Node* F = Find_Scope(s);
		if (F != nullptr)
			if (Find(name, F, Need_Parent_existance) != nullptr)
				return Find(name, F, Need_Parent_existance);
	}
	return nullptr;
}

void Node::Get_Inheritted_Class_Members() {
	for (auto Inherit : Inheritted) {
		if (Lexer::GetComponent(Inherit).is(::Flags::KEYWORD_COMPONENT))
			continue;
		for (auto Member : Find(Inherit, Scope)->Defined)
			if (Member->is("const") == -1)
				if (Locate(Member->Name, Defined) == false)
					Defined.insert(Defined.begin(), Member);
	}
}

void Node::Update_Inheritance() {
	if (Cast_Type != "")
		return;
	//save all keyword inheritances.
	vector<string> Keyword_Inheritance;
	for (auto i : Inheritted)
		if (Lexer::GetComponents(i)[0].is(Flags::KEYWORD_COMPONENT))
			Keyword_Inheritance.push_back(i);
	//remove all inheritances
	Inheritted.clear();

	Inheritted.push_back(Cast_Type);
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

vector<Node*> Trace;
int Node::Update_Size() {
	if (is("const") != -1 && Size != 0)
		return Size;

	Trace.push_back(this);

	for (int i = 0; i < Trace.size(); i++)
		for (int j = 0; j < Trace.size(); j++)
			if (Trace[i] == Trace[j] && i != j) {
				Trace.pop_back();
				if (is("ptr") != -1 || is("func") != -1)
					return _SYSTEM_BIT_SIZE_;
				return Size;
			}


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
	Trace.pop_back();
	return Size;
}