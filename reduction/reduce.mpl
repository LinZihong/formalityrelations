# Erik Panzer
# 19 August 2022
# solve formality relations, to update reduction table
interface(errorbreak=2):

if FileTools[Exists]("reduction.m") then
	read "reduction.m":
else
	w:=table():
	w[[[]]]:=1:
	w[[]]:=1:
#	w[[[p1]]]:=1:
end if:

if (type(n,nonnegint) and type(m,nonnegint)) then
	fName := cat("../FormalityRelations/rel",n+m,"_",m,"_",2*n+m-3):
	if not FileTools[Exists](fName) then
		fName := cat("../graph_generation_linz/formality relations/relations_",n,"_",m,"_",2*n+m-3,".rel"):
	end if:
elif not assigned(fName) then
	error "specify n and m, or filename (fName)":
end if:

printf("Reading relations from file named: %s\n", fName):

if not FileTools[Exists](fName) then
	error "relations not found":
end if:

isworse := proc(a,b)
local na,nb,ma,mb,ea,eb,stara,starb,wa,wb:
	ea := nops(map(op,a)):
	eb := nops(map(op,b)):
	na := nops(a):
	nb := nops(b):
	ma := ea+2-2*na:
	mb := eb+2-2*nb:
	# make sure that m=2 is considered easiest
	if (ma=2) and (mb<>2) then
		return false:
	elif (ma<>2) and (mb=2) then
		return true:
	end if:
	# if both are m=2, prefer star product graphs (outdegree 2 at each internal vertex)
	if (ma=2) and (mb=2) then
		stara := evalb(map(nops,a)=[2$na]):
		starb := evalb(map(nops,b)=[2$nb]):
		if stara and (not starb) then
			return false:
		elif starb and (not stara) then
			return true:
		end if:
	end if:
	# higher weight is worse:
	wa := `if`(ma=0,na-1,na+ma-2):
	wb := `if`(mb=0,nb-1,nb+mb-2):
	if wa>wb then
		return true:
	elif wa<wb then
		return false:
	end if:
	# more edges is worse:
	if ea>eb then
		return true:
	elif ea<eb then
		return false:
	# more n is worse:
	elif na>=nb then
		return true:
	else
		return false:
	end if:
end proc:

wInExpr := proc(expr)
	return map(op, select(q->op(0,q)=w, indets(expr))):
end proc:

# For faster back-substitution:
# Table to store the reductions containing a given free parameter
InitUsedIn := proc()
global UsedIn:
global w:
local left,q:
	UsedIn := table():
	for left in indices(w,nolist) do
#		w[left] := expand(w[left]):
		for q in wInExpr(w[left]) do
			if not assigned(UsedIn[q]) then
				UsedIn[q] := table():
			end if:
			UsedIn[q][numelems(UsedIn[q])+1] := left:
		end do:
	end do:
	for left in indices(UsedIn,nolist) do
		UsedIn[left] := {entries(UsedIn[left],nolist)}:
	end do:
end proc:
InitUsedIn():

ndone := 0:
nsolved := 0:
Warnlines := {}:
while true do
	rel := readline(fName):
	if rel=0 then
		break:
	end if:
	ndone := ndone+1:
	bprint := (ndone<100) or (ndone mod 100 = 0):
	rel := parse(rel):
	if bprint then
		printf("relation in line %d:\n", ndone):
		lprint(rel):
	end if:
	rel := eval(rel, W=(q->w[q])):
	rel := expand(rel):
	left := indets(rel):
	left := select(q->op(0,q)=w, left):
	if nops(left)=0 then
		if rel=0 then
			next:
		else
			printf("line %d\n", ndone):
			error "relation violated":
		end if:
	end if:
	# pick 'worst' term
	left := [op(left)]:
	left := map(op,left):
	left := sort(left, isworse):
	left := w[left[1]]:
	# solve
	if bprint then
		printf("solving for %a\n",left):
	end if:
	if degree(rel,left)>1 then
		WARNING("non-linear relation in line %1", ndone):
		Warnlines := Warnlines union {ndone}:
		next:
	end if:
	if indets(coeff(rel,left,1))<>{} then
		WARNING("leading coefficient not constant in line %1", ndone):
		Warnlines := Warnlines union {ndone}:
		next:
	end if:
	rel := -coeff(rel,left,0)/coeff(rel,left,1):
	rel := expand(rel):
	if bprint then
		print(left=rel):
	end if:

	# update substitutions
	left := op(left):
	for qq in wInExpr(rel) do
		if not assigned(UsedIn[qq]) then
			UsedIn[qq] := {left}:
		else
			UsedIn[qq] := UsedIn[qq] union {left}:
		end if:
	end do:
#	for q in indices(w,nolist) do
	if not assigned(UsedIn[left]) then
		UsedIn[left] := {}:
	end if:
	if nops(UsedIn[left])>500 then
		printf("Updating %d reductions\n", nops(UsedIn[left])):
		# the below updating is slow; hence instead recompute the full table
		doUpdate:=false:
	else
		doUpdate:=true:
	end if:
	for q in UsedIn[left] do
		# remove q from lists
		if doUpdate then
			for qq in wInExpr(w[q]) do
				if not (q in UsedIn[qq]) then
					error "table corrupted":
				end if:
				UsedIn[qq] := UsedIn[qq] minus {q}:
			end do:
		end if:
		# update reduction
		w[q] := expand(eval(w[q],w[left]=rel)):
#		w[q] := expand(eval(w[q])):
		# update UsedIn
		if doUpdate then
			for qq in wInExpr(w[q]) do
				if not assigned(UsedIn[qq]) then
					UsedIn[qq] := {q}:
				else
					UsedIn[qq] := UsedIn[qq] union {q}:
				end if:
			end do:
		end if:
	end do:
	w[left] := rel:
	if not doUpdate then
		InitUsedIn():
	end if:

	# store intermediate results
	nsolved := nsolved+1:
	if nsolved mod 1000 = 0 then
		printf("Solved %d rows, saving results...\n", nsolved):
		save w, "temp.m":
	end if:
end do:

save w, "reduction.m":

# biggest reduction:
ww:=map(length,w):
bad:=max(entries(ww,nolist)):
printf("Largest reduction has length %d:\n",bad):
S:=select(q->rhs(q)=bad, [entries(ww,pairs)]):
S:=lhs(S[1]):
print(S=w[S]):

# print all we have:
# (n,m) pairs:
Got := table():
nmFromEdges := G->[nops(G),nops(map(op,G))-2*nops(G)+2]:
AddToTable := proc(G)
local n,m:
global Got:
	n,m:=op(nmFromEdges(G)):
	if not assigned(Got[n,m]) then
		Got[n,m]:=table():
	end if:
	Got[n,m][numelems(Got[n,m])+1]:=G:
end proc:
seq(AddToTable(q), q in indices(w,nolist)):
for q in indices(Got) do
	Got[op(q)] := {entries(Got[op(q)],nolist)}:
end do:
#S:={indices(w,nolist)}:
#for q in map(nmFromEdges, S) do
#	Got[op(q)]:=select(qq->nmFromEdges(qq)=q, S):
#end do:
#for q in {indices(Got)} do
#	n,m:=op(q):
#	if n+m>5 then next: fi:
#	printf("Weights with (n,m)=(%d,%d):\n",n,m):
#	for qq in Got[n,m] do
#		printf("%a=%a\n",qq,w[qq]):
#	end do:
#end do:
Left := table():
S:={seq(op(indets(q)),q in entries(w))}:
S:=select(q->op(0,q)=w,S):
S:=map(op,S):
AddToTable := proc(G)
local n,m:
global Left:
	n,m:=op(nmFromEdges(G)):
	if not assigned(Left[n,m]) then
		Left[n,m]:=table():
	end if:
	Left[n,m][numelems(Left[n,m])+1]:=G:
end proc:
map(AddToTable,S):
for q in indices(Left) do
	Left[op(q)] := {entries(Left[op(q)],nolist)}:
end do:
for q in {indices(Got)} union {indices(Left)} do
	printf("(n,m)=(%d,%d): %d reductions, %d indeterminates\n",op(q),`if`(assigned(Got[op(q)]),nops(Got[op(q)]),0), `if`(assigned(Left[op(q)]),nops(Left[op(q)]),0)):
end do:

WeightsToFile := proc(S::set(list(list)), fName::string, $)
local f, w:
	if nops(S)=0 then return: end if:
	f := fopen(fName, WRITE, TEXT):
	for w in S do
		fprintf(f,"%a\n",w):
	end do:
	fclose(f):
end proc:

# Statistics of star product graphs and output files (weights to compute):
printf("Star product graphs:\n"):
for q in {indices(Got)} union {indices(Left)} do
	if not assigned(Got[op(q)]) then Got[op(q)]:={}: end if:
	if not assigned(Left[op(q)]) then Left[op(q)]:={}: end if:
	if q[2]<>2 then
		WeightsToFile(Left[op(q)], cat("n",q[1],"m",q[2],".mpl")):
		next:
	end if:
	Got[op(q)] := select(qq->map(nops,qq)=[2$nops(qq)],Got[op(q)]):
	Left[op(q)],Nonstar[op(q)] := selectremove(qq->map(nops,qq)=[2$nops(qq)],Left[op(q)]):
	printf("(n,m)=(%d,%d): %d reductions, %d indeterminates\n",op(q),nops(Got[op(q)]), nops(Left[op(q)])):
	WeightsToFile(Left[op(q)], cat("n",q[1],"m",q[2],"star.mpl")):
	WeightsToFile(Nonstar[op(q)], cat("n",q[1],"m",q[2],".mpl")):
end do:

if nops(Warnlines)>0 then
	WARNING("There were warnings for %1 lines:", nops(Warnlines)):
	lprint(Warnlines):
end if:

printf("Equations solved: %d\n", nsolved):
