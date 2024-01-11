#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "coreos.h"
#include "../cgen/cgen.h"
#include "ld.h"
#include "../utils.h"
#include "mnemonic.h"
#include "../GLOBALS.h"

void storeContext(memmory* ram);
void loadContext(memmory* ram);

int osCall(quad* fun, memmory* ram, stack* params){
	int fp, reg, reg2, nlit, idx;
	char c;

	if(strcmp(fun->arg2, INPUTF) == 0){
		reg = getN(fun->arg1);
		newInstruction(ram, mvi, oa, INPUT_ADDR);
		newInstruction(ram, get, oa, 0, 0);
		newInstruction(ram, lw, oa, 0, 0);
		newInstruction(ram, mv, reg, oa, 0);
	}
	else if(strcmp(fun->arg2, OUTPUTF) == 0){
		reg = getN(popStack(&params));
		newInstruction(ram, mvi, oa, OUTPUT_ADDR);
		newInstruction(ram, sw, oa, reg, 0);
		newInstruction(ram, print, oa, 0, 0);
	}
	else if(strcmp(fun->arg2, ISDIRF) == 0){
		fp = getN(popStack(&params));
		reg = getN(fun->arg1);
		newInstruction(ram, mv, oa, fp, 0);
		newInstruction(ram, lwHD, oa, oa, 0);
		newInstruction(ram, shiftR, oa, 0, 0, 7);
		newInstruction(ram, ANDi, oa, 1);
		newInstruction(ram, mv, reg, oa, 0);
	}
	else if(strcmp(fun->arg2, ISACTIVEF) == 0){
		fp = getN(popStack(&params));
		reg = getN(fun->arg1);
		newInstruction(ram, mv, oa, fp, 0);
		newInstruction(ram, lwHD, oa, oa, 0);
		newInstruction(ram, shiftR, oa, 0, 0, 6);
		newInstruction(ram, ANDi, oa, 1);
		newInstruction(ram, mv, reg, oa, 0);
	}
	else if(strcmp(fun->arg2, GETBYTEF) == 0){
		fp = getN(popStack(&params));		// addr
		reg = getN(popStack(&params));		// target
		reg2 = getN(fun->arg1);				// return

		newInstruction(ram, lwHD, reg, fp, 0);
		newInstruction(ram, mv, reg2, reg, 0);
	}
	else if(strcmp(fun->arg2, GETNFILESF) == 0){
		fp = getN(popStack(&params));		// fp
		reg = getN(fun->arg1);
		newInstruction(ram, mv, oa, fp, 0);
		newInstruction(ram, lwHD, oa, oa, 2);
		newInstruction(ram, ANDi, oa, 255);
		newInstruction(ram, mv, reg, oa, 0);
	}
	else if(strcmp(fun->arg2, SHIFTLBYTEF) == 0){
		reg = getN(popStack(&params));		//val
		reg2 = getN(fun->arg1);
		newInstruction(ram, mv, oa, reg, 0);
		newInstruction(ram, shiftL, oa, 0, 0, 8);
		newInstruction(ram, mv, reg2, oa, 0);
	}
	else if(strcmp(fun->arg2, SHIFTRBYTEF) == 0){
		reg = getN(popStack(&params));		//val
		reg2 = getN(fun->arg1);
		newInstruction(ram, mv, oa, reg, 0);
		newInstruction(ram, shiftR, oa, 0, 0, 8);
		newInstruction(ram, mv, reg2, oa);
	}
	else if(strcmp(fun->arg2, GETADDRF) == 0){
		reg = getN(popStack(&params));		// addr begin
		reg2 = getN(fun->arg1);

		newInstruction(ram, mv, ra1$, reg, 0);
		newInstruction(ram, lwHD, oa, ra1$, 0); 	// hd to reg[24:36]
		newInstruction(ram, shiftL, oa, 0, 0, 8);	// shift left
		newInstruction(ram, lwHD, oa, ra1$, 1); 	// hd to reg
		newInstruction(ram, shiftL, oa, 0, 0, 8);	// shift left
		newInstruction(ram, lwHD, oa, ra1$, 2); 	// hd to reg
		newInstruction(ram, shiftL, oa, 0, 0, 8);	// shift left
		newInstruction(ram, lwHD, oa, ra1$, 3); 	// hd to reg

		newInstruction(ram, mv, reg2, reg, 0);
	}
	else if(strcmp(fun->arg2, GETSIZENAMEF) == 0){
		fp = getN(popStack(&params));		// fp
		reg = getN(fun->arg1);

		newInstruction(ram, mv, ra1$, fp, 0);
		newInstruction(ram, mvi, oa, 0);
		newInstruction(ram, lwHD, oa, ra1$, 1); 	// hd to reg[24:36]

		newInstruction(ram, mv, reg, oa, 0);
	}
	else if(strcmp(fun->arg2, GETSIZEPAYLOADF) == 0){
		fp = getN(popStack(&params));		// fp
		reg = getN(fun->arg1);

		newInstruction(ram, mv, ra1$, fp, 0);
		newInstruction(ram, mvi, oa, 0);
		newInstruction(ram, lwHD, oa, ra1$, 2); 	// hd to reg[24:36]
		newInstruction(ram, shiftL, oa, 0, 0, 8);
		newInstruction(ram, lwHD, oa, ra1$, 3);

		newInstruction(ram, mv, reg, oa, 0);
	}
	else if(strcmp(fun->arg2, DISPLAYBYTEF) == 0){
		reg = getN(popStack(&params));

		newInstruction(ram, display, reg, 0, 0);
	}
	else if(strcmp(fun->arg2, WRITEINSTRUCTIONF) == 0){
		reg = getN(popStack(&params));		// MI addr
		reg2 = getN(popStack(&params));		// instruction

		newInstruction(ram, swMI, reg, reg2, 0);	// write here to MI
	}

	else if(strcmp(fun->arg2, RUNF) == 0){
		// Jump addres to specific pc
		nlit = getN(popStack(&params));
		reg = getN(popStack(&params));
		
		newInstruction(ram, mvi, oa, nlit);
		newInstruction(ram, sb, oa, 0, 0);
		newInstruction(ram, jal, reg, 0, 0);
	}
	
	else if(strcmp(fun->arg2, SETBASISF) == 0){
		// new base val
		nlit = getN(popStack(&params));
		newInstruction(ram, mvi, oa, nlit);
		newInstruction(ram, sb, oa, 0, 0);
	}
	
	else if(strcmp(fun->arg2, RUNCHRONOF) == 0){
		// addr
		nlit = getN(popStack(&params));		// basis literal
		reg = getN(popStack(&params));		// Addr
		reg2 = getN(fun->arg1);

		// Addr saved to rr$
		newInstruction(ram, mv, rr$, reg, 0);

		// Store SO context
		storeContext(ram);

		// Update basis
		newInstruction(ram, mvi, oa, nlit);
		newInstruction(ram, sb, oa, 0, 0);

		// get first data
		newInstruction(ram, mvi, oa, VISITOR_FLAG_ADDR);
		newInstruction(ram, lw, oa, 0, 0);	// First position

		newInstruction(ram, eqi, oa, 1);
		newInstruction(ram, bc, 0, 0, 2);	// IF is not first time

		newInstruction(ram, eqi, oa, 0);
		newInstruction(ram, bc, 0, 0, 62);	// Else IF 

		/*
		newInstruction(ram, eqi, oa, 2);	
		newInstruction(ram, bc, 0, 0, );	// ELSE IF
		*/

		// IF CONTENT
		loadContext(ram);
		newInstruction(ram, branch, 0, 0, 1);

		// ELSE IF content
		newInstruction(ram, mvi, sp, 0);

		// AFTER 

		// salva oa em sp+1
		newInstruction(ram, sw, sp, oa, 1);
		
		//salva ra1 em sp+2
		newInstruction(ram, sw, sp, ra1$, 2);

		newInstruction(ram, mvi, oa, VISITOR_FLAG_ADDR);
		newInstruction(ram, mvi, ra1$, 1);
		newInstruction(ram, sw, oa, ra1$, 0);

		//load oa em sp+1
		newInstruction(ram, mv, oa, sp, 0);
		newInstruction(ram, lw, oa, 0, 1);

		//load ra1 em sp+2
		newInstruction(ram, mv, ra1$, sp, 0);
		newInstruction(ram, lw, ra1$, 0, 2);

		newInstruction(ram, jt, rt$, rr$, 0);	// goes to rr$ and back to rt$. When back, saves to rr$
		newInstruction(ram, subi, rr$, 1);		// correction of jt

		// Store Context
		storeContext(ram);

		// Update basis
		newInstruction(ram, mvi, oa, 0);
		newInstruction(ram, sb, oa, 0, 0);

		// load SO Context
		loadContext(ram);

		newInstruction(ram, mv, reg2, rr$, 0);
	}

	else if(strcmp(fun->arg2, HASFINISHEDF) == 0){
		reg2 = getN(popStack(&params));
		reg = getN(fun->arg1);

		newInstruction(ram, sb, reg2, 0, 0);
		newInstruction(ram, mvi, reg, FINISH_FLAG_ADDR);
		newInstruction(ram, lw, reg, 0, 0);
		newInstruction(ram, mvi, oa, 0);
		newInstruction(ram, sb, oa, 0, 0);
	}
	else if(strcmp(fun->arg2, PRINTF) == 0){
		idx = 0;
		c = fun->arg1[idx];
		while(c != '\0'){
			if(c == '\\'){
				continue;
				// TODO: impement here moderators
			}
			newInstruction(ram, mvi, oa, (int)c);
			newInstruction(ram, display, oa, 0, 0);

			c = fun->arg1[++idx];
		}
	}
	else{
		return 0;
	}
	return 1;
}

void storeContext(memmory* ram){
	int desl;
	desl = 0;
	
	newInstruction(ram, addi, sp, ntemps);

	for (int reg = (ntemps - 1); reg >= 0; reg--){
		newInstruction(ram, sw, sp, reg, -(desl));
		desl++;
	}
	newInstruction(ram, addi, sp, 7);
	newInstruction(ram, sw, sp, ra2$, -6);
	newInstruction(ram, sw, sp, ra1$, -5);
	newInstruction(ram, sw, sp, rd, -4);
	newInstruction(ram, sw, sp, oa, -3);
	newInstruction(ram, sw, sp, fp$, -2);
	newInstruction(ram, sw, sp, rf, -1);
	newInstruction(ram, sw, sp, rj, 0);

	newInstruction(ram, mvi, oa, SP_ADDR);
	newInstruction(ram, sw, oa, sp, 0);		// Store sp
}

void loadContext(memmory* ram){
	int desl;
	desl = 0;
	
	newInstruction(ram, mvi, sp, SP_ADDR);
	newInstruction(ram, lw, sp, 0, 0);		// Load sp

	newInstruction(ram, mv, rj, sp, 0);
	newInstruction(ram, lw, rj, 0, 0);

	newInstruction(ram, mv, rf, sp, 0);
	newInstruction(ram, lw, rf, 0, -1);

	newInstruction(ram, mv, fp$, sp, 0);
	newInstruction(ram, lw, fp$, 0, -2);

	newInstruction(ram, mv, oa, sp, 0);
	newInstruction(ram, lw, oa, 0, -3);

	newInstruction(ram, mv, rd, sp, 0);
	newInstruction(ram, lw, rd, 0, -4);

	newInstruction(ram, mv, ra1$, sp, 0);
	newInstruction(ram, lw, ra1$, 0, -5);

	newInstruction(ram, mv, ra2$, sp, 0);
	newInstruction(ram, lw, ra2$, 0, -6);

	desl = 7;

	for (int reg = (ntemps - 1); reg >= 0; reg--){
		newInstruction(ram, mv, reg, sp, 0);
		newInstruction(ram, lw, reg, 0, -(desl));
		desl++;
	}

	newInstruction(ram, subi, sp, (ntemps + 7));
}