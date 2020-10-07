#include <iostream>
#include <stdlib.h>
#include <string.h>

using namespace std;

/**
 * Welcome to vivo !
 */

#define MAX_NUM 100

void sol(int boxs[],int& sum ,int l,int r){
    if(l==r) return;
    int beg = l,end= r;
    int count =0;
    while(boxs[beg]==boxs[l] && beg<=r) {count++;beg++;}
    if(count == (r-l+1)) {
        while(l<=r){boxs[l]=-1;l++;}
        sum += count*count;
        return ;
    }
    else{
        while(beg<r){
            for(int i=beg;i<r;i++){
                if(boxs[beg]== boxs[i]) end = i;
            }

            sol(boxs,sum,beg,end);

            int count1 =0;
            for(int i=l;i<=r;i++){
                if(boxs[i]==boxs[l]) count1++;
                else if(boxs[i] ==-1) continue;
                else{ count1=0; break;}
            }
            sum += count1*count1;

            beg = end+1;
        }
    }
    return;
}


int solution(int boxs[], int N)
{
        // TODO Write your code here
 int score=0;
    int n =0;
    while(n<N){ 
            int end = n;
            for(int i=n;i<N;i++){
                if(boxs[n] == boxs[i]) end = i;
            }

            sol(boxs,score,n,end);

            int count1 =0;
            for(int i=n;i<=end;i++){
                if(boxs[i]==boxs[n]) count1++;
                else if(boxs[i] ==-1) continue;
                else{ count1=0; break;}
            }
            score += count1*count1;

            n = end+1;
        }
   

return score;
}

int main()
{
	string str("");
	getline(cin, str);
	int boxs[MAX_NUM];
	int i = 0;
	char *p;
	int count = 0;
	
	const char* strs = str.c_str();
	p = strtok((char *)strs, " ");
	while(p)
	{
		boxs[i] = atoi(p);
		count++;
		p = strtok(NULL, " ");
		i++;
		if(i >= MAX_NUM)
			break;
	}

	int num = solution(boxs, count);
	cout << num << endl;
	return 0;
}