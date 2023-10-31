#hits.awk by Wilhelmiina Hämäläinen 27.10. 2023 (for gnu awk)
#Implements HITS algorithm i.e. calculates hub and authority values, see 
#Kleinberg (1999): "Authoritative sources in a hyperlinked environment".
#Input graph (text file): On each line give a node and  its adjacency list
#(nodes it links). The node ids should be consecutive non-negative integers
#(assumption that the minimum index is < 1000). Other parameters: give
#the querynodes in the beginning and adjust the convergence conditions
#thr=maximum allowed difference between two rounds (for any hub or auth
#value) and maxR=max number of iterations.
#Run: awk -f hits.awk < inputgraph.txt

BEGIN{
  #give query nodes separated by colon. E.g., all pages containing the keyword.
  querynodes="7:8:9";
#give convergence threshold and/or maximum number of rounds
  thr=0.0001; maxR=100;
  nq=split(querynodes, root, ":")
#root is the root set of HITS
    FS=" "; min=1000; max=-1;
}
{
  #determine min and max node index values
  if ($1>max) max=$1;
  if ($1<min) min=$1;
  #read graph
  for (i=2;i<=NF;i++){
    gr[$1][$i]=1;
    if ($i>max) max=$i;
    if ($i<min) min=$i;
  }
}
END {
#print original graph as an adjacency matrix
  printf("Original graph\n");
  for (i=min;i<=max;i++){
      printf("%d",gr[i][min]);
    for (j=min+1;j<=max;j++)
        printf(",%d",gr[i][j]);
    printf("\n");
  }

  
#create a graph containing 1) the root set, 2) all nodes pointed by the root
#  nodes, 3) nodes pointing to the root nodes (note: usually only some would
#be selected)
  #new nodes will be added to the root array
  #num=number of nodes in the new graph, initially nq
  num=nq;
  for (i=1;i<=nq;i++){
    for (j=min;j<=max;j++){
      #nodes pointed by the i:th root node
      if ((gr[root[i]][j]>0)&&(!incl(root,num,j))){
          num++; root[num]=j;
        }
      #nodes pointing to the i:th root nodes
      #here you could add some selection (take only some)
       if ((gr[j][root[i]]>0)&&(!incl(root,num,j))){
        num++; root[num]=j;
      }
    } #for j
  }

  printf("\nGraph for the query\n");
  for (i=min;i<=max;i++){
    if (incl(root,num,i)){
      printf("(node %d)\t",i);
      if (incl(root,num,min))
        printf("%d",gr[i][min]);
        for (j=min+1;j<=max;j++)
          if (incl(root,num,j))
          printf(",%d",gr[i][j]);
        printf("\n");
      } #for i
  }
  

  
#Now all nodes of the new graph in root. Create the adjacency matrix M
  #for the graph structure. In M, new indexing 1,...,num so that no gaps. 
#Also initialize hub (h) and authority (a) weights.
  printf("\nInitialization\n");
  for (i=1;i<=num;i++){
    h[i]=1.0/sqrt(1.0*num); a[i]=1.0/sqrt(1.0*num);
    printf("h(%d)=%.3e a(%d)=%.3e\n",root[i],h[i],root[i],a[i]);
    for (j=1;j<=num;j++)
      if (gr[root[i]][root[j]]>0)
        M[i][j]=1;

  }

  converge=0;  
#Then begin power iteration
  round=1;
  while ((!converge)&&(round<=maxR)){
    printf("round %d\n",round);
    ssumh=0.0; ssuma=0.0; #for squaresums
    for (i=1;i<=num;i++){
      oldh[i]=h[i]; h[i]=0.0;
      olda[i]=a[i]; a[i]=0.0;
      for (j=1;j<=num;j++){
        if (M[i][j]>0) #i pointing to j
          h[i]+=olda[j]; #update hub i
        if (M[j][i]>0) #j pointing to i
          a[i]+=oldh[j]; #update auth. i
      } #for j
      #update squaresums
      ssumh+=h[i]^2; ssuma+=a[i]^2; 
    } #for i
    #normalize weights and check difference to the previous round
    #if any node has diff>thr, then not converged
    H=sqrt(ssumh); A=sqrt(ssuma);
    converge=1;
    for (i=1;i<=num;i++){
      h[i]=h[i]/H; a[i]=a[i]/A;
      printf("h(%d)=%.3f a(%d)=%.3f\n",root[i],h[i],root[i],a[i]);
      if ((abs(h[i]-oldh[i])>thr)||(abs(a[i]-olda[i])>thr))
        converge=0;
    }
    round++;      
  } #while

  if (converge==1) printf("Converged (all differences less than %f)\n",thr);
}

#Does label lab occur in arr of length len?
#Note: when local variable i introduced, give extra space before it! 
function incl(arr,len,lab, i){
  for (i=1;i<=len;i++){
    if (arr[i]==lab)
      return 1;
  }
  return 0;
}


function abs(v) {return v < 0 ? -v : v}

