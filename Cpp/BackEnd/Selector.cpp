#include "../../H/BackEnd/Selector.h"
#include "../../H/UI/Usr.h"

extern Usr* sys;
extern x86_64_Win X86_64_WIN;

void Selector::Init() {
	if (sys->Info.Architecture == "x86") {
		if (sys->Info.Bits_Mode == "8") {
			if (sys->Info.OS == "win32") {
				X86_64_WIN.Init();
				for (auto i : X86_64_WIN.Registers)
					Registers.push_back({ nullptr, i });
				for (auto i : X86_64_WIN.Parameter_Registers) {
					Parameter_Registers.push_back(Transform(i));
				}
			}
		}

	}
}

vector<Token*> Selector::Transform(Token* parent)
{
	vector<Token*> Result;
	for (auto c : parent->Get_Childs()) {
		Result.push_back(c);
		vector<Token*> r = Transform(c);
		Result.insert(Result.end(), r.begin(), r.end());
	}
	return Result;
}

Path* Selector::Get_Path_Info(vector<IR*> source, int i, Token* t) {
	Path* p = new Path;
	//try to find if this t* is used in future
	//also check if it intersects cllations
	vector<int> Intersects_Calls;
	int Parameter_Place = -1;
	int Last_Usage = i;
	for (int j = i; j < source.size(); j++) {
		for (auto k : source[j]->Arguments)
			//TODO: Cheking names wont work on duplicated local varibles inside a condition like scopes.
			if (k->Get_Name() == t->Get_Name())
				//check if this is the same variable...
				Last_Usage = j;
		if (source[j]->is(TOKEN::CALL)) {
			Intersects_Calls.push_back(j);
			for (int k = 0; k < source[j]->Arguments.size(); k++)
				if (source[j]->Arguments[k]->Get_Name() == t->Get_Name())
					Parameter_Place = k;
		}
		if (source[j]->is(TOKEN::END_OF_FUNCTION))
			break;
	}
	p->Intersects_Calls = Intersects_Calls;
	p->Last_Usage = Last_Usage;
	p->Parameter_Place = Parameter_Place;
	return p;
}

void Selector::Make_Solution_For_Crossed_Register_Usages(pair<Register_Descriptor*, Token*> Current, pair<Register_Descriptor*, Token*> New, vector<IR*>* source, int i)
{
	/*
	mov rbx, [a]
	;try to find non-volatile
	;if no non_volatile then use stack
	;inform stack about the new push
	add rbx, [c]	//a += c
	mov rcx, [b]
	call banana(b)
	mov rcx, rbx
	call banana(a)
	*/
	//get a new non-volatile register to use
	Token* non_volatile = Get_Register(TOKEN::NONVOLATILE, New.first);
	if (non_volatile == nullptr) {
		//if this happends then all registers are in use and we need to use stack
	}

	//now we need to give the cuurent user the new non-volatile register.
	//TODO: Because im a peace of shit and dont know how to implement LIVE register change ill have to make a new move IR
	IR* MOV = new IR(new Token(TOKEN::OPERATOR, "move"), {non_volatile, Current.second});

	//insert the mov before the new user starts
	source->insert(source->begin() + New.first->First_Usage_Index, MOV);

	//the load back is handled by the situation system, call, etc..

	//now we need to give the current user the new non-volatile
	//if the non-volatile area is a memory then no need to pair
	if (!non_volatile->is(TOKEN::MEMORY))
		Pair_Up(non_volatile, Current.first);

	//break the old connection between the register and the user
	Break_Up(Current.second);
}

Token* Selector::Get_New_Reg(vector<IR*>* source, int i, Token* t)
{
	Path* p = Get_Path_Info(*source, i, t);
	if (p->Parameter_Place != -1) {
		//TODO: this can go broke if there is one or more calls before this so the parameter chosen by this is going to be overwritten :(
		Token* reg = nullptr;
		for (auto k : Parameter_Registers[p->Parameter_Place])
			if (k->Get_Size() == t->Get_Size())
				reg = k;


		for (auto r : Registers) {
			if (r.second->Get_Name() != reg->Get_Name())
				//skip until we get into the right register
				continue;
			if (r.first == nullptr) {
				//no current owner
				r.first->First_Usage_Index = i;
				r.first->Last_Usage_Index = p->Last_Usage;
				r.first->User = t->Get_Name();
				return r.second;
			}
			else if (r.first->Last_Usage_Index > p->Last_Usage){
				//if the current register user is more superior register user than us, then we need to do a temporary save to stack 
				Make_Solution_For_Crossed_Register_Usages({ r.first, r.second }, { new Register_Descriptor(i, p->Last_Usage, t->Get_Name()), t }, source, i);
				//now the wanted register is solutioned :D
				return r.second;
			}
		}
	}

	long Reg_Type = 0;
	if ((p->Intersects_Calls.size() > 0) && p->Intersects_Calls.back() < p->Last_Usage)
		Reg_Type |= TOKEN::NONVOLATILE;
	else
		Reg_Type |= TOKEN::VOLATILE;

	for (auto r : Registers) {
		if (r.first == nullptr) {
			if (r.second->is(Reg_Type)) {
				if (r.second->Get_Size() == t->Get_Size()) {
					r.first->Last_Usage_Index = p->Last_Usage;
					r.first->User = t->Get_Name();
					return r.second;
				}
			}
		}
		else if (r.first->Last_Usage_Index < i){
			if (r.second->is(Reg_Type)) {
				if (r.second->Get_Size() == t->Get_Size()) {
					r.first->Last_Usage_Index = p->Last_Usage;
					r.first->User = t->Get_Name();
					return r.second;
				}
			}
		}
		//still... checking names is wrong
		else if (r.first->User == t->Get_Name()) {
			return r.second;
		}
	}
	//if the code gets here, then return nullptr to indicate that we need to release some registers
	return nullptr;
}

Token* Selector::Get_Register(Token* t)
{
	for (auto i : Registers)
		if (i.first->User == t->Get_Name())
			return i.second;
	return nullptr;	//need to use Get_New_Reg();
}

Token* Selector::Get_Register(long F, Register_Descriptor* user)
{
	for (auto i : Registers) {
		if (!i.second->is(F))
			continue;
		if (i.first == nullptr || (i.first->Last_Usage_Index <= user->First_Usage_Index)) {
			i.first->First_Usage_Index = user->First_Usage_Index;
			i.first->Last_Usage_Index = user->Last_Usage_Index;
			i.first->User = user->User;
			return i.second;
		}
	}
	//if this gets here, all non & non-non-volatile register are used already.
	//use stack
	return nullptr;
}

Token* Selector::Allocate_Register(vector<IR*>& source, int i, Token* t)
{
	
}

void Selector::Pair_Up(Token* r, Register_Descriptor* t)
{
	for (auto i : Registers)
		if (i.second->Get_Name() == r->Get_Name()) {
			i.first = t;
			return;
		}
}

void Selector::Break_Up(Token* r)
{
	for (auto i : Registers)
		if (i.second->Get_Name() == r->Get_Name()) {
			i.first = nullptr;
			return;
		}
}
