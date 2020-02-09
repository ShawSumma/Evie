#include "..\..\..\H\Architecture\x86\x86.h"
#include "../../../H/Back/Token.h"


void x86::ARC_Factory()
{
	OpC* x86_ADD_ = new OpC(0, "+", "add");
	OpCodes.push_back(x86_ADD_);
	OpC* x86_OR_ = new OpC(0, "|", "or");
	OpCodes.push_back(x86_OR_);
	OpC* x86_AND_ = new OpC(0, "&", "and");
	OpCodes.push_back(x86_AND_);
	OpC* x86_XOR_ = new OpC(0, "^", "xor");
	OpCodes.push_back(x86_XOR_);
	OpC* x86_CMP_ = new OpC(0, "?", "cmp");
	OpCodes.push_back(x86_CMP_);
	OpC* x86_POP_ = new OpC(0, "pop", "pop");
	OpCodes.push_back(x86_POP_);
	OpC* x86_PUSH_ = new OpC(0, "push", "push");
	OpCodes.push_back(x86_PUSH_);
	OpC* x86_XCHG_ = new OpC(0, "xchg", "xchg");
	OpCodes.push_back(x86_XCHG_);
	OpC* x86_LOAD_ = new OpC(0, "=", "mov");
	OpCodes.push_back(x86_LOAD_);
	OpC* x86_STORE_ = new OpC(0, "=", "mov");
	OpCodes.push_back(x86_STORE_);
	OpC* x86_LEA_ = new OpC(0, "lea", "lea");
	OpCodes.push_back(x86_LEA_);
	OpC* x86_SHL_ = new OpC(0, "<<", "shl");
	OpCodes.push_back(x86_SHL_);
	OpC* x86_SHR_ = new OpC(0, ">>", "shr");
	OpCodes.push_back(x86_SHR_);
	OpC* x86_NEG_ = new OpC(0, "neg", "neg");
	OpCodes.push_back(x86_NEG_);
	OpC* x86_NOT_ = new OpC(0, "!", "not");
	OpCodes.push_back(x86_NOT_);
	OpC* x86_CALL_ = new OpC(0, "call", "call");
	OpCodes.push_back(x86_CALL_);
	OpC* x86_SUB_ = new OpC(0, "-", "sub");
	OpCodes.push_back(x86_SUB_);
	OpC* x86_IMUL_ = new OpC(0, "*", "imul");
	OpCodes.push_back(x86_IMUL_);
	OpC* x86_IDIV_ = new OpC(0, "*", "idiv");
	OpCodes.push_back(x86_IDIV_);
	OpC* x86_JNE_ = new OpC(0, "==", "jne");
	OpCodes.push_back(x86_JNE_);
	OpC* x86_JE_ = new OpC(0, "!=", "je");
	OpCodes.push_back(x86_JE_);
	OpC* x86_JNLE_ = new OpC(0, "<=", "jnle");
	OpCodes.push_back(x86_JNLE_);
	OpC* x86_JNGE_ = new OpC(0, ">=", "jnge");
	OpCodes.push_back(x86_JNGE_);
	OpC* x86_JLE_ = new OpC(0, "!<", "jle");
	OpCodes.push_back(x86_JLE_);
	OpC* x86_JGE_ = new OpC(0, "!>", "jge");
	OpCodes.push_back(x86_JGE_);
	OpC* x86_JNL_ = new OpC(0, "<", "jnl");
	OpCodes.push_back(x86_JNL_);
	OpC* x86_JNG_ = new OpC(0, ">", "jng");
	OpCodes.push_back(x86_JNG_);
	OpC* x86_JMP_ = new OpC(0, "jmp", "jmp");
	OpCodes.push_back(x86_JMP_);
	OpC* x86_RET_ = new OpC(0, "ret", "ret");
	OpCodes.push_back(x86_RET_);


	EAX = new Register("EAX", 4, "r1");
	EAX->Flags |= Task_For_Returning;
	Registers.push_back(EAX);
	EBX = new Register("EBX", 4, "r2");
	Registers.push_back(EBX);
	ECX = new Register("ECX", 4, "r3");
	Registers.push_back(ECX);
	EDX = new Register("EDX", 4, "r4");
	Registers.push_back(EDX);
	EDI = new Register("EDI", 4, "r5");
	Registers.push_back(EDI);
	ESI = new Register("ESI", 4, "r6");
	Registers.push_back(ESI);
	ESP = new Register("ESP", 4, "r7");
	ESP->Flags |= Task_For_Type_Address;
	Registers.push_back(ESP);
	EBP = new Register("EBP", 4, "r8");
	EBP->Flags |= Task_For_Type_Address_Basing;
	Registers.push_back(EBP);
	//16Bit
	AX = new Register("AX", 2, "r9");
	Registers.push_back(AX);
	BX = new Register("BX", 2, "r10");
	Registers.push_back(BX);
	CX = new Register("CX", 2, "r11");
	Registers.push_back(CX);
	DX = new Register("DX", 2, "r12");
	Registers.push_back(DX);
	DI = new Register("DI", 2, "r13");
	Registers.push_back(DI);
	SI = new Register("SI", 2, "r14");
	Registers.push_back(SI);
	SP = new Register("SP", 2, "r15");
	Registers.push_back(SP);
	BP = new Register("BP", 2, "r16");
	Registers.push_back(BP);
	//8Bit
	AL = new Register("AL", 1, "r17");
	Registers.push_back(AL);
	AH = new Register("AH", 1, "r18");
	Registers.push_back(AH);
	BL = new Register("BL", 1, "r19");
	Registers.push_back(BL);
	BH = new Register("BH", 1, "r20");
	Registers.push_back(BH);
	CL = new Register("CL", 1, "r21");
	Registers.push_back(CL);
	CH = new Register("CH", 1, "r22");
	Registers.push_back(CH);
	DL = new Register("DL", 1, "r23");
	Registers.push_back(DL);
	DH = new Register("DH", 1, "r24");
	Registers.push_back(DH);

}
