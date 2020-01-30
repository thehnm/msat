#!/usr/bin/env python

import sys

def read_text_file (fname):
    with open(fname) as f:
        content = f.readlines()
    return [x.strip() for x in content] 

def read_DIMACS (fname):
    content=read_text_file(fname)

    header=content[0].split(" ")

    assert header[0]=="p" and header[1]=="cnf"
    variables_total, clauses_total = int(header[2]), int(header[3])

    # array idx=number (of line) of clause
    # val=list of terms
    # term can be negative signed integer
    clauses=[]
    for c in content[1:]:
        if c.startswith ("c "):
            continue
        clause=[]
        for var_s in c.split(" "):
            var=int(var_s)
            if var!=0:
                clause.append(var)
        clauses.append(clause)

    if clauses_total != len(clauses):
        print "warning: header says ", clauses_total, " but read ", len(clauses)
    return variables_total, clauses

# variable numbers and corresponding literal numbers (the last bit is 1 for a positive variable and 0 for a negative variable):
# -1 : 0
# 1  : 1
# -2 : 2
# 2  : 3
# -3 : 4
# 3  : 5
# etc

def var2lit(var):
    if var<0:
        return (abs(var)-1)*2
    else:
        return (abs(var)-1)*2+1

variables_total, clauses = read_DIMACS(sys.argv[1])

literals_total = variables_total*2

# key = literal
# val = watched list of clauses
literals={}
for l in range(literals_total):
    literals[l]=[]

# set initial watched lists by attaching first variable of each clause to the corresponding literal
# first literal/variable of each clause is the default watchee
c_no=0
for clause in clauses:
    first_var=clause[0]
    l=var2lit(first_var)
    literals[l].append(c_no)
    c_no=c_no+1

# array of choices, 0 for False, 1 for True
move=[]

# if the variable is already assigned?
def var_already_assigned_to_false (var):
    global move
    return move[abs(var)-1]==0

def var_already_assigned_to_true (var):
    global move
    return move[abs(var)-1]==1

# find a place for a clause to reattach it
# Change watchee
def find_better_place_for_clause(level, c_no):
    global clauses, move, literals, variables_total
    if len(clauses[c_no])==1: # unit clause? we can't do anything
        return False
    # enumerate all literals/variables in clause except first:
    for v in range(1, len(clauses[c_no])):
        var=clauses[c_no][v]
        # this variable/literal isn't yet assigned
        # so we can "postpone" it for future
        if (abs(var)-1) > level:
            # reattach clause:
            literals[var2lit(var)].append(c_no)
            # swap first variable/literal in clause with newly watchee
            clauses[c_no][v], clauses[c_no][0] = clauses[c_no][0], clauses[c_no][v]
            return True

        # this variable/literal is already assigned and it's value coincides with our variable:
        cond1 = var_already_assigned_to_false(var) and var<0
        cond2 = var_already_assigned_to_true(var) and var>0
        if cond1 or cond2:
            # reattach clause:
            literals[var2lit(var)].append(c_no)
            # swap first variable/literal in clause with newly watchee
            clauses[c_no][v], clauses[c_no][0] = clauses[c_no][0], clauses[c_no][v]
            return True

    return False

# "disembowel" a watch list by reattaching all clauses in WL to other literals/watch lists:
def reconnect_all_clauses(level):
    global clauses, move, literals, variables_total
    # which literal to "disembowel"?
    if move[level]==0:
        lit=level*2+1
    else:
        lit=level*2
    # list of clauses in WL to "disembowel":
    tmp=literals[lit][:]
    for i in range(len(tmp)):
        if find_better_place_for_clause(level, tmp[i]):
            # delete first element. maybe slow?
            literals[lit]=literals[lit][1:]
        else:
            # there is a contradicted clause we can't reattach, so return False
            return False
    # all clauses from WL has been reconnected, proceed further
    return True

def try_bits(level):
    #print "try_bit(", level, ")"
    global clauses, move, literals, variables_total
    #print move
    if level==variables_total:
    # all variables are assigned, and we are here, hence, all satisfied, so we stop here
        print "SAT"
        s=""
        for i in range(variables_total):
            if move[i]==1:
                s=s+str(i+1)+" "
            else:
                s=s+"-"+str(i+1)+" "
        print s
        exit(0)
    # if the "positive" literal has a watch list attached, start with True, then switch to False
    # because there is a chance after backtrack that "negative" literal has no watch list attached, so why do unneeded work
    # by reattaching?
    # otherwise, start with False and switch to True
    if len(literals[level*2+1])>0:
        first_alternative, second_alternative = 1,0
    else:
        first_alternative, second_alternative = 0,1

    move.append(first_alternative)
    # try to reconnect all clauses
    # proceed deeper in case of success
    if reconnect_all_clauses(level):
        try_bits(level+1)

    # no success, remove first_alternative from move[], proceed to the second one:
    move=move[:-1]
    move.append(second_alternative)
    if reconnect_all_clauses(level):
        try_bits(level+1)
    move=move[:-1]

    # no luck with both alternatives, so we backtrack by returning from recursive call
    # if there is no place to return (level=0), the instance cannot be satisfied:
    if level==0:
        print "UNSAT"
        exit(0)

try_bits(level=0)
