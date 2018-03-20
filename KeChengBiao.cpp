#include "stdio.h"
#include "stdlib.h"
#include "math.h"
#include "time.h"

#define lastline 141979//50299//5000//8000//10000//50000     //文件"1000.txt"行数目 
int data[lastline+1][8];

#define Num_node  10000      //节点数目
#define Num_packet 800//1000//600//300        //发送包数目
int total_num_interest = 0;          //对于所有包，感兴趣节点总数 
int source;
int received[4][Num_node+1] ;   //网络中节点是否收到包

double delay[4] = {0};                     //目的节点收到包时间
long int num_copy[4] = {0};                  //网络中拷贝数目
long int num_delivered[4] = {0};             //传递到目的节点包数目 
long int classtime[12]={0,  0, 3600,3600*2+600,3600*3+600,   3600*4,3600*5,3600*6,3600*7,      3600*11,3600*12,3600*13};
long int TTL=3600*48; 

//增加变量
int curseed[Num_node+1], nextseed[Num_node+1];          //当前节课的种子、下一节课的种子 
#define Num_classroom 2600                              //教室最大数目 
int choose_seed_for_class[Num_node+1][Num_classroom+1]; //节点为下节课的所有教室选择种子  

#define  numofin  10//5  // 兴趣个数最多为5个
float pinterest = 0.2;//0.1;//0.3;//0.4;//     //节点对每个兴趣的感兴趣的概率
int getpinterest()//返回一个节点是否感兴趣的概率，1---对此兴趣感兴趣，0---否则
{   int num = rand() % 1000 + 1;
    if( num <= 1000 * pinterest )  return 1;   else return 0;
} 
int interest[Num_node+1][numofin+1];  //numofin[i][j]=1表示节点i有兴趣j，=0表示没有此兴趣 
int interestpacket[Num_packet+1];     //包的兴趣
int cur_roomid[Num_node+1],next_roomid[Num_node+1];   //存放节点某节课所在教室

//*** add for all algorithms 
#define para1 15  //第几周发包 
#define para2 1//3//5//4//2//1  //周几发包 
#define para3 1  //第几节课发包 
//***add end
int curgap, delta, currand1, currand2, currand3, nextrand1, nextrand2, nextrand3;
//第几周、周几、第几节课发包

//以下为Publish/Subscribe(PS)系统代码
int size_win = 50*2*60;        //时间窗口为2节课
int isbroker[Num_node+1];      //1---是broker；0----否则
int popular[Num_node+1][2];    //popular[][0],...[1]分别保存节点在前一个窗口、当前窗口内流行值 
int otherspopular[Num_node+1][Num_node+1];   //otherspopular[i][j]节点i保存的节点j以前流行值
int ismet[Num_node+1][Num_node+1];           //1---两节点本时间窗口相遇过；0----否则  
int lower_bound =int(0.01*Num_node);//int(0.02*Num_node);// 3; 
int upper_bound =int(0.01*Num_node);//int(0.02*Num_node);//int(0.01*Num_node);//int(0.02*Num_node);//int(0.01*Num_node);//int(0.02*Num_node);//int(0.1*Num_node);//12;//9;//3;//8;//        //上下界 
int metbrokers[Num_node+1];                  //当前窗口内，节点遇到的brokers数目 
//end for PS 



//以下考虑课间联系代码
float pout = 0.2;//0.25;//0.1;//0.3;// 0.5;//    //课间走出教室概率
int   Num_place = 2;//4;  //课间每个楼层公共活动场所 
int getprob()//返回一个节点课间是否出去，1---出去，0---不出去
{   int num = rand() % 1000 + 1;
    if( num <= 1000 * pout )  return 1;   else return 0;
} 
int outroom[Num_node+1];    //节点课间出去吗？1--出去，0--否则 
int place[Num_node+1];      //节点课间出去所在的热点区域  
int Iscontact_outroom(int nodeid1, int nodeid2 )//判断节点nodeid1，nodeid2下课期间是否在同一楼层、走出教室、去同一个热点 
{  int classroom1 =cur_roomid[nodeid1], classroom2 =cur_roomid[nodeid2]; 
   if( ( classroom1 > 0 ) && ( classroom2 > 0 ) && ( (classroom1/100) == (classroom2/100) ) 
         && ( outroom[nodeid1]==1 ) && ( outroom[nodeid2]==1) && (place[nodeid1]==place[nodeid2])     )    
        {    //printf("%d---%d",classroom1,classroom2);  getchar();      
             return 1;   
        }
   return 0;
}
//end 


int Haveclass(int nodeid, int week, int day, int classnum)//判断学生nodeid在week周星期day第classnum是否有课 
{                                                         //找到----返回1，否则返回0
   int i, j, flag1=0; //flag初始为0，表示没有找到                                      
   for(i=1; i<=lastline; i++)
 // if(i==1455)
   {//  printf("i=%d",i);  printf("两个整数:%d  %d ",data[i][1], nodeid);
       if(data[i][1] > nodeid) break;
       else 
       if(data[i][1] < nodeid) continue;
            else if(data[i][1] == nodeid) 
             {
                 j=data[i][2];
                 int flag2=0;
                 while(j<=data[i][3])
                 {    if(j == week){     flag2 = 1;    break;     }
                      if(data[i][4] == 1)   j++;
                      else if(data[i][4] == 2)  j=j+2;
                  }
                  //printf("flag2=%d ",flag2);
                  
                  if(flag2==0) continue; 
                  else {  
                           if( (data[i][5] == day) && (data[i][6] == classnum) ) 
                           {     flag1 = 1;  break; } //找到符合要求的那一行 
                       } 
              }//end else
   }//end for
  
   return flag1;   //返回查找结果 
}  
   
int returnclassroom(int nodeid, int week, int day, int classnum)//判断学生nodeid在week周星期day第classnum是否有课 
{                                                         //找到----返回教室id，否则返回0
   int i, j, flag1=0; //flag初始为0，表示没有找到                                      
   for(i=1; i<=lastline; i++)
 // if(i==1455)
   {//  printf("i=%d",i);  printf("两个整数:%d  %d ",data[i][1], nodeid);
       if(data[i][1] > nodeid) break;
       else if(data[i][1] < nodeid) continue;
            else if(data[i][1] == nodeid) 
             {
                 j=data[i][2];
                 int flag2=0;
                 while(j<=data[i][3])
                 {    if(j == week){     flag2 = 1;    break;     }
                      if(data[i][4] == 1)   j++;
                      else if(data[i][4] == 2)  j=j+2;
                  }
                  //printf("flag2=%d ",flag2);
                  
                  if(flag2==0) continue; 
                  else {  
                           if( (data[i][5] == day) && (data[i][6] == classnum) ) 
                           {     flag1 = data[i][7];  break; } //找到符合要求的那一行 
                       } 
              }//end else
   }//end for
              
   return flag1;   //返回查找结果 
}
int Iscontact(int classroom1, int classroom2 )//判断教室classroom1和classroom2是否为同一个教室 //判断节点nodeid1，nodeid2在第week周，星期day,第classnum节课，
{  if( ( classroom1 > 0 ) && ( classroom1 == classroom2 ) )    return 1;
   return 0;
}


int oneofinterest(int interestid, int nodeid)
{                                            //是返回1，否则返回0 
   int i;
   //for(i=1; i<=numofin; i++) 
   //{  if(interest[nodeid][i]==1) && (interestpacket[packetid]==1) ) }   
   if( interest[nodeid][interestid] == 1 ) return 1; else return 0;
} 

int absor(int i,int j,int interestid)   //节点i和j之间的吸收算法，返回1--节点i吸收j，返回0--节点j吸收i  
{  if(i==source) return 1; //节点i吸收j 
   if(j==source) return 0; //节点j吸收i
    
   if( (oneofinterest(interestid,i)==1) && (oneofinterest(interestid,j)==0) )
       return 1;  //节点i吸收j 
   else if( (oneofinterest(interestid,i)==0) && (oneofinterest(interestid,j)==1) ) 
             return 0;   //节点j吸收i 
             
   if(i>j) return 1;  else return 0;
}  

int absor_action(int i, int j, int interestid) //种子节点i和j之间执行吸收动作,还有两个动作没有编写！！！！！！！！！！ 
{   int k, direction = absor(i,j,interestid);
    if(direction == 1)  //节点i吸收j 
    {    curseed[j] = 0;
         for(k=1; k<=Num_classroom; k++)  
             if( ( choose_seed_for_class[i][k] == 0 ) && ( choose_seed_for_class[j][k] > 0 ) )  choose_seed_for_class[i][k] = choose_seed_for_class[j][k];
             else  if( choose_seed_for_class[j][k] > 0 )  nextseed[ choose_seed_for_class[j][k] ] = 0; //这个节点j选择的下节课种子，不再是了，因为节点i选择过 
         for(k=1; k<=Num_classroom; k++)   choose_seed_for_class[j][k] = 0; //表示节点j没有为任何教室，选择下节课种子 
         curseed[j] = 0;  //节点j不再是当前课的种子 
    }
    else                //节点j吸收i
    {    curseed[i] = 0;
         if( ( choose_seed_for_class[j][k] == 0 ) && ( choose_seed_for_class[i][k] > 0 ) )  choose_seed_for_class[j][k] = choose_seed_for_class[i][k];
             else  if( choose_seed_for_class[i][k] > 0 )  nextseed[ choose_seed_for_class[i][k] ] = 0; //这个节点i选择的下节课种子，不再是了，因为节点j选择过
         for(k=1; k<=Num_classroom; k++)   choose_seed_for_class[i][k] = 0; //表示节点i没有为任何教室，选择下节课种子 
         curseed[i] = 0;  //节点i不再是当前课的种子
    }
    return 0;
} 

void statistic(int algonum, int nodeid, int pakcetinterest, int duration)
{   //在算法algonum中，节点nodeid收到一个拷贝，根据当前包的兴趣pakcetinterest对算法的拷贝数（跟包兴趣无关）、延迟、传递率做累加
    num_copy[algonum] ++;
    if( oneofinterest(pakcetinterest,nodeid) == 1 ) 
    {    delay[algonum] += duration/10000.0;   num_delivered[algonum] ++;  }   
    /*if(algonum==0) 
    {   printf("算法%d 节点%d收到包,算法拷贝总数=%d 传递个数=%d*****************************", algonum,nodeid, num_copy[algonum], num_delivered[algonum] );   
        getchar();
    } */                                                        
} 
int haveinterest(int nodeid)
{
      for(int i=1; i<=numofin; i++)  if( interest[nodeid][i] == 1) return 1;
      return 0;
}

void pause()
{    printf("暂停,按任意键继续。");    getchar(); return;
}

//***add for SGBR路由算法 
//int step=360;//300;//600;//300;//480;//180;   //步长：5分钟
float connected_d[Num_node+1][Num_node+1];            //记录节点之间连通度数 
float last_contact_time[Num_node+1][Num_node+1];      //记录节点之间上一次联系最后时间 
//int recent_contact_time[Num_node+1][Num_node+1];    //记录节点之间最近一次联系时间 
//int (*recent_contact_time)[Num_node+1] = new int[Num_node+1][Num_node+1]; 
float *recent_contact_time[Num_node+1];
float sgbr_updating_factor=0.45, sgbr_aging_constant=0.98, sgbr_cth=0.5;//0.6;//0.3;//0.4;//0.2;//0.75;//0.65;// 
int sgbr_copies=int(3*Num_node);   //32;//24;//24;//30%#nodes //12;//   //参数配置
int   sgbr_num_copies[Num_node+1];                    //每个节点处拷贝数目 

int Isnextrelay_sgbr(int i, int j, int interestid)  //在SGBR算法中，节点i是否选择节点j作为下一个中继节点
{   if( oneofinterest(interestid, j) == 1 ) return 1;   //if(j==dest) return 1;
    int k, u, v, w;
    //printf("相遇节点%d,%d连通系数=%.2f  ", i, j, connected_d[i][j]); //char ch=getchar();
    //if( (connected_d[i][j]<sgbr_cth) && (sgbr_num_copies[i]>1) && (last_contact_time[i][j]>0) )
    if( (connected_d[i][j]<sgbr_cth) && (sgbr_num_copies[i]>1)  ) 
    {   //printf("相遇节点%d,%d连通系数=%.2f ", i, j, connected_d[i][j]); //char ch=getchar();
        return 1;
    }

	return 0;
}
int times=15;   //每次课算几个时间单元 
//***add end

int main()
{

    long	int i,j,k,l, val1, val2, val3, val4, val;
	char fname[10000];
	int curgap, nextgap;
    
    
    //***add for sgbr 由于静态申请二维数组因空间太大而出错，所以动态申请二维数组 
    for(i=0; i<=Num_node; i++)   recent_contact_time[i] = new float[Num_node+1];
	//***add end
	
	
	
	//(1)从文件中读出数据到数组 Data[][]
	FILE *fp1;
	sprintf( fname, ".\\6500.txt" );
	if( ( fp1 = fopen ( fname , "r" ) ) == NULL )
	{
		printf( "\n The file's operation is failed." );
		exit( 0 );
	}	
	for(i=1; i<=lastline; i++) 
	{   fscanf(fp1,  "%d", &data[i][1]); //printf( "%d ",data[i][1]);
        fscanf(fp1,  "%d", &data[i][2]); //printf( "%d ",data[i][2]);
        fscanf(fp1,  "%d", &data[i][3]); //printf( "%d ",data[i][3]);
        fscanf(fp1,  "%d", &data[i][4]); //printf( "%d ",data[i][4]);
        fscanf(fp1,  "%d", &data[i][5]); //printf( "%d ",data[i][5]);
        fscanf(fp1,  "%d", &data[i][6]); //printf( "%d ",data[i][6]);
        fscanf(fp1,  "%d", &data[i][7]); //printf( "%d ",data[i][7]);
        //printf("\n");
        //getchar();
     }      	
    fclose(fp1);
    // printf("一个数%d ", data[13941][1]); 


   //随机化产生所有节点的兴趣集合
   for(k=1; k<=Num_node; k++) 
         //for(i=1; i<=numofin; i++) {    interest[k][i] = rand()%2;   }               //对任何兴趣感兴趣的概率为0.5 
         for(i=1; i<=numofin; i++) {    interest[k][i] = getpinterest();   }   //对任何兴趣感兴趣的概率为pinterest 
   // printf("节点兴趣集合："); 

   for(i=1;i<=Num_packet; i++)
   {
          printf("第%d个包\n", i); 
          
          //***revision 下面赋值语句移到发包前的while循环 
          currand1=para1, currand2=para2, currand3=para3;   //设置发包具体时间 
          //***revision end
          
          //初始化
          int s1 = rand()%Num_node+1;
          //s1=47;  Haveclass(s1, rand1, rand2, rand3);
          while( ( Haveclass(s1, currand1, currand2, currand3) == 0 )  || ( haveinterest(s1) == 0 )  ) 
          {    /*printf("再找源节点.");*/    s1 = rand()%Num_node+1;   }
          source = s1;
           
          printf("源节点=%d %d ", source, haveinterest(s1) );  
          //getchar();
          
          //*** revision set j=0 not j=1 for sgbr       
          for(j=0; j<=3; j++)   for(k=1; k<=Num_node; k++) received[j][k]=0;  //所有节点没有收到包 
          for(j=0; j<=3; j++)   received[j][source] = 1;                      //源节点收到包 
          for(j=0; j<=3; j++)   statistic(j, source, interestpacket[i], 0 ); 
          //***revision end 
          
          for(k=1; k<=Num_node; k++)    curseed[k]=nextseed[k]=0;     //所有节点都不是种子 
          nextseed[source] = 1;                                       //源节点初始化为种子 
          
          
          for(k=1; k<=numofin; k++)  if(interest[source][k]==1) {   interestpacket[i] = interest[source][k];  printf("ok!");  break;  }
          //interestpacket[i] = rand()%numofin +1;  //给包i随机选择一个兴趣 
          printf("包的兴趣=%d ", interestpacket[i]); 
          //printf("结果=%d ", oneofinterest(interestpacket[i],8) );
          //printf("结果=%d ", absor(2,3,interestpacket[i]) );
          for(k=1; k<=Num_node; k++) if( oneofinterest(interestpacket[i],k) == 1 ) total_num_interest++;    //累积对包i感兴趣的节点数目 
         
          for(k=1; k<=Num_node; k++)    isbroker[k] = 0;                     //所有节点都不是broker
          for(k=1; k<=Num_node; k++)    popular[k][0] = popular[k][1] = 1;   //所有节点的前一个时间窗口和当前窗口流行值清零     
          for(k=1; k<=Num_node; k++)    for(j=1; j<=Num_node; j++)  otherspopular[k][j] = 0; //节点没有保存其他节点以前时间窗口流行值 
          for(k=1; k<=Num_node; k++)    for(j=1; j<=Num_node; j++)  ismet[k][j] = 0;         //任何两个节点没有相遇过 
          for(k=1; k<=Num_node; k++)    metbrokers[k] = 0;   //所有节点都没有遇到brokers 
          
          //getchar(); 
          
          
          //***add for sgbr, 初始化
          for(j=1; j<=Num_node; j++)     
              for(k=1; k<=Num_node; k++){  connected_d[j][k]=recent_contact_time[j][k]=0;    last_contact_time[j][k]=-1;   } 
          for(j=1; j<=Num_node; j++)     sgbr_num_copies[j]=0;     
          sgbr_num_copies[source]=sgbr_copies; //printf("***%d*** ",sgbr_num_copies[source]); pause(); 
          //***end add
         
         
          //发送包
          int flag=0; //默认没收到  
          int loop,week;
          int  t = 1; 
          
          //***add for sgbr训练五天时间
          //currand1=para1, currand2=para2, currand3=para3; 
          currand1--;
          if(currand1>=1)//训练前一周时间 
          while( currand1 <= (para1-1) )//while( (currand1<=(para1-1)) && (currand2<=5) && (currand3<=11) )
          {
             printf("t=%d \n",t); 
            
             //step1:先求下一节课情况 
             if(currand3==11)//当前为一天最后一节课 
             {    if(currand2==5) //当天为周五 
                  {    delta = (24*3600-(classtime[11]+8*3600+50*60) )+24*3600*2+8*3600;
                       nextrand1=currand1+1;  nextrand2=1; nextrand3=1;
                  }    
                  else//当天不是周五 
                  {    delta = (24*3600-(classtime[11]+8*3600+50*60) )+8*3600;
                       nextrand1=currand1;  nextrand2=currand2+1;  nextrand3=1;
                  }     
             } 
             else
             {    delta = classtime[currand3+1]-classtime[currand3];
                  nextrand1=currand1;  nextrand2=currand2;    nextrand3=currand3+1;
             }
             

            //step1:求出当前课所有节点的教室id
            for(j=1; j<=Num_node; j++)    
            //{    cur_roomid[j] = returnclassroom(j, rand1,rand2,rand3+t-1); next_roomid[j] = returnclassroom(j, rand1,rand2,rand3+t);  }
              {    cur_roomid[j]  = returnclassroom(j, currand1,currand2,currand3); 
                   next_roomid[j] = returnclassroom(j, nextrand1,nextrand2,nextrand3);  
              }

  
            //上课期间更新节点之间连通度* 
             for(j=1; j<=Num_node; j++)
                for(k=1; k<=Num_node; k++)//for(k=1; k<=Num_node; k++) if(k!=j)     
                if(k!=j) 
                {    
                     if( Iscontact(cur_roomid[j], cur_roomid[k]) > 0 )  //节点j和k本节课在同一个教室(相遇)，进行包的扩散
                     {                         
                        if( k >= (j+1) )
                        { 
                          //更新sgbr节点相遇度 
                          //if( t % step == 0 )  
                          {   if( (t-recent_contact_time[j][k]) >1 )                 //(1)设置上一次联系最后时间 
                              {    last_contact_time[j][k] = last_contact_time[k][j] = recent_contact_time[j][k];    }
                              recent_contact_time[j][k]=recent_contact_time[k][j]= t; //(2)更新最近一次联系时间为当前
                              if(last_contact_time[j][k]>0)                                      //(3)从第二次联系开始计算连通系数 
                              {    //printf("(1)before=%.2f  %.2f  ",connected_d[j][k],(t-last_contact_time[j][k])*6);
                                   double tempt_f = pow(sgbr_aging_constant, (t-last_contact_time[j][k])*times);
                                   tempt_f = connected_d[j][k]*tempt_f + (1.0-connected_d[j][k]*tempt_f)*sgbr_updating_factor; 
                                   connected_d[j][k] = connected_d[k][j] = tempt_f;
                                   //printf("(1)tempt_f=%.2f after=%.2f ",tempt_f, connected_d[j][k]); //pause();
                              }
                          } 
                         }//if( k >= (j+1) )
                     }//if( Iscontact(val1, val2) > 0 ) 
                            
                }//if
            //上课期间结束 
                
               
            //课间更新节点之间连通度
            //if( (curgap+50*60) < TTL ) //包的生命期还没有结束
            { for(j=1; j<=Num_node; j++)  {  outroom[j] = getprob();  place[j] = rand()%Num_place+1;  }
            
              for(j=1; j<=Num_node; j++)
                for(k=1; k<=Num_node; k++)   
                if(k!=j) 
                {    //printf("节点%d<--->节点%d\n",j,k); 
                     if( Iscontact_outroom(j, k )==1 ) //节点j和k课间在同一楼层、都走出教室、去同一个热点区域，进行包扩散
                     {  
                        if( k >= (j+1) )//下面执行Epidemic、PS两个算法 
                        { 
                          //更新sgbr节点相遇度 
                          //if( t % step == 0 )  
                          {   if( (t-recent_contact_time[j][k]) >1 )                 //(1)设置上一次联系最后时间 
                              {    last_contact_time[j][k] = last_contact_time[k][j] = recent_contact_time[j][k];    }
                              recent_contact_time[j][k]=recent_contact_time[k][j]= t; //(2)更新最近一次联系时间为当前
                              if(last_contact_time[j][k]>0)                                      //(3)从第二次联系开始计算连通系数 
                              {    //printf("(2)before=%.2f  %.2f  ",connected_d[j][k],(t-last_contact_time[j][k])*6);
                                   double tempt_f = pow(sgbr_aging_constant, (t-last_contact_time[j][k])*times);
                                   tempt_f = connected_d[j][k]*tempt_f + (1.0-connected_d[j][k]*tempt_f)*sgbr_updating_factor; 
                                   connected_d[j][k] = connected_d[k][j] = tempt_f;
                                   //printf("(2)tempt_f=%.2f after=%.2f ",tempt_f, connected_d[j][k]); //pause();
                              }
                          }                         
                        }//if( k >= (j+1) )                            
                     }//if( Iscontact_outroom(j, k )==1 )                             
                }//if                
           }//end if( (curgap+50*60) < TTL )  
          //(2/2)课间结束               
          
          
          currand1 = nextrand1;    currand2 = nextrand2;   currand3 = nextrand3;
          //curgap += delta;
  
          t++;    //t变为下一节课
          
       /*   for(j=1; j<=Num_node; j++) for(k=j+1; k<=Num_node; k++) 
          if(connected_d[j][k]>0)
          {     printf("%.2f  ", connected_d[j][k]); 
                //char ch=getchar();
          }
         */  
      }//end while
/*      for(j=1; j<=Num_node; j++) for(k=j+1; k<=Num_node; k++) 
          if(connected_d[j][k]>0)
          {     printf("%.2f  ", connected_d[j][k]); 
                //char ch=getchar();
          }*/
      //pause();
      ///for(j=1; j<=Num_node; j++) //???设置节点之间重新相遇，不考虑以前热身相遇情况（只保留节点之间连通度）    
      //        for(k=1; k<=Num_node; k++){  connected_d[j][k]=recent_contact_time[j][k]=0;    last_contact_time[j][k]=-1;   }
      //***add end   
          
          
          
          //***revision 赋值语句从前面移到这里 
          printf("热身后:t=%d currand1=%d currand2=%d currand3=%d \n",t, currand1, currand2,currand3,curgap); 
          currand1=para1, currand2=para2, currand3=para3;   //设置发包具体时间 
          //t = 1;
          //***revision end
          
          int  num_win_ps=0;  
          curgap = 0; //因为没有上一节课，所以和上一节课时间距离为0 
          while( (currand1<=18) && (curgap<TTL) )
          {
//             printf("t=%d currand1=%d currand2=%d currand3=%d curgap=%d(%d) \n",t, currand1, currand2,currand3,curgap, curgap/3600); 
//             printf("t=%d \n",t); 
             //pause();     
             
             //step1:先求下一节课情况 
             if(currand3==11)//当前为一天最后一节课 
             {    if(currand2==5) //当天为周五 
                  {    delta = (24*3600-(classtime[11]+8*3600+50*60) )+24*3600*2+8*3600;
                       nextrand1=currand1+1;  nextrand2=1; nextrand3=1;
                  }    
                  else//当天不是周五 
                  {    delta = (24*3600-(classtime[11]+8*3600+50*60) )+8*3600;
                       //printf("delta=%d  ",delta);
                       nextrand1=currand1;  nextrand2=currand2+1;  nextrand3=1;
                  }     
             } 
             else
             {    delta = classtime[currand3+1]-classtime[currand3];
                  nextrand1=currand1;  nextrand2=currand2;    nextrand3=currand3+1;
             }
             //printf("t=%d currand1=%d currand2=%d currand3=%d curgap=%d(%d) \n",t, currand1, currand2,currand3,curgap); 

            //step1:求出当前课所有节点的教室id
            for(j=1; j<=Num_node; j++)    
            //{    cur_roomid[j] = returnclassroom(j, rand1,rand2,rand3+t-1); next_roomid[j] = returnclassroom(j, rand1,rand2,rand3+t);  }
              {    cur_roomid[j]  = returnclassroom(j, currand1,currand2,currand3); 
                   next_roomid[j] = returnclassroom(j, nextrand1,nextrand2,nextrand3);  
              }

             //step2：我们算法部分代码----设置当前节课种子、当前节课种子为下节课自己去的教室，设置自己为种子 
             for(j=1; j<=Num_node; j++)  if(nextseed[j]==1) curseed[j]=1;   else curseed[j]=0; //一般情况：使用上节课设置的种子，更新当前种子状态
             val = cur_roomid[source]; //val = returnclassroom(source, rand1,rand2,rand3+t-1); //针对一种特殊情况：源节点有可能上节课没有课，这节课开始有课，自然成为种子 
             if(val>0)  curseed[j] = 1;
             
             for(j=1; j<=Num_node; j++) //nextseed[j] = 0; 
                  if(cur_roomid[j]>0) nextseed[j] = 0;     //使得以前被选为种子的节点，即使当前课没有课，下节课仍作为未来种子    
             //if( (rand3+t) < 11 )
             {   for(j=1; j<=Num_node; j++)  for(k=1; k<=Num_classroom; k++)  choose_seed_for_class[j][k] = 0;//当前节课，所有节点都没有为下节课的教室选择种子 
                 for(j=1; j<=Num_node; j++)
                     if( curseed[j]==1)    //当前节课是种子，下节课有课的话，优先选择自己作为下节课种子
                     {   val = next_roomid[j]; //val = returnclassroom(j, rand1,rand2,rand3+t);  
                         if( val>0 ) { choose_seed_for_class[ j ][ val ] = j;  nextseed[j]=1;  }  //
                     }       
             }       
             
              
            //step3：Publish/Subscribe(PS)算法部分代码 
            if( ( curgap - num_win_ps*size_win ) >= 0 ) //if( ( (classtime[rand3+t-1]-classtime[rand3]) % size_win ) == 0 )  //新一轮时间窗口开始
            {   //printf("新一轮时间窗口开始:\n"); 
                num_win_ps++;
                for(k=1; k<=Num_node; k++)   {   popular[k][0] = popular[k][1];    popular[k][1] = 0;   }  //更新上一时间窗口流行值 
                for(j=1; j<=Num_node; j++)   for(k=1; k<=Num_node; k++)    ismet[j][k] = 0;     //节点之间见面情况清零
                for(j=1; j<=Num_node; j++)   metbrokers[j] = 0;    //所有节点都没有遇见brokers         
                
            }
            
          
            
            
            //step4：两节点相遇，包扩散
            //(1/2)上课期间课通信开始************************************************************************************************* 
             for(j=1; j<=Num_node; j++)
                for(k=1; k<=Num_node; k++)//for(k=1; k<=Num_node; k++) if(k!=j)     
                if(k!=j) 
                {    //printf("节点%d<--->节点%d\n",j,k); 
                     //val1 = returnclassroom(j, rand1,rand2,rand3+t-1);     val2 = returnclassroom(k, rand1,rand2,rand3+t-1);
                     if( Iscontact(cur_roomid[j], cur_roomid[k]) > 0 ) //if( Iscontact(val1, val2) > 0 ) //节点j和k本节课在同一个教室(相遇)，进行包的扩散
                     {   
                       //我们算法开始************************************************************************************************************************ 
                       if(curseed[j]==1)//节点j为种子
                       {  //val3 = returnclassroom(j, rand1,rand2,rand3+t);   val4 = returnclassroom(k, rand1,rand2,rand3+t);
                         //case 1:节点j和节点k将在下一节属于同一教室
                         //if( ((rand3+t)<11) && (Iscontact(next_roomid[j], next_roomid[k])>0) ) //if( ((rand3+t)<11) && (Iscontact(val3, val4)>0) )
                         if( (Iscontact(next_roomid[j], next_roomid[k])>0) )
                         {   if( (received[1][k]==1) && (curseed[k]==1) ) 
                             {    absor_action(j, k, interestpacket[i]);   //printf("case1.1:节点%d<->%d互相吸收 ",j,k);  pause();  
                             } //执行吸收动作 
                             else if( (received[1][k]==0) && (oneofinterest(interestpacket[i],k)==1) )   
                                  {  received[1][k]=1;     statistic(1, k, interestpacket[i], curgap );//statistic(1, k, interestpacket[i], classtime[rand3+t-1]-classtime[rand3] );    
                                     //printf("case1.2:节点%d收到包，来自节点%d ",k,j); pause();  
                                  }
                         }
                         else //case 2:否则,都有课但不在同一教室 
                             //if( ((rand3+t)<11) && (next_roomid[j]>0) && (next_roomid[k]>0) && (next_roomid[j]!=next_roomid[k]) ) //if( ((rand3+t)<11) && (val3>0) && (val4>0) && (val3!=val4) )
                             if( (next_roomid[j]>0) && (next_roomid[k]>0) && (next_roomid[j]!=next_roomid[k]) )
                             {   if( received[1][k]==1) //case 2.1:节点k有拷贝 
                                 {    if(curseed[k]==1) //节点k是种子 
                                      {    val = absor_action(j, k, interestpacket[i]);   //执行吸收动作
                                           //printf("case2.1:节点%d<->%d互相吸收 ",j,k);  pause(); 
                                           if(val==1)//节点j吸收k 
                                           {  if( choose_seed_for_class[j][ next_roomid[k] ]==0 ) { nextseed[k]=1;  choose_seed_for_class[j][ next_roomid[k] ]==k;}  }
                                           else      //节点k吸收j 
                                           {  if( choose_seed_for_class[k][ next_roomid[j] ]==0 ) { nextseed[j]=1;  choose_seed_for_class[j][ next_roomid[j] ]==j;} } 
                                      }
                                      else  //节点k不是种子 
                                      {  if( choose_seed_for_class[j][next_roomid[k]]==0 ) { nextseed[k]=1;  choose_seed_for_class[j][next_roomid[k]]==k; }   }
                                 }
                                 else //case 2.2:节点k没有拷贝 
                                 {  //printf("case2.2:");  pause(); 
                                    if( oneofinterest(interestpacket[i],k)==1)  
                                    //{   received[1][k]=1;  statistic(1, k, interestpacket[i], classtime[rand3+t-1]-classtime[rand3] );     }
                                    {   received[1][k]=1;  statistic(1, k, interestpacket[i], curgap );     }
                                    if( choose_seed_for_class[j][ next_roomid[k] ]==0 )  //之前还没有为j下节课教室选择种子，现在选择j作为种子 
                                    {   nextseed[k]=1;   choose_seed_for_class[j][ next_roomid[k] ]==k;                                 }
                                 }
                             }
                             //else if( ((rand3+t)<11) && (next_roomid[j]>0) && (next_roomid[k]==0) )//case 3:节点j下节有课，节点k没有 
                             else if( (next_roomid[j]>0) && (next_roomid[k]==0) )
                                  {   //printf("case3:");  pause(); 
                                      if( (received[1][k]==0) && (oneofinterest(interestpacket[i],k)==1) )   
                                      //{     received[1][k]=1;    statistic(1, k, interestpacket[i], classtime[rand3+t-1]-classtime[rand3] );     }
                                      {     received[1][k]=1;    statistic(1, k, interestpacket[i], curgap );     }   
                                  }    //节点k没有拷贝但感兴趣，就拷贝一份 
                                  else//case 4:节点j下节没有课，节点k有    
                                  {   //printf("case4:");  pause(); 
                                      if( (received[1][k]==0) && (oneofinterest(interestpacket[i],k)==1) )  //同上... 
                                      //{     received[1][k]=1;    statistic(1, k, interestpacket[i], classtime[rand3+t-1]-classtime[rand3] );     } 
                                      {     received[1][k]=1;    statistic(1, k, interestpacket[i], curgap );     }     
                                      if( choose_seed_for_class[j][next_roomid[k]]==0 )  //之前还没有为j下节课教室选择种子，现在选择j作为种子 
                                      {     nextseed[k]=1;   choose_seed_for_class[j][next_roomid[k]]==k;  }
                                      //if( j!=source )    curseed[j]=0;            //节点j不是源节点，设置节点j当前不是种子节点 
                                  }                         
                         }//end if(curseed[j]==1)//节点j为种子
                        //我们算法结束***********************************************************************************************************************
                          
                  
                          
                        if( k >= (j+1) )//下面执行Epidemic、PS两个算法 
                        { 
                          //Epidemic算法开始*************************************************************************************************************** 
                          if( (received[2][j]==1) && (received[2][k]==0)  )  //携带者遇到任意节点 
                          {   received[2][k]=1;  statistic(2,k,interestpacket[i],curgap);//statistic(2,k,interestpacket[i],classtime[rand3+t-1]-classtime[rand3]);  
                              //printf("case(2):携带者%d遇到节点-%d ",j,k); //pause();
                          }
                          else if( (received[2][k]==1) && (received[2][j]==0)  ) //携带者遇到任意节点
                          {   received[2][j]=1;  statistic(2,j,interestpacket[i],curgap);//statistic(2,j,interestpacket[i],classtime[rand3+t-1]-classtime[rand3]);  
                              //printf("case(2):携带者%d遇到节点-%d ",k,j); //pause();
                          }
                          //以下代码与上面二选一，携带者仅仅在遇到感兴趣节点，才产生拷贝 
                
                         
                          //Publish/Subscribe(PS)算法开始*************************************************************************************************** 
                          if(ismet[j][k]==0){ popular[j][1]++; if(isbroker[k]==1) metbrokers[j]++; }       ismet[j][k] = 1; //1:更新节点的当前时间窗口流行值、标识相遇过 
                          if(ismet[k][j]==0){ popular[k][1]++; if(isbroker[j]==1) metbrokers[k]++; }       ismet[k][j] = 1;
                          
                          otherspopular[j][k] = popular[k][0];   otherspopular[k][j] = popular[j][0];//2:更新节点保存的其他节点上一个时间窗口流行值   
                     
                          if( (popular[j][1]<lower_bound) && (isbroker[k]==0) )   isbroker[k] = 1;//3_1：将普通节点选为broker 
                          if( (popular[k][1]<lower_bound) && (isbroker[j]==0) )   isbroker[j] = 1;//3_1：将普通节点选为broker
                          
                          if( (metbrokers[j]>=upper_bound) && (isbroker[k]==1) )  isbroker[k] = 0;//3_2：将broker选为普通节点
                          if( (metbrokers[k]>=upper_bound) && (isbroker[j]==1) )  isbroker[j] = 0;//3_2：将broker选为普通节点
                          
                          //4：包的扩散
                          if((j==source)&&(isbroker[k]==1)&&(received[3][k]==0))//源遇到broker
                          {   received[3][k]=1;  statistic(3,k,interestpacket[i],curgap);//statistic(3,k,interestpacket[i],classtime[rand3+t-1]-classtime[rand3]);  
                              //printf("case4.1:源遇到broker-%d ",k); //pause();
                          } 
                          if((k==source)&&(isbroker[j]==1)&&(received[3][j]==0))//同上
                          {   received[3][j]=1;  statistic(3,j,interestpacket[i],curgap);//statistic(3,j,interestpacket[i],classtime[rand3+t-1]-classtime[rand3]);  
                              //printf("case4.1:源遇到broker-%d ",j); //pause();
                          }
                          if((isbroker[j]==1)&&(received[3][j]==1)&&(isbroker[k]==1)&&(received[3][k]==0))//broker遇到broker
                          {   received[3][k]=1;  statistic(3,k,interestpacket[i],curgap);//statistic(3,k,interestpacket[i],classtime[rand3+t-1]-classtime[rand3]);  
                              //printf("case4.2:broker%d遇到broker-%d ",j,k); //pause();
                          }
                          if((isbroker[k]==1)&&(received[3][k]==1)&&(isbroker[j]==1)&&(received[3][j]==0))//同上
                          {   received[3][j]=1;  statistic(3,j,interestpacket[i],curgap);//statistic(3,j,interestpacket[i],classtime[rand3+t-1]-classtime[rand3]);  
                              //printf("case4.2:broker%d遇到broker-%d ",k,j); //pause();
                          }
                          if((isbroker[j]==1)&&(received[3][j]==1)&&(isbroker[k]==0)&&(k!=source)
                          &&(received[3][k]==0)&&(oneofinterest(interestpacket[i],k)==1))//broker遇到感兴趣节点 
                          {   received[3][k]=1;  statistic(3,k,interestpacket[i],curgap);//statistic(3,k,interestpacket[i],classtime[rand3+t-1]-classtime[rand3]);  
                              //printf("case4.3:broker%d遇到兴趣节点-%d ",j,k); //pause();
                          }
                          if((isbroker[k]==1)&&(received[3][k]==1)&&(isbroker[j]==0)&&(j!=source)
                          &&(received[3][j]==0)&&(oneofinterest(interestpacket[i],j)==1))//broker遇到感兴趣节点 
                          {   received[3][j]=1;  statistic(3,j,interestpacket[i],curgap);//statistic(3,j,interestpacket[i],classtime[rand3+t-1]-classtime[rand3]);  
                              //printf("case4.3:broker%d遇到感兴趣节点-%d ",k,j); //pause();
                          }      /**/  
                          //PS算法结束********************************************************************************************* 
            
                          
                          //***add for sgbr                           
                          if( (received[0][j]==1) && (sgbr_num_copies[j]>=1) && (received[0][k]==0) && (oneofinterest(interestpacket[i],j)==0)
                              && (oneofinterest(interestpacket[i],k)==1) )//携带者j(不感兴趣、1 个拷贝)遇到感兴趣节点，转发 
                          {   received[0][k]=1;  sgbr_num_copies[k]=sgbr_num_copies[j]; sgbr_num_copies[j]=0; statistic(0,k,interestpacket[i],curgap);  
                              //printf("case(0)1.1:携带者%d遇到兴趣节点--%d ",j,k); pause();
                          }
                          else if( (received[0][k]==1) && (sgbr_num_copies[k]>=1) && (received[0][j]==0) && (oneofinterest(interestpacket[i],k)==0)
                              && (oneofinterest(interestpacket[i],j)==1) )//与上面对称：携带者k(不感兴趣、1 个拷贝)遇到感兴趣节点，转发 
                          {   received[0][j]=1;  sgbr_num_copies[j]=sgbr_num_copies[k]; sgbr_num_copies[k]=0;  statistic(0,j,interestpacket[i],curgap);  
                              //printf("case(0)1.2:携带者%d遇到兴趣节点--%d ",k,j); pause();
                          }
     
                          else if( (received[0][j]==1) && (sgbr_num_copies[j]>1) && (received[0][k]==0) && (Isnextrelay_sgbr(j,k,interestpacket[i])==1)  )
                          //携带者j(大于1个拷贝)遇到没有收到包、且符合条件节点，转发    
                          {   received[0][k]=1;   l = sgbr_num_copies[j]; sgbr_num_copies[j]=l/2; sgbr_num_copies[k]=l-sgbr_num_copies[j]; 
                              //printf("(1)%d %d %d  ",l,sgbr_num_copies[j],sgbr_num_copies[k]); pause(); 
                              statistic(0,k,interestpacket[i],curgap); 
                              //printf("case(0)2.1:携带者%d遇到没有拷贝节点-%d ",j,k); pause();
                          }
                          else if( (received[0][k]==1) && (sgbr_num_copies[k]>1) && (received[0][j]==0) && (Isnextrelay_sgbr(k,j,interestpacket[i])==1)  )
                          //与上面对称：携带者k(大于1个拷贝)遇到没有收到包、且符合条件节点，转发    
                          {   received[0][j]=1;   l = sgbr_num_copies[k]; sgbr_num_copies[k]=l/2; sgbr_num_copies[j]=l-sgbr_num_copies[k]; 
                              //printf("(1)%d %d %d  ",l,copies_sw[j],copies_sw[k]); pause(); 
                              statistic(0,j,interestpacket[i],curgap);
                              //printf("case(4)2.1:携带者%d遇到没有拷贝节点-%d ",j,k); //pause();
                          }   
                          //更新sgbr节点相遇度 
                          //if( t % step == 0 )  
                          {   if( (t-recent_contact_time[j][k]) >1 )                 //(1)设置上一次联系最后时间 
                              {    last_contact_time[j][k] = last_contact_time[k][j] = recent_contact_time[j][k];    }
                              recent_contact_time[j][k]=recent_contact_time[k][j]= t; //(2)更新最近一次联系时间为当前
                              if(last_contact_time[j][k]>0)                                      //(3)从第二次联系开始计算连通系数 
                              {    double tempt_f = pow(sgbr_aging_constant, (t-last_contact_time[j][k])*times);
                                   double ddd=tempt_f;
                                   tempt_f = connected_d[j][k]*tempt_f + (1.0-connected_d[j][k]*tempt_f)*sgbr_updating_factor; 
                                   connected_d[j][k] = connected_d[k][j] = tempt_f;
                                   if(connected_d[j][k]>1) 
                                   { printf("ERROR! tempt_f=%.2f, %.2f %.2f ",tempt_f, (t-last_contact_time[j][k])*times, ddd); pause(); } 
                              }
                          }    
                          //***add end for sgbr
                                     
            
            
            
            
                         }//if( k >= (j+1) )//下面执行Epidemic、PS两个算法  
                                         
                     }//if( Iscontact(val1, val2) > 0 ) 
                            
                }//for(k=(j+1); k<=Num_node;
           //(1/2)上课期间课通信结束************************************************************************************************* 
                
                
           //(2/2)课间通信开始***************************************************************************************************** 
            if( (curgap+50*60) < TTL ) //包的生命期还没有结束
            { for(j=1; j<=Num_node; j++)  {  outroom[j] = getprob();  place[j] = rand()%Num_place+1;  }
            
              for(j=1; j<=Num_node; j++)
                for(k=1; k<=Num_node; k++)//for(k=1; k<=Num_node; k++) if(k!=j)     
                if(k!=j) 
                {    //printf("节点%d<--->节点%d\n",j,k); 
                     if( Iscontact_outroom(j, k )==1 ) //节点j和k课间在同一楼层、都走出教室、去同一个热点区域，进行包扩散
                     {   
                         
                         //我们算法开始************************************************************************************************************************ 
                        /* if( ( received[1][j]==1) && ( received[1][k]==0) )
                         {    received[1][k]=1;     statistic(1, k, interestpacket[i], curgap+50*60 );//statistic(1, k, interestpacket[i], classtime[rand3+t-1]-classtime[rand3]+50*60 ); 
                              if( choose_seed_for_class[j][ next_roomid[k] ]==0 ) { nextseed[k]=1;  choose_seed_for_class[j][ next_roomid[k] ]==k; }
                         } */
                         if( received[1][j]==1 ) 
                         {    if( received[1][k]==1) //节点k有拷贝 
                              {    if( choose_seed_for_class[j][ next_roomid[k] ]==0 ) { nextseed[k]=1;  choose_seed_for_class[j][ next_roomid[k] ]==k; }
                              } 
                              else//否则 
                              {    if( oneofinterest(interestpacket[i],k)==1) //节点k对此包感兴趣 
                                   {    received[1][k]=1;     statistic(1, k, interestpacket[i], curgap+50*60 ); 
                                        if( choose_seed_for_class[j][ next_roomid[k] ]==0 ) { nextseed[k]=1;  choose_seed_for_class[j][ next_roomid[k] ]==k; }
                                   }
                                   else//否则
                                   {    if( choose_seed_for_class[j][ next_roomid[k] ]==0 ) 
                                        {     received[1][k]=1;     statistic(1, k, interestpacket[i], curgap+50*60 );
                                              nextseed[k]=1;        choose_seed_for_class[j][ next_roomid[k] ]==k; 
                                        }
                                   } 
                              } 
                         }
                        //我们算法结束***********************************************************************************************************************
                          
                          
                          
                        if( k >= (j+1) )//下面执行Epidemic、PS两个算法 
                        { 
                          //Epidemic算法开始*************************************************************************************************************** 
                          if( (received[2][j]==1) && (received[2][k]==0)  )  //携带者遇到任意节点 
                          {   received[2][k]=1;  statistic(2,k,interestpacket[i],curgap+50*60);//statistic(2,k,interestpacket[i],classtime[rand3+t-1]-classtime[rand3]+50*60);  
                              //printf("case(2):携带者%d遇到节点-%d ",j,k); //pause();
                          }
                          else if( (received[2][k]==1) && (received[2][j]==0)  ) //携带者遇到任意节点
                          {   received[2][j]=1;  statistic(2,j,interestpacket[i],curgap+50*60);  //statistic(2,j,interestpacket[i],classtime[rand3+t-1]-classtime[rand3]+50*60);  
                              //printf("case(2):携带者%d遇到节点-%d ",k,j); //pause();
                          }
                      
                          //Publish/Subscribe(PS)算法开始*************************************************************************************************** 
                          if(ismet[j][k]==0){ popular[j][1]++; if(isbroker[k]==1) metbrokers[j]++; }       ismet[j][k] = 1; //1:更新节点的当前时间窗口流行值、标识相遇过 
                          if(ismet[k][j]==0){ popular[k][1]++; if(isbroker[j]==1) metbrokers[k]++; }       ismet[k][j] = 1;
                          
                          otherspopular[j][k] = popular[k][0];   otherspopular[k][j] = popular[j][0];//2:更新节点保存的其他节点上一个时间窗口流行值   
                     
                          if( (popular[j][1]<lower_bound) && (isbroker[k]==0) )   isbroker[k] = 1;//3_1：将普通节点选为broker 
                          if( (popular[k][1]<lower_bound) && (isbroker[j]==0) )   isbroker[j] = 1;//3_1：将普通节点选为broker
                          
                          if( (metbrokers[j]>=upper_bound) && (isbroker[k]==1) )  isbroker[k] = 0;//3_2：将broker选为普通节点
                          if( (metbrokers[k]>=upper_bound) && (isbroker[j]==1) )  isbroker[j] = 0;//3_2：将broker选为普通节点
                          
                          //4：包的扩散
                          if((j==source)&&(isbroker[k]==1)&&(received[3][k]==0))//源遇到broker
                          {   received[3][k]=1;  statistic(3,k,interestpacket[i],curgap+50*60);//statistic(3,k,interestpacket[i],classtime[rand3+t-1]-classtime[rand3]+50*60);  
                              //printf("case4.1:源遇到broker-%d ",k); //pause();
                          } 
                          if((k==source)&&(isbroker[j]==1)&&(received[3][j]==0))//同上
                          {   received[3][j]=1;  statistic(3,j,interestpacket[i],curgap+50*60);//statistic(3,j,interestpacket[i],classtime[rand3+t-1]-classtime[rand3]+50*60);  
                              //printf("case4.1:源遇到broker-%d ",j); //pause();
                          }
                          if((isbroker[j]==1)&&(received[3][j]==1)&&(isbroker[k]==1)&&(received[3][k]==0))//broker遇到broker
                          {   received[3][k]=1;  statistic(3,k,interestpacket[i],curgap+50*60);//statistic(3,k,interestpacket[i],classtime[rand3+t-1]-classtime[rand3]+50*60);  
                              //printf("case4.2:broker%d遇到broker-%d ",j,k); //pause();
                          }
                          if((isbroker[k]==1)&&(received[3][k]==1)&&(isbroker[j]==1)&&(received[3][j]==0))//同上
                          {   received[3][j]=1;  statistic(3,j,interestpacket[i],curgap+50*60);//statistic(3,j,interestpacket[i],classtime[rand3+t-1]-classtime[rand3]+50*60);  
                              //printf("case4.2:broker%d遇到broker-%d ",k,j); //pause();
                          }
                          if((isbroker[j]==1)&&(received[3][j]==1)&&(isbroker[k]==0)&&(k!=source)
                          &&(received[3][k]==0)&&(oneofinterest(interestpacket[i],k)==1))//broker遇到感兴趣节点 
                          {   received[3][k]=1;  statistic(3,k,interestpacket[i],curgap+50*60);//statistic(3,k,interestpacket[i],classtime[rand3+t-1]-classtime[rand3]+50*60);  
                              //printf("case4.3:broker%d遇到兴趣节点-%d ",j,k); //pause();
                          }
                          if((isbroker[k]==1)&&(received[3][k]==1)&&(isbroker[j]==0)&&(j!=source)
                          &&(received[3][j]==0)&&(oneofinterest(interestpacket[i],j)==1))//broker遇到感兴趣节点 
                          {   received[3][j]=1;  statistic(3,j,interestpacket[i],curgap+50*60);//statistic(3,j,interestpacket[i],classtime[rand3+t-1]-classtime[rand3]+50*60);  
                              //printf("case4.3:broker%d遇到感兴趣节点-%d ",k,j); //pause();
                          }        
                          //PS算法结束********************************************************************************************* 
                     
                          //***add for sgbr                           
                          if( (received[0][j]==1) && (sgbr_num_copies[j]>=1) && (received[0][k]==0) && (oneofinterest(interestpacket[i],j)==0)
                              && (oneofinterest(interestpacket[i],k)==1) )//携带者j(不感兴趣、1 个拷贝)遇到感兴趣节点，转发 
                          {   //printf("(0)拷贝数=%d       ",sgbr_num_copies[j]); 
                              received[0][k]=1;  sgbr_num_copies[k]=sgbr_num_copies[j]; sgbr_num_copies[j]=0; statistic(0,k,interestpacket[i],curgap+50*60);  
                              //printf("case(0)1.1:携带者%d遇到兴趣节点-%d ",j,k); pause();
                          }
                          else if( (received[0][k]==1) && (sgbr_num_copies[k]>=1) && (received[0][j]==0) && (oneofinterest(interestpacket[i],k)==0)
                              && (oneofinterest(interestpacket[i],j)==1) )//与上面对称：携带者k(不感兴趣、1 个拷贝)遇到感兴趣节点，转发 
                          {   //printf("(0)拷贝数=%d     ",sgbr_num_copies[k]); 
                              received[0][j]=1;  sgbr_num_copies[j]=sgbr_num_copies[k]; sgbr_num_copies[k]=0;  statistic(0,j,interestpacket[i],curgap+50*60);  
                              //printf("case(0)1.2:携带者%d遇到兴趣节点-%d ",k,j); pause();
                          }
     
                          else if( (received[0][j]==1) && (sgbr_num_copies[j]>1) && (received[0][k]==0) && (Isnextrelay_sgbr(j,k,interestpacket[i])==1)  )
                          //携带者j(大于1个拷贝)遇到没有收到包、且符合条件节点，转发    
                          {   //printf("(1)拷贝数=%d      ",sgbr_num_copies[j]); 
                              received[0][k]=1;   l = sgbr_num_copies[j]; sgbr_num_copies[j]=l/2; sgbr_num_copies[k]=l-sgbr_num_copies[j]; 
                              //printf("(1)%d %d %d  ",l,copies_sw[j],copies_sw[k]); pause(); 
                              statistic(0,k,interestpacket[i],curgap+50*60); 
                              //printf("case(0)2.1:携带者%d遇到没有拷贝节点-%d ",j,k); pause();
                          }
                          else if( (received[0][k]==1) && (sgbr_num_copies[k]>1) && (received[0][j]==0) && (Isnextrelay_sgbr(k,j,interestpacket[i])==1)  )
                          //与上面对称：携带者k(大于1个拷贝)遇到没有收到包、且符合条件节点，转发    
                          {   //printf("(1)拷贝数=%d     ",sgbr_num_copies[k]); 
                              received[0][j]=1;   l = sgbr_num_copies[k]; sgbr_num_copies[k]=l/2; sgbr_num_copies[j]=l-sgbr_num_copies[k]; 
                              //printf("(1)%d %d %d  ",l,copies_sw[j],copies_sw[k]); pause(); 
                              statistic(0,j,interestpacket[i],curgap+50*60);
                              //printf("case(0)2.2:携带者%d遇到没有拷贝节点-%d ",j,k); pause();
                          }   
                          //更新sgbr节点相遇度 
                          //if( t % step == 0 )  
                          {   if( (t-recent_contact_time[j][k]) >1 )                 //(1)设置上一次联系最后时间 
                              {    last_contact_time[j][k] = last_contact_time[k][j] = recent_contact_time[j][k];    }
                              recent_contact_time[j][k]=recent_contact_time[k][j]= t; //(2)更新最近一次联系时间为当前
                              if(last_contact_time[j][k]>0)                                      //(3)从第二次联系开始计算连通系数 
                              {    double tempt_f = pow(sgbr_aging_constant, (t-last_contact_time[j][k])*times);
                                   tempt_f = connected_d[j][k]*tempt_f + (1.0-connected_d[j][k]*tempt_f)*sgbr_updating_factor; 
                                   connected_d[j][k] = connected_d[k][j] = tempt_f;
                              }
                          }    
                          //***add end for sgbr                          
                     
                     
                        }//if( k >= (j+1) )//下面执行Epidemic、PS两个算法  
                                         
                     }//if( Iscontact_outroom(j, k )==1 ) 
                            
                }//for(k=(j+1); k<=Num_node;                
           }//end if( (curgap+50*60) < TTL ) //包的生命期还没有结束  
          //(2/2)课间通信结束*************************************************************************************************                 
          
          
          currand1 = nextrand1;    currand2 = nextrand2;   currand3 = nextrand3;
          curgap += delta;
          
          t++;    //t变为下一节课
          l=0;
          for(j=1; j<=Num_node; j++) 
            if(sgbr_num_copies[j]>0){  l +=sgbr_num_copies[j]; /*printf( "%d--",sgbr_num_copies[j]);*/ } 
         // printf("总拷贝数=%d ",l); //getchar(); 
           
      }//end while( ((rand3+t-1)<=11) && ((classtime[rand3+t-1]-classtime[rand3])<TTL) )
      
      
      printf("\n时间=%ld 本次发包结束\n",t); 
   }//end for(i=1;
                                
                                
   //(7)显示运行结果
   //对延迟预处理
   delay[1]=int( delay[1]*1.0/total_num_interest*10000.0+(total_num_interest-num_delivered[1])*1.0/total_num_interest*TTL );   
   delay[2]=int( delay[2]*1.0/total_num_interest*10000.0+(total_num_interest-num_delivered[2])*1.0/total_num_interest*TTL );   
   delay[3]=int( delay[3]*1.0/total_num_interest*10000.0+(total_num_interest-num_delivered[3])*1.0/total_num_interest*TTL );  
   
   //***add for sgbr
   delay[0]=int( delay[0]*1.0/total_num_interest*10000.0+(total_num_interest-num_delivered[0])*1.0/total_num_interest*TTL ); 
   //***add end
   
   //***revision for sgbr
   //printf("delay:%f %f %f %f\n",delay[1],delay[2],delay[3],delay[0]); 
   //***revision end for sgbr
     
   double ratio1,ratio2,ratio3;
   ratio1=(num_delivered[1]-num_delivered[2])*1.0/total_num_interest;  
   ratio2=(delay[1]-delay[2])/(delay[2]*1.0);
   ratio3=(num_copy[1]*1.0/num_delivered[1]-num_copy[2]*1.0/num_delivered[2])/(num_copy[2]*1.0/num_delivered[2]);
   printf("我们算法:  传递率=%f, 平均延迟=%f 拷贝数目=%f\n", num_delivered[1]*1.0/total_num_interest, delay[1]*1.0, num_copy[1]*1.0/num_delivered[1]);
   printf("3 ratios(相对Epidemic)：%f, %f, %f\n", ratio1, ratio2, ratio3);
   
   ratio1=(num_delivered[1]-num_delivered[3])*1.0/total_num_interest;  
   ratio2=(delay[1]-delay[3])/(delay[3]*1.0);
   ratio3=(num_copy[1]*1.0/num_delivered[1]-num_copy[3]*1.0/num_delivered[3])/(num_copy[3]*1.0/num_delivered[3]);
   printf("3 ratios(相对PS)      ：%f, %f, %f\n\n", ratio1, ratio2, ratio3);
   
   //***add for sgbr
   ratio1=(num_delivered[1]-num_delivered[0])*1.0/total_num_interest;  
   ratio2=(delay[1]-delay[0])/(delay[0]*1.0);
   ratio3=(num_copy[1]*1.0/num_delivered[1]-num_copy[0]*1.0/num_delivered[0])/(num_copy[0]*1.0/num_delivered[0]);
   printf("3 ratios(相对SGBR)    ：%f, %f, %f\n\n", ratio1, ratio2, ratio3);   
   //***add end

   printf("Epidemic:  传递率=%f, 平均延迟=%f 拷贝数目=%f\n\n", num_delivered[2]*1.0/total_num_interest, delay[2]*1.0, num_copy[2]*1.0/num_delivered[2]);

   printf("PS      :  传递率=%f, 平均延迟=%f 拷贝数目=%f\n\n", num_delivered[3]*1.0/total_num_interest, delay[3]*1.0, num_copy[3]*1.0/num_delivered[3]);
 
   //***add for sgbr
   printf("SGBR    :  传递率=%f, 平均延迟=%f 拷贝数目=%f\n\n", num_delivered[0]*1.0/total_num_interest, delay[0]*1.0, num_copy[0]*1.0/num_delivered[0]);
   //***add end
 
   pause();
   printf("\nPress any key to end.");
   return 1;
}//主函数结束 

                              
                       
