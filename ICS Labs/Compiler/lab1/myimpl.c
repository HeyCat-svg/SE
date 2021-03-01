#include "prog1.h"

int maxargs(A_stm stm){
	//TODO: put your code here.
	int static max = -1;
	int type = stm->kind;
	if(type == A_compoundStm){
		maxargs(stm->u.compound.stm1);
		maxargs(stm->u.compound.stm2);
	}
	else if(type == A_assignStm){
		A_exp p = stm->u.assign.exp;
		while(p->kind == A_eseqExp){
			maxargs(p->u.eseq.stm);
			p = p->u.eseq.exp;
		}
	}
	else{	// printStm
		A_expList p = stm->u.print.exps;
		int local_max = 0;
		while(1){
			if(p->kind == A_lastExpList){	// lastExpList
				local_max += 1;
				A_exp p1 = p->u.last;
				while(p1->kind == A_eseqExp){
					maxargs(p1->u.eseq.stm);
					p1 = p1->u.eseq.exp;
				}
				break;
			}
			else{	// pairExpList
				local_max += 1;
				A_exp p1 = p->u.pair.head;
				while(p1->kind == A_eseqExp){
					maxargs(p1->u.eseq.stm);
					p1 = p1->u.eseq.exp;
				}
				p = p->u.pair.tail;
			}
		}
		if(local_max > max){
			max = local_max;
		}
	}
	return max;
}

typedef struct table  *Table_;
struct table{
  string id;
  int value;
  Table_ tail;
};
typedef struct intAndTable_ *intAndTable;
struct intAndTable_{
    int i;
    Table_ table;
};
Table_ interStm(A_stm stm, Table_ t);
intAndTable interExp(A_exp exp, Table_ t);
intAndTable interExpList(A_expList expList, Table_ t);
Table_ update(Table_, string, int);
int lookup(Table_, string);

Table_ Table(string id,  int value, Table_ *tail){
    Table_ t = malloc(sizeof(*t));
    t->id = id;
    t->value = value;
    t->tail = t;
    return t;
}

Table_ interStm(A_stm stm, Table_ t){
    if (stm == NULL ){
        return NULL;
    }
    Table_ table;
    intAndTable itable;
    A_expList tmpExplist;
   	switch(stm->kind){
   		case A_compoundStm:
     		table = interStm(stm->u.compound.stm1, t);
     		return interStm(stm->u.compound.stm2, table);
   		case A_printStm:
       		tmpExplist = stm->u.print.exps;
       		while(1){
           		if (tmpExplist->kind == A_lastExpList){
               		itable = interExp(tmpExplist->u.last, t);
               		printf("   %d", itable->i);
               		break;
           		}
           		else{
               		itable = interExp(tmpExplist->u.pair.head, t);
               		printf("  %d", itable->i);
               		tmpExplist = tmpExplist->u.pair.tail;
           		}
       		}
     		printf("\n" ) ;
     		return itable->table;
   		case A_assignStm:
       		itable = interExp(stm->u.assign.exp, t);
       		table = update(itable->table, stm->u.assign.id, itable->i);
       		return table;
   }
   return NULL;
}


intAndTable interExpList(A_expList expList, Table_ t){
    if (expList == NULL){
        return NULL;
    }
    intAndTable tmp;
    switch(expList->kind){
    case A_lastExpList:
     	tmp = interExp(expList->u.last, t);
     	return tmp;
    case A_pairExpList:
        tmp = interExp(expList->u.pair.head, t);
        tmp = interExpList(expList->u.pair.tail, tmp->table);
        return tmp;
    }
    return NULL;
}

intAndTable interExp(A_exp exp, Table_ t){
    if ( exp == NULL ){
        return NULL;
    }
    intAndTable t1 = malloc(sizeof(*t));
    int tmp;
   	switch(exp->kind){
   		case A_idExp:
     		t1->i = lookup(t, exp->u.id);
     		t1->table = t;
     		return t1;
   		case A_numExp:
       		t1->i = exp->u.num;
       		t1->table = t;
       		return t1;
   		case A_opExp:
     	 	t1 = interExp(exp->u.op.left, t);
      		tmp = t1->i;
      		t1 = interExp(exp->u.op.right, t1->table);
      		switch(exp->u.op.oper){
      			case A_plus:
         			tmp += t1->i;
          			break;
      			case A_minus:
          			tmp -= t1->i;
          			break;
      			case A_times:
          			tmp *= t1->i;
          			break;
      			case A_div:
          			tmp /= t1->i;
          			break;
      		}
      		t1->i = tmp;
      		return t1;
   		case A_eseqExp:
        	t = interStm(exp->u.eseq.stm, t);
        	t1 = interExp(exp->u.eseq.exp, t);
        	return t1;
	}
   	return NULL;
}

Table_ update(Table_ t, string s, int v){
    Table_ t1 = malloc(sizeof(*t1));
    t1->id = s;
    t1->tail = t;
    t1->value = v;
    return t1;
}

int lookup(Table_ t, string c){
    Table_ tmp = t;
    while(tmp){
        if (tmp->id == c){
            return tmp->value;
        }
        tmp = tmp->tail;
    }
    return -1;
}


void interp(A_stm stm) {
	//TODO: put your code here.
	Table_ t = NULL;
	interStm(stm, t);
	
}
