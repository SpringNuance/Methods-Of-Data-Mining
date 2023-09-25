#combdist.py for MDM-2023 by Wiki Hämäläinen 21.9.2023
#Implementation of Aggarwal Eq 3.9, combining parwise Euclidean distances
#based on numerical features and overlap distance based on categorical
#features. Note: the numerical features should be in consecutive columns,
#likewise the catergorical features. Set the weight before calling the
#combdist function!

import pandas as pd
import numpy as np
from math import sqrt

#L2=Euclidean distance between arrays r1 and r2 using numerical features
#in columns beg-end.
def L2dist(r1,r2,beg,end):
	sum=0.0;
	for i in range(beg,end+1):
		sum+=(r1[i]-r2[i])**2;
	return sqrt(sum);


#overlap distance between arrays r1 and r2 using categorical features
#in columns beg-end
def overlapdist(r1,r2,beg,end):
	sum=0.0; len=end-beg+1;
	for i in range(beg,end+1):
		if r1[i]!=r2[i]:
			sum+=1.0;
	return sum/len;	


#Combined distance. Parameters: data as pd.dataframe, beginning and end
#indices of numerical (nbeg, nend) and categorical (cbeg, cend) columns,
#weight w in [0,1] for numerical distance (and 1-w for categorical).
def combdist(data,nbeg,nend,cbeg,cend,w):
	n=data.shape[0]; #number of data points
	nn=int(n*(n-1)/2); #number of pairwise distances
	numdist=np.empty(nn);
	catdist=np.empty(nn);
	next=0;
	for i in range(n-1):
		for j in range(i+1,n):
			print("%s %s"%(data.iloc[i,0],data.iloc[j,0]));
			numdist[next]=L2dist(data.iloc[i,:],data.iloc[j,:],nbeg,nend);
			catdist[next]=overlapdist(data.iloc[i,:],data.iloc[j,:],cbeg,cend);
			print("ndist=%.4f"%numdist[next]);
			next=next+1;

			
	#standard deviations of distances for scaling
	#note: degrees of freedom ddof=1 (divisor N-ddof)  
	nstdev=numdist.std(ddof=1);
	cstdev=catdist.std(ddof=1);

#	print("nmean=%.4f nstdev=%.4f cmean=%.4f cstdev=%.4f"%(numdist.mean(),nstdev,catdist.mean(),cstdev));

	#calculate combined distance and insert into distance matrix
	D=np.empty((n,n));
	next=0;
	for i in range(n-1):
		D[i][i]=0.0; #dist(x,x)=0
		for j in range(i+1,n):
			D[i][j]=w*numdist[next]/nstdev+(1-w)*catdist[next]/cstdev;
			D[j][i]=D[i][j];
			next=next+1;
	return D;


# def main():
# 	#how to call:
# 	#if new numerical features in columns 2,3 and categorical in 4,5
# 	bdata=pd.read_csv("test.csv",delimiter=",");
# 	weight=0.5; #Set the weight in [0,1] here!
# 	D=combdist(bdata,2,3,4,5,weight);

def main():
    # Read the bird species data
    bird_df = pd.read_csv("birdspecies.csv", delimiter=";")
    
    # Set the weight \(\lambda\)
    weight = 0.5  # You can experiment with different values
    
    # Calculate the combined distance matrix
    # Assuming BMI and WSI are in columns 8 and 9, and back and belly are in columns 6 and 7
    D = combdist(bird_df, 8, 9, 6, 7, weight)
    print(D)
    # Print or further process the distance matrix D as needed
	# print(D)
	
if __name__=="__main__":
	main()

# cd "Round 2"
# python combdist.py