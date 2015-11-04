#include "cachelab.h"
#include <getopt.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#define true 1
#define false 0
//Hao Yang andrewid:haoyang

//todo
//process inputs: check
//read file:
//summary:
//



//we're doing least recent so we need time

typedef struct{
  int valid;
  int tag;
  int time;
}line;

typedef struct{
  int tme ;
  int checked;
  bool rec;
}checker;

typedef struct{
  int miss;
  int hit;
  int evict;
}counter;


int v,s,E,b = 0;
int hits,misses,evictions = 0;
int t = 0;
char readFrom[100];
int addr = 0;

line initCache(line* c, int start,int i){//creates the struct and index
  return(c[start+i]);
}
void refreshind(checker* c, int cintime,int startind,int i){
   c->tme = cintime;
   c->checked = startind + i;
}

void cacheOps(int addr, bool x,int num,line* cache,int* iterator,counter* hme){
  int addinv = (0xFFFFFFFF) + num;//checks the starting indices
  int startindex = (((addr >> b)&(addinv))*E);
  t +=1;
  checker* c = malloc( sizeof(checker));//creates a struct to keep track
  if(c == NULL){
    return ;
  }


  c->tme = t;
  c->rec = x;



  for (int i = 0; i < *iterator; i++){//interates through the indices
      line cindex = initCache(cache,startindex,i);
      int temp = c->checked;
      int temp2 = c->tme;

      if(cache[startindex+i].valid == true){//valid flag
        c->rec = !(cindex.time > c->tme);

        if(c->rec == true && (cindex.tag == (addr>>b)) == true){
          refreshind(c, cindex.time,startindex,i);
           cindex.time = t;
           cache[startindex].time = cindex.time;
           if(hme == NULL){//checked malloc is not null
              return;
                }
           hme->hit++;//increaes hit on struct
           return;
        }

        else if(c->rec == true){//if only one is true
          refreshind(c, cindex.time,startindex,i);

         }

        else if ((cindex.tag == (addr>>b))==true){//if only one is true
          cindex.time = t;
          cache[startindex].time = cindex.time;
          c->checked = temp;
          c->tme = temp2;
          if(hme == NULL){//make sure not null
             return;
               }
          hme->hit++;//increaes hit rate on struct
          return;

        }
      }

      else if(cindex.valid == false){
        c->checked = startindex + i;
        c->tme = t;
        break;
      }


       }
  hme->miss++;//increases miss
  hme->evict++;
  if (c->tme == t){
    hme->evict--;
  }

  cache[c->checked].valid = (true);
  cache[c->checked].tag = (addr>>b);
  cache[c->checked].time = t;
  free(c);//free what u malloc
}




int main(int argc, char *argv[]){
  line* cache1;
  counter* counter1;
  char buffer[100];
  int opt;
  int par;
  int numLine = 1;
  FILE* filePath;
  //parsing options
  while ((opt = getopt(argc,argv,"vs:E:b:t:")) != -1){//reads all options
    if(optarg != NULL){
      par = atoi(optarg);
    }
    if (opt == 'v'){
      v = 1; //sets v
    }
    else if (opt == 's'){
      s = par;
      numLine = (numLine << s);
    }
    else if (opt == 'E'){
      E = par;
      if (E > s){
        E++;}
    }
    else if (opt == 'b'){
      b = par;
    }
    else if (opt == 't'){
      strncpy(buffer,optarg,100);
    }
    else {
      printf("Invalid Input");
    }}
  filePath = fopen(buffer,"r");

  if (filePath == NULL){
    printf("File cannot be opened or exist");
    return -1;
  }

  cache1 = malloc(sizeof(line)*(numLine*E));
  //check nullllllllllllllll
  if(cache1 == NULL){
    return -1;
  }

  counter1 = malloc(sizeof(counter));

  while (fgets(buffer,100,filePath) != NULL){
    sscanf(buffer+3,"%x", &addr);
    switch(buffer[1]){//M does twice
    case 'L': if(v == 1){ printf("\n%s", "L");}
    case 'M':  if(v == 1){ printf("%s", "M");}
    case 'S': if(v == 1){printf("%s", "S");}
      cacheOps(addr,false,numLine, cache1,&E,counter1);
      break;

    }
    if (buffer[1] == 'M'){

      printf("\n%s", "M");
      cacheOps(addr,false,numLine,cache1,&E,counter1);
    }
  }
  fclose(filePath);
  printSummary(counter1->hit,counter1->miss,counter1->evict);
  free(cache1);//freee what u malloc
  free(counter1);//free this too
  return (0);
}



