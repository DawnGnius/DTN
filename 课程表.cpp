#include "stdio.h"
#include "stdlib.h"
#include "math.h"
#include "time.h"

#define lastline 141979//50299//5000//8000//10000//50000     //�ļ�"1000.txt"����Ŀ 
int data[lastline+1][8];

#define Num_node  10000      //�ڵ���Ŀ
#define Num_packet 800//1000//600//300        //���Ͱ���Ŀ
int total_num_interest = 0;          //�������а�������Ȥ�ڵ����� 
int source;
int received[4][Num_node+1] ;   //�����нڵ��Ƿ��յ���

double delay[4] = {0};                     //Ŀ�Ľڵ��յ���ʱ��
long int num_copy[4] = {0};                  //�����п�����Ŀ
long int num_delivered[4] = {0};             //���ݵ�Ŀ�Ľڵ����Ŀ 
long int classtime[12]={0,  0, 3600,3600*2+600,3600*3+600,   3600*4,3600*5,3600*6,3600*7,      3600*11,3600*12,3600*13};
long int TTL=3600*48; 

//���ӱ���
int curseed[Num_node+1], nextseed[Num_node+1];          //��ǰ�ڿε����ӡ���һ�ڿε����� 
#define Num_classroom 2600                              //���������Ŀ 
int choose_seed_for_class[Num_node+1][Num_classroom+1]; //�ڵ�Ϊ�½ڿε����н���ѡ������  

#define  numofin  10//5  // ��Ȥ�������Ϊ5��
float pinterest = 0.2;//0.1;//0.3;//0.4;//     //�ڵ��ÿ����Ȥ�ĸ���Ȥ�ĸ���
int getpinterest()//����һ���ڵ��Ƿ����Ȥ�ĸ��ʣ�1---�Դ���Ȥ����Ȥ��0---����
{   int num = rand() % 1000 + 1;
    if( num <= 1000 * pinterest )  return 1;   else return 0;
} 
int interest[Num_node+1][numofin+1];  //numofin[i][j]=1��ʾ�ڵ�i����Ȥj��=0��ʾû�д���Ȥ 
int interestpacket[Num_packet+1];     //������Ȥ
int cur_roomid[Num_node+1],next_roomid[Num_node+1];   //��Žڵ�ĳ�ڿ����ڽ���

//*** add for all algorithms 
#define para1 15  //�ڼ��ܷ��� 
#define para2 1//3//5//4//2//1  //�ܼ����� 
#define para3 1  //�ڼ��ڿη��� 
//***add end
int curgap, delta, currand1, currand2, currand3, nextrand1, nextrand2, nextrand3;
//�ڼ��ܡ��ܼ����ڼ��ڿη���

//����ΪPublish/Subscribe(PS)ϵͳ����
int size_win = 50*2*60;        //ʱ�䴰��Ϊ2�ڿ�
int isbroker[Num_node+1];      //1---��broker��0----����
int popular[Num_node+1][2];    //popular[][0],...[1]�ֱ𱣴�ڵ���ǰһ�����ڡ���ǰ����������ֵ 
int otherspopular[Num_node+1][Num_node+1];   //otherspopular[i][j]�ڵ�i����Ľڵ�j��ǰ����ֵ
int ismet[Num_node+1][Num_node+1];           //1---���ڵ㱾ʱ�䴰����������0----����  
int lower_bound =int(0.01*Num_node);//int(0.02*Num_node);// 3; 
int upper_bound =int(0.01*Num_node);//int(0.02*Num_node);//int(0.01*Num_node);//int(0.02*Num_node);//int(0.01*Num_node);//int(0.02*Num_node);//int(0.1*Num_node);//12;//9;//3;//8;//        //���½� 
int metbrokers[Num_node+1];                  //��ǰ�����ڣ��ڵ�������brokers��Ŀ 
//end for PS 



//���¿��ǿμ���ϵ����
float pout = 0.2;//0.25;//0.1;//0.3;// 0.5;//    //�μ��߳����Ҹ���
int   Num_place = 2;//4;  //�μ�ÿ��¥�㹫������� 
int getprob()//����һ���ڵ�μ��Ƿ��ȥ��1---��ȥ��0---����ȥ
{   int num = rand() % 1000 + 1;
    if( num <= 1000 * pout )  return 1;   else return 0;
} 
int outroom[Num_node+1];    //�ڵ�μ��ȥ��1--��ȥ��0--���� 
int place[Num_node+1];      //�ڵ�μ��ȥ���ڵ��ȵ�����  
int Iscontact_outroom(int nodeid1, int nodeid2 )//�жϽڵ�nodeid1��nodeid2�¿��ڼ��Ƿ���ͬһ¥�㡢�߳����ҡ�ȥͬһ���ȵ� 
{  int classroom1 =cur_roomid[nodeid1], classroom2 =cur_roomid[nodeid2]; 
   if( ( classroom1 > 0 ) && ( classroom2 > 0 ) && ( (classroom1/100) == (classroom2/100) ) 
         && ( outroom[nodeid1]==1 ) && ( outroom[nodeid2]==1) && (place[nodeid1]==place[nodeid2])     )    
        {    //printf("%d---%d",classroom1,classroom2);  getchar();      
             return 1;   
        }
   return 0;
}
//end 


int Haveclass(int nodeid, int week, int day, int classnum)//�ж�ѧ��nodeid��week������day��classnum�Ƿ��п� 
{                                                         //�ҵ�----����1�����򷵻�0
   int i, j, flag1=0; //flag��ʼΪ0����ʾû���ҵ�                                      
   for(i=1; i<=lastline; i++)
 // if(i==1455)
   {//  printf("i=%d",i);  printf("��������:%d  %d ",data[i][1], nodeid);
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
                           {     flag1 = 1;  break; } //�ҵ�����Ҫ�����һ�� 
                       } 
              }//end else
   }//end for
  
   return flag1;   //���ز��ҽ�� 
}  
   
int returnclassroom(int nodeid, int week, int day, int classnum)//�ж�ѧ��nodeid��week������day��classnum�Ƿ��п� 
{                                                         //�ҵ�----���ؽ���id�����򷵻�0
   int i, j, flag1=0; //flag��ʼΪ0����ʾû���ҵ�                                      
   for(i=1; i<=lastline; i++)
 // if(i==1455)
   {//  printf("i=%d",i);  printf("��������:%d  %d ",data[i][1], nodeid);
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
                           {     flag1 = data[i][7];  break; } //�ҵ�����Ҫ�����һ�� 
                       } 
              }//end else
   }//end for
              
   return flag1;   //���ز��ҽ�� 
}
int Iscontact(int classroom1, int classroom2 )//�жϽ���classroom1��classroom2�Ƿ�Ϊͬһ������ //�жϽڵ�nodeid1��nodeid2�ڵ�week�ܣ�����day,��classnum�ڿΣ�
{  if( ( classroom1 > 0 ) && ( classroom1 == classroom2 ) )    return 1;
   return 0;
}


int oneofinterest(int interestid, int nodeid)
{                                            //�Ƿ���1�����򷵻�0 
   int i;
   //for(i=1; i<=numofin; i++) 
   //{  if(interest[nodeid][i]==1) && (interestpacket[packetid]==1) ) }   
   if( interest[nodeid][interestid] == 1 ) return 1; else return 0;
} 

int absor(int i,int j,int interestid)   //�ڵ�i��j֮��������㷨������1--�ڵ�i����j������0--�ڵ�j����i  
{  if(i==source) return 1; //�ڵ�i����j 
   if(j==source) return 0; //�ڵ�j����i
    
   if( (oneofinterest(interestid,i)==1) && (oneofinterest(interestid,j)==0) )
       return 1;  //�ڵ�i����j 
   else if( (oneofinterest(interestid,i)==0) && (oneofinterest(interestid,j)==1) ) 
             return 0;   //�ڵ�j����i 
             
   if(i>j) return 1;  else return 0;
}  

int absor_action(int i, int j, int interestid) //���ӽڵ�i��j֮��ִ�����ն���,������������û�б�д�������������������� 
{   int k, direction = absor(i,j,interestid);
    if(direction == 1)  //�ڵ�i����j 
    {    curseed[j] = 0;
         for(k=1; k<=Num_classroom; k++)  
             if( ( choose_seed_for_class[i][k] == 0 ) && ( choose_seed_for_class[j][k] > 0 ) )  choose_seed_for_class[i][k] = choose_seed_for_class[j][k];
             else  if( choose_seed_for_class[j][k] > 0 )  nextseed[ choose_seed_for_class[j][k] ] = 0; //����ڵ�jѡ����½ڿ����ӣ��������ˣ���Ϊ�ڵ�iѡ��� 
         for(k=1; k<=Num_classroom; k++)   choose_seed_for_class[j][k] = 0; //��ʾ�ڵ�jû��Ϊ�κν��ң�ѡ���½ڿ����� 
         curseed[j] = 0;  //�ڵ�j�����ǵ�ǰ�ε����� 
    }
    else                //�ڵ�j����i
    {    curseed[i] = 0;
         if( ( choose_seed_for_class[j][k] == 0 ) && ( choose_seed_for_class[i][k] > 0 ) )  choose_seed_for_class[j][k] = choose_seed_for_class[i][k];
             else  if( choose_seed_for_class[i][k] > 0 )  nextseed[ choose_seed_for_class[i][k] ] = 0; //����ڵ�iѡ����½ڿ����ӣ��������ˣ���Ϊ�ڵ�jѡ���
         for(k=1; k<=Num_classroom; k++)   choose_seed_for_class[i][k] = 0; //��ʾ�ڵ�iû��Ϊ�κν��ң�ѡ���½ڿ����� 
         curseed[i] = 0;  //�ڵ�i�����ǵ�ǰ�ε�����
    }
    return 0;
} 

void statistic(int algonum, int nodeid, int pakcetinterest, int duration)
{   //���㷨algonum�У��ڵ�nodeid�յ�һ�����������ݵ�ǰ������Ȥpakcetinterest���㷨�Ŀ�������������Ȥ�޹أ����ӳ١����������ۼ�
    num_copy[algonum] ++;
    if( oneofinterest(pakcetinterest,nodeid) == 1 ) 
    {    delay[algonum] += duration/10000.0;   num_delivered[algonum] ++;  }   
    /*if(algonum==0) 
    {   printf("�㷨%d �ڵ�%d�յ���,�㷨��������=%d ���ݸ���=%d*****************************", algonum,nodeid, num_copy[algonum], num_delivered[algonum] );   
        getchar();
    } */                                                        
} 
int haveinterest(int nodeid)
{
      for(int i=1; i<=numofin; i++)  if( interest[nodeid][i] == 1) return 1;
      return 0;
}

void pause()
{    printf("��ͣ,�������������");    getchar(); return;
}

//***add for SGBR·���㷨 
//int step=360;//300;//600;//300;//480;//180;   //������5����
float connected_d[Num_node+1][Num_node+1];            //��¼�ڵ�֮����ͨ���� 
float last_contact_time[Num_node+1][Num_node+1];      //��¼�ڵ�֮����һ����ϵ���ʱ�� 
//int recent_contact_time[Num_node+1][Num_node+1];    //��¼�ڵ�֮�����һ����ϵʱ�� 
//int (*recent_contact_time)[Num_node+1] = new int[Num_node+1][Num_node+1]; 
float *recent_contact_time[Num_node+1];
float sgbr_updating_factor=0.45, sgbr_aging_constant=0.98, sgbr_cth=0.5;//0.6;//0.3;//0.4;//0.2;//0.75;//0.65;// 
int sgbr_copies=int(3*Num_node);   //32;//24;//24;//30%#nodes //12;//   //��������
int   sgbr_num_copies[Num_node+1];                    //ÿ���ڵ㴦������Ŀ 

int Isnextrelay_sgbr(int i, int j, int interestid)  //��SGBR�㷨�У��ڵ�i�Ƿ�ѡ��ڵ�j��Ϊ��һ���м̽ڵ�
{   if( oneofinterest(interestid, j) == 1 ) return 1;   //if(j==dest) return 1;
    int k, u, v, w;
    //printf("�����ڵ�%d,%d��ͨϵ��=%.2f  ", i, j, connected_d[i][j]); //char ch=getchar();
    //if( (connected_d[i][j]<sgbr_cth) && (sgbr_num_copies[i]>1) && (last_contact_time[i][j]>0) )
    if( (connected_d[i][j]<sgbr_cth) && (sgbr_num_copies[i]>1)  ) 
    {   //printf("�����ڵ�%d,%d��ͨϵ��=%.2f ", i, j, connected_d[i][j]); //char ch=getchar();
        return 1;
    }

	return 0;
}
int times=15;   //ÿ�ο��㼸��ʱ�䵥Ԫ 
//***add end

int main()
{

    long	int i,j,k,l, val1, val2, val3, val4, val;
	char fname[10000];
	int curgap, nextgap;
    
    
    //***add for sgbr ���ھ�̬�����ά������ռ�̫����������Զ�̬�����ά���� 
    for(i=0; i<=Num_node; i++)   recent_contact_time[i] = new float[Num_node+1];
	//***add end
	
	
	
	//(1)���ļ��ж������ݵ����� Data[][]
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
    // printf("һ����%d ", data[13941][1]); 


   //������������нڵ����Ȥ����
   for(k=1; k<=Num_node; k++) 
         //for(i=1; i<=numofin; i++) {    interest[k][i] = rand()%2;   }               //���κ���Ȥ����Ȥ�ĸ���Ϊ0.5 
         for(i=1; i<=numofin; i++) {    interest[k][i] = getpinterest();   }   //���κ���Ȥ����Ȥ�ĸ���Ϊpinterest 
   // printf("�ڵ���Ȥ���ϣ�"); 

   for(i=1;i<=Num_packet; i++)
   {
          printf("��%d����\n", i); 
          
          //***revision ���渳ֵ����Ƶ�����ǰ��whileѭ�� 
          currand1=para1, currand2=para2, currand3=para3;   //���÷�������ʱ�� 
          //***revision end
          
          //��ʼ��
          int s1 = rand()%Num_node+1;
          //s1=47;  Haveclass(s1, rand1, rand2, rand3);
          while( ( Haveclass(s1, currand1, currand2, currand3) == 0 )  || ( haveinterest(s1) == 0 )  ) 
          {    /*printf("����Դ�ڵ�.");*/    s1 = rand()%Num_node+1;   }
          source = s1;
           
          printf("Դ�ڵ�=%d %d ", source, haveinterest(s1) );  
          //getchar();
          
          //*** revision set j=0 not j=1 for sgbr       
          for(j=0; j<=3; j++)   for(k=1; k<=Num_node; k++) received[j][k]=0;  //���нڵ�û���յ��� 
          for(j=0; j<=3; j++)   received[j][source] = 1;                      //Դ�ڵ��յ��� 
          for(j=0; j<=3; j++)   statistic(j, source, interestpacket[i], 0 ); 
          //***revision end 
          
          for(k=1; k<=Num_node; k++)    curseed[k]=nextseed[k]=0;     //���нڵ㶼�������� 
          nextseed[source] = 1;                                       //Դ�ڵ��ʼ��Ϊ���� 
          
          
          for(k=1; k<=numofin; k++)  if(interest[source][k]==1) {   interestpacket[i] = interest[source][k];  printf("ok!");  break;  }
          //interestpacket[i] = rand()%numofin +1;  //����i���ѡ��һ����Ȥ 
          printf("������Ȥ=%d ", interestpacket[i]); 
          //printf("���=%d ", oneofinterest(interestpacket[i],8) );
          //printf("���=%d ", absor(2,3,interestpacket[i]) );
          for(k=1; k<=Num_node; k++) if( oneofinterest(interestpacket[i],k) == 1 ) total_num_interest++;    //�ۻ��԰�i����Ȥ�Ľڵ���Ŀ 
         
          for(k=1; k<=Num_node; k++)    isbroker[k] = 0;                     //���нڵ㶼����broker
          for(k=1; k<=Num_node; k++)    popular[k][0] = popular[k][1] = 1;   //���нڵ��ǰһ��ʱ�䴰�ں͵�ǰ��������ֵ����     
          for(k=1; k<=Num_node; k++)    for(j=1; j<=Num_node; j++)  otherspopular[k][j] = 0; //�ڵ�û�б��������ڵ���ǰʱ�䴰������ֵ 
          for(k=1; k<=Num_node; k++)    for(j=1; j<=Num_node; j++)  ismet[k][j] = 0;         //�κ������ڵ�û�������� 
          for(k=1; k<=Num_node; k++)    metbrokers[k] = 0;   //���нڵ㶼û������brokers 
          
          //getchar(); 
          
          
          //***add for sgbr, ��ʼ��
          for(j=1; j<=Num_node; j++)     
              for(k=1; k<=Num_node; k++){  connected_d[j][k]=recent_contact_time[j][k]=0;    last_contact_time[j][k]=-1;   } 
          for(j=1; j<=Num_node; j++)     sgbr_num_copies[j]=0;     
          sgbr_num_copies[source]=sgbr_copies; //printf("***%d*** ",sgbr_num_copies[source]); pause(); 
          //***end add
         
         
          //���Ͱ�
          int flag=0; //Ĭ��û�յ�  
          int loop,week;
          int  t = 1; 
          
          //***add for sgbrѵ������ʱ��
          //currand1=para1, currand2=para2, currand3=para3; 
          currand1--;
          if(currand1>=1)//ѵ��ǰһ��ʱ�� 
          while( currand1 <= (para1-1) )//while( (currand1<=(para1-1)) && (currand2<=5) && (currand3<=11) )
          {
             printf("t=%d \n",t); 
            
             //step1:������һ�ڿ���� 
             if(currand3==11)//��ǰΪһ�����һ�ڿ� 
             {    if(currand2==5) //����Ϊ���� 
                  {    delta = (24*3600-(classtime[11]+8*3600+50*60) )+24*3600*2+8*3600;
                       nextrand1=currand1+1;  nextrand2=1; nextrand3=1;
                  }    
                  else//���첻������ 
                  {    delta = (24*3600-(classtime[11]+8*3600+50*60) )+8*3600;
                       nextrand1=currand1;  nextrand2=currand2+1;  nextrand3=1;
                  }     
             } 
             else
             {    delta = classtime[currand3+1]-classtime[currand3];
                  nextrand1=currand1;  nextrand2=currand2;    nextrand3=currand3+1;
             }
             

            //step1:�����ǰ�����нڵ�Ľ���id
            for(j=1; j<=Num_node; j++)    
            //{    cur_roomid[j] = returnclassroom(j, rand1,rand2,rand3+t-1); next_roomid[j] = returnclassroom(j, rand1,rand2,rand3+t);  }
              {    cur_roomid[j]  = returnclassroom(j, currand1,currand2,currand3); 
                   next_roomid[j] = returnclassroom(j, nextrand1,nextrand2,nextrand3);  
              }

  
            //�Ͽ��ڼ���½ڵ�֮����ͨ��* 
             for(j=1; j<=Num_node; j++)
                for(k=1; k<=Num_node; k++)//for(k=1; k<=Num_node; k++) if(k!=j)     
                if(k!=j) 
                {    
                     if( Iscontact(cur_roomid[j], cur_roomid[k]) > 0 )  //�ڵ�j��k���ڿ���ͬһ������(����)�����а�����ɢ
                     {                         
                        if( k >= (j+1) )
                        { 
                          //����sgbr�ڵ������� 
                          //if( t % step == 0 )  
                          {   if( (t-recent_contact_time[j][k]) >1 )                 //(1)������һ����ϵ���ʱ�� 
                              {    last_contact_time[j][k] = last_contact_time[k][j] = recent_contact_time[j][k];    }
                              recent_contact_time[j][k]=recent_contact_time[k][j]= t; //(2)�������һ����ϵʱ��Ϊ��ǰ
                              if(last_contact_time[j][k]>0)                                      //(3)�ӵڶ�����ϵ��ʼ������ͨϵ�� 
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
            //�Ͽ��ڼ���� 
                
               
            //�μ���½ڵ�֮����ͨ��
            //if( (curgap+50*60) < TTL ) //���������ڻ�û�н���
            { for(j=1; j<=Num_node; j++)  {  outroom[j] = getprob();  place[j] = rand()%Num_place+1;  }
            
              for(j=1; j<=Num_node; j++)
                for(k=1; k<=Num_node; k++)   
                if(k!=j) 
                {    //printf("�ڵ�%d<--->�ڵ�%d\n",j,k); 
                     if( Iscontact_outroom(j, k )==1 ) //�ڵ�j��k�μ���ͬһ¥�㡢���߳����ҡ�ȥͬһ���ȵ����򣬽��а���ɢ
                     {  
                        if( k >= (j+1) )//����ִ��Epidemic��PS�����㷨 
                        { 
                          //����sgbr�ڵ������� 
                          //if( t % step == 0 )  
                          {   if( (t-recent_contact_time[j][k]) >1 )                 //(1)������һ����ϵ���ʱ�� 
                              {    last_contact_time[j][k] = last_contact_time[k][j] = recent_contact_time[j][k];    }
                              recent_contact_time[j][k]=recent_contact_time[k][j]= t; //(2)�������һ����ϵʱ��Ϊ��ǰ
                              if(last_contact_time[j][k]>0)                                      //(3)�ӵڶ�����ϵ��ʼ������ͨϵ�� 
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
          //(2/2)�μ����               
          
          
          currand1 = nextrand1;    currand2 = nextrand2;   currand3 = nextrand3;
          //curgap += delta;
  
          t++;    //t��Ϊ��һ�ڿ�
          
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
      ///for(j=1; j<=Num_node; j++) //???���ýڵ�֮��������������������ǰ�������������ֻ�����ڵ�֮����ͨ�ȣ�    
      //        for(k=1; k<=Num_node; k++){  connected_d[j][k]=recent_contact_time[j][k]=0;    last_contact_time[j][k]=-1;   }
      //***add end   
          
          
          
          //***revision ��ֵ����ǰ���Ƶ����� 
          printf("�����:t=%d currand1=%d currand2=%d currand3=%d \n",t, currand1, currand2,currand3,curgap); 
          currand1=para1, currand2=para2, currand3=para3;   //���÷�������ʱ�� 
          //t = 1;
          //***revision end
          
          int  num_win_ps=0;  
          curgap = 0; //��Ϊû����һ�ڿΣ����Ժ���һ�ڿ�ʱ�����Ϊ0 
          while( (currand1<=18) && (curgap<TTL) )
          {
//             printf("t=%d currand1=%d currand2=%d currand3=%d curgap=%d(%d) \n",t, currand1, currand2,currand3,curgap, curgap/3600); 
//             printf("t=%d \n",t); 
             //pause();     
             
             //step1:������һ�ڿ���� 
             if(currand3==11)//��ǰΪһ�����һ�ڿ� 
             {    if(currand2==5) //����Ϊ���� 
                  {    delta = (24*3600-(classtime[11]+8*3600+50*60) )+24*3600*2+8*3600;
                       nextrand1=currand1+1;  nextrand2=1; nextrand3=1;
                  }    
                  else//���첻������ 
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

            //step1:�����ǰ�����нڵ�Ľ���id
            for(j=1; j<=Num_node; j++)    
            //{    cur_roomid[j] = returnclassroom(j, rand1,rand2,rand3+t-1); next_roomid[j] = returnclassroom(j, rand1,rand2,rand3+t);  }
              {    cur_roomid[j]  = returnclassroom(j, currand1,currand2,currand3); 
                   next_roomid[j] = returnclassroom(j, nextrand1,nextrand2,nextrand3);  
              }

             //step2�������㷨���ִ���----���õ�ǰ�ڿ����ӡ���ǰ�ڿ�����Ϊ�½ڿ��Լ�ȥ�Ľ��ң������Լ�Ϊ���� 
             for(j=1; j<=Num_node; j++)  if(nextseed[j]==1) curseed[j]=1;   else curseed[j]=0; //һ�������ʹ���Ͻڿ����õ����ӣ����µ�ǰ����״̬
             val = cur_roomid[source]; //val = returnclassroom(source, rand1,rand2,rand3+t-1); //���һ�����������Դ�ڵ��п����Ͻڿ�û�пΣ���ڿο�ʼ�пΣ���Ȼ��Ϊ���� 
             if(val>0)  curseed[j] = 1;
             
             for(j=1; j<=Num_node; j++) //nextseed[j] = 0; 
                  if(cur_roomid[j]>0) nextseed[j] = 0;     //ʹ����ǰ��ѡΪ���ӵĽڵ㣬��ʹ��ǰ��û�пΣ��½ڿ�����Ϊδ������    
             //if( (rand3+t) < 11 )
             {   for(j=1; j<=Num_node; j++)  for(k=1; k<=Num_classroom; k++)  choose_seed_for_class[j][k] = 0;//��ǰ�ڿΣ����нڵ㶼û��Ϊ�½ڿεĽ���ѡ������ 
                 for(j=1; j<=Num_node; j++)
                     if( curseed[j]==1)    //��ǰ�ڿ������ӣ��½ڿ��пεĻ�������ѡ���Լ���Ϊ�½ڿ�����
                     {   val = next_roomid[j]; //val = returnclassroom(j, rand1,rand2,rand3+t);  
                         if( val>0 ) { choose_seed_for_class[ j ][ val ] = j;  nextseed[j]=1;  }  //
                     }       
             }       
             
              
            //step3��Publish/Subscribe(PS)�㷨���ִ��� 
            if( ( curgap - num_win_ps*size_win ) >= 0 ) //if( ( (classtime[rand3+t-1]-classtime[rand3]) % size_win ) == 0 )  //��һ��ʱ�䴰�ڿ�ʼ
            {   //printf("��һ��ʱ�䴰�ڿ�ʼ:\n"); 
                num_win_ps++;
                for(k=1; k<=Num_node; k++)   {   popular[k][0] = popular[k][1];    popular[k][1] = 0;   }  //������һʱ�䴰������ֵ 
                for(j=1; j<=Num_node; j++)   for(k=1; k<=Num_node; k++)    ismet[j][k] = 0;     //�ڵ�֮������������
                for(j=1; j<=Num_node; j++)   metbrokers[j] = 0;    //���нڵ㶼û������brokers         
                
            }
            
          
            
            
            //step4�����ڵ�����������ɢ
            //(1/2)�Ͽ��ڼ��ͨ�ſ�ʼ************************************************************************************************* 
             for(j=1; j<=Num_node; j++)
                for(k=1; k<=Num_node; k++)//for(k=1; k<=Num_node; k++) if(k!=j)     
                if(k!=j) 
                {    //printf("�ڵ�%d<--->�ڵ�%d\n",j,k); 
                     //val1 = returnclassroom(j, rand1,rand2,rand3+t-1);     val2 = returnclassroom(k, rand1,rand2,rand3+t-1);
                     if( Iscontact(cur_roomid[j], cur_roomid[k]) > 0 ) //if( Iscontact(val1, val2) > 0 ) //�ڵ�j��k���ڿ���ͬһ������(����)�����а�����ɢ
                     {   
                       //�����㷨��ʼ************************************************************************************************************************ 
                       if(curseed[j]==1)//�ڵ�jΪ����
                       {  //val3 = returnclassroom(j, rand1,rand2,rand3+t);   val4 = returnclassroom(k, rand1,rand2,rand3+t);
                         //case 1:�ڵ�j�ͽڵ�k������һ������ͬһ����
                         //if( ((rand3+t)<11) && (Iscontact(next_roomid[j], next_roomid[k])>0) ) //if( ((rand3+t)<11) && (Iscontact(val3, val4)>0) )
                         if( (Iscontact(next_roomid[j], next_roomid[k])>0) )
                         {   if( (received[1][k]==1) && (curseed[k]==1) ) 
                             {    absor_action(j, k, interestpacket[i]);   //printf("case1.1:�ڵ�%d<->%d�������� ",j,k);  pause();  
                             } //ִ�����ն��� 
                             else if( (received[1][k]==0) && (oneofinterest(interestpacket[i],k)==1) )   
                                  {  received[1][k]=1;     statistic(1, k, interestpacket[i], curgap );//statistic(1, k, interestpacket[i], classtime[rand3+t-1]-classtime[rand3] );    
                                     //printf("case1.2:�ڵ�%d�յ��������Խڵ�%d ",k,j); pause();  
                                  }
                         }
                         else //case 2:����,���пε�����ͬһ���� 
                             //if( ((rand3+t)<11) && (next_roomid[j]>0) && (next_roomid[k]>0) && (next_roomid[j]!=next_roomid[k]) ) //if( ((rand3+t)<11) && (val3>0) && (val4>0) && (val3!=val4) )
                             if( (next_roomid[j]>0) && (next_roomid[k]>0) && (next_roomid[j]!=next_roomid[k]) )
                             {   if( received[1][k]==1) //case 2.1:�ڵ�k�п��� 
                                 {    if(curseed[k]==1) //�ڵ�k������ 
                                      {    val = absor_action(j, k, interestpacket[i]);   //ִ�����ն���
                                           //printf("case2.1:�ڵ�%d<->%d�������� ",j,k);  pause(); 
                                           if(val==1)//�ڵ�j����k 
                                           {  if( choose_seed_for_class[j][ next_roomid[k] ]==0 ) { nextseed[k]=1;  choose_seed_for_class[j][ next_roomid[k] ]==k;}  }
                                           else      //�ڵ�k����j 
                                           {  if( choose_seed_for_class[k][ next_roomid[j] ]==0 ) { nextseed[j]=1;  choose_seed_for_class[j][ next_roomid[j] ]==j;} } 
                                      }
                                      else  //�ڵ�k�������� 
                                      {  if( choose_seed_for_class[j][next_roomid[k]]==0 ) { nextseed[k]=1;  choose_seed_for_class[j][next_roomid[k]]==k; }   }
                                 }
                                 else //case 2.2:�ڵ�kû�п��� 
                                 {  //printf("case2.2:");  pause(); 
                                    if( oneofinterest(interestpacket[i],k)==1)  
                                    //{   received[1][k]=1;  statistic(1, k, interestpacket[i], classtime[rand3+t-1]-classtime[rand3] );     }
                                    {   received[1][k]=1;  statistic(1, k, interestpacket[i], curgap );     }
                                    if( choose_seed_for_class[j][ next_roomid[k] ]==0 )  //֮ǰ��û��Ϊj�½ڿν���ѡ�����ӣ�����ѡ��j��Ϊ���� 
                                    {   nextseed[k]=1;   choose_seed_for_class[j][ next_roomid[k] ]==k;                                 }
                                 }
                             }
                             //else if( ((rand3+t)<11) && (next_roomid[j]>0) && (next_roomid[k]==0) )//case 3:�ڵ�j�½��пΣ��ڵ�kû�� 
                             else if( (next_roomid[j]>0) && (next_roomid[k]==0) )
                                  {   //printf("case3:");  pause(); 
                                      if( (received[1][k]==0) && (oneofinterest(interestpacket[i],k)==1) )   
                                      //{     received[1][k]=1;    statistic(1, k, interestpacket[i], classtime[rand3+t-1]-classtime[rand3] );     }
                                      {     received[1][k]=1;    statistic(1, k, interestpacket[i], curgap );     }   
                                  }    //�ڵ�kû�п���������Ȥ���Ϳ���һ�� 
                                  else//case 4:�ڵ�j�½�û�пΣ��ڵ�k��    
                                  {   //printf("case4:");  pause(); 
                                      if( (received[1][k]==0) && (oneofinterest(interestpacket[i],k)==1) )  //ͬ��... 
                                      //{     received[1][k]=1;    statistic(1, k, interestpacket[i], classtime[rand3+t-1]-classtime[rand3] );     } 
                                      {     received[1][k]=1;    statistic(1, k, interestpacket[i], curgap );     }     
                                      if( choose_seed_for_class[j][next_roomid[k]]==0 )  //֮ǰ��û��Ϊj�½ڿν���ѡ�����ӣ�����ѡ��j��Ϊ���� 
                                      {     nextseed[k]=1;   choose_seed_for_class[j][next_roomid[k]]==k;  }
                                      //if( j!=source )    curseed[j]=0;            //�ڵ�j����Դ�ڵ㣬���ýڵ�j��ǰ�������ӽڵ� 
                                  }                         
                         }//end if(curseed[j]==1)//�ڵ�jΪ����
                        //�����㷨����***********************************************************************************************************************
                          
                  
                          
                        if( k >= (j+1) )//����ִ��Epidemic��PS�����㷨 
                        { 
                          //Epidemic�㷨��ʼ*************************************************************************************************************** 
                          if( (received[2][j]==1) && (received[2][k]==0)  )  //Я������������ڵ� 
                          {   received[2][k]=1;  statistic(2,k,interestpacket[i],curgap);//statistic(2,k,interestpacket[i],classtime[rand3+t-1]-classtime[rand3]);  
                              //printf("case(2):Я����%d�����ڵ�-%d ",j,k); //pause();
                          }
                          else if( (received[2][k]==1) && (received[2][j]==0)  ) //Я������������ڵ�
                          {   received[2][j]=1;  statistic(2,j,interestpacket[i],curgap);//statistic(2,j,interestpacket[i],classtime[rand3+t-1]-classtime[rand3]);  
                              //printf("case(2):Я����%d�����ڵ�-%d ",k,j); //pause();
                          }
                          //���´����������ѡһ��Я���߽�������������Ȥ�ڵ㣬�Ų������� 
                
                         
                          //Publish/Subscribe(PS)�㷨��ʼ*************************************************************************************************** 
                          if(ismet[j][k]==0){ popular[j][1]++; if(isbroker[k]==1) metbrokers[j]++; }       ismet[j][k] = 1; //1:���½ڵ�ĵ�ǰʱ�䴰������ֵ����ʶ������ 
                          if(ismet[k][j]==0){ popular[k][1]++; if(isbroker[j]==1) metbrokers[k]++; }       ismet[k][j] = 1;
                          
                          otherspopular[j][k] = popular[k][0];   otherspopular[k][j] = popular[j][0];//2:���½ڵ㱣��������ڵ���һ��ʱ�䴰������ֵ   
                     
                          if( (popular[j][1]<lower_bound) && (isbroker[k]==0) )   isbroker[k] = 1;//3_1������ͨ�ڵ�ѡΪbroker 
                          if( (popular[k][1]<lower_bound) && (isbroker[j]==0) )   isbroker[j] = 1;//3_1������ͨ�ڵ�ѡΪbroker
                          
                          if( (metbrokers[j]>=upper_bound) && (isbroker[k]==1) )  isbroker[k] = 0;//3_2����brokerѡΪ��ͨ�ڵ�
                          if( (metbrokers[k]>=upper_bound) && (isbroker[j]==1) )  isbroker[j] = 0;//3_2����brokerѡΪ��ͨ�ڵ�
                          
                          //4��������ɢ
                          if((j==source)&&(isbroker[k]==1)&&(received[3][k]==0))//Դ����broker
                          {   received[3][k]=1;  statistic(3,k,interestpacket[i],curgap);//statistic(3,k,interestpacket[i],classtime[rand3+t-1]-classtime[rand3]);  
                              //printf("case4.1:Դ����broker-%d ",k); //pause();
                          } 
                          if((k==source)&&(isbroker[j]==1)&&(received[3][j]==0))//ͬ��
                          {   received[3][j]=1;  statistic(3,j,interestpacket[i],curgap);//statistic(3,j,interestpacket[i],classtime[rand3+t-1]-classtime[rand3]);  
                              //printf("case4.1:Դ����broker-%d ",j); //pause();
                          }
                          if((isbroker[j]==1)&&(received[3][j]==1)&&(isbroker[k]==1)&&(received[3][k]==0))//broker����broker
                          {   received[3][k]=1;  statistic(3,k,interestpacket[i],curgap);//statistic(3,k,interestpacket[i],classtime[rand3+t-1]-classtime[rand3]);  
                              //printf("case4.2:broker%d����broker-%d ",j,k); //pause();
                          }
                          if((isbroker[k]==1)&&(received[3][k]==1)&&(isbroker[j]==1)&&(received[3][j]==0))//ͬ��
                          {   received[3][j]=1;  statistic(3,j,interestpacket[i],curgap);//statistic(3,j,interestpacket[i],classtime[rand3+t-1]-classtime[rand3]);  
                              //printf("case4.2:broker%d����broker-%d ",k,j); //pause();
                          }
                          if((isbroker[j]==1)&&(received[3][j]==1)&&(isbroker[k]==0)&&(k!=source)
                          &&(received[3][k]==0)&&(oneofinterest(interestpacket[i],k)==1))//broker��������Ȥ�ڵ� 
                          {   received[3][k]=1;  statistic(3,k,interestpacket[i],curgap);//statistic(3,k,interestpacket[i],classtime[rand3+t-1]-classtime[rand3]);  
                              //printf("case4.3:broker%d������Ȥ�ڵ�-%d ",j,k); //pause();
                          }
                          if((isbroker[k]==1)&&(received[3][k]==1)&&(isbroker[j]==0)&&(j!=source)
                          &&(received[3][j]==0)&&(oneofinterest(interestpacket[i],j)==1))//broker��������Ȥ�ڵ� 
                          {   received[3][j]=1;  statistic(3,j,interestpacket[i],curgap);//statistic(3,j,interestpacket[i],classtime[rand3+t-1]-classtime[rand3]);  
                              //printf("case4.3:broker%d��������Ȥ�ڵ�-%d ",k,j); //pause();
                          }      /**/  
                          //PS�㷨����********************************************************************************************* 
            
                          
                          //***add for sgbr                           
                          if( (received[0][j]==1) && (sgbr_num_copies[j]>=1) && (received[0][k]==0) && (oneofinterest(interestpacket[i],j)==0)
                              && (oneofinterest(interestpacket[i],k)==1) )//Я����j(������Ȥ��1 ������)��������Ȥ�ڵ㣬ת�� 
                          {   received[0][k]=1;  sgbr_num_copies[k]=sgbr_num_copies[j]; sgbr_num_copies[j]=0; statistic(0,k,interestpacket[i],curgap);  
                              //printf("case(0)1.1:Я����%d������Ȥ�ڵ�--%d ",j,k); pause();
                          }
                          else if( (received[0][k]==1) && (sgbr_num_copies[k]>=1) && (received[0][j]==0) && (oneofinterest(interestpacket[i],k)==0)
                              && (oneofinterest(interestpacket[i],j)==1) )//������Գƣ�Я����k(������Ȥ��1 ������)��������Ȥ�ڵ㣬ת�� 
                          {   received[0][j]=1;  sgbr_num_copies[j]=sgbr_num_copies[k]; sgbr_num_copies[k]=0;  statistic(0,j,interestpacket[i],curgap);  
                              //printf("case(0)1.2:Я����%d������Ȥ�ڵ�--%d ",k,j); pause();
                          }
     
                          else if( (received[0][j]==1) && (sgbr_num_copies[j]>1) && (received[0][k]==0) && (Isnextrelay_sgbr(j,k,interestpacket[i])==1)  )
                          //Я����j(����1������)����û���յ������ҷ��������ڵ㣬ת��    
                          {   received[0][k]=1;   l = sgbr_num_copies[j]; sgbr_num_copies[j]=l/2; sgbr_num_copies[k]=l-sgbr_num_copies[j]; 
                              //printf("(1)%d %d %d  ",l,sgbr_num_copies[j],sgbr_num_copies[k]); pause(); 
                              statistic(0,k,interestpacket[i],curgap); 
                              //printf("case(0)2.1:Я����%d����û�п����ڵ�-%d ",j,k); pause();
                          }
                          else if( (received[0][k]==1) && (sgbr_num_copies[k]>1) && (received[0][j]==0) && (Isnextrelay_sgbr(k,j,interestpacket[i])==1)  )
                          //������Գƣ�Я����k(����1������)����û���յ������ҷ��������ڵ㣬ת��    
                          {   received[0][j]=1;   l = sgbr_num_copies[k]; sgbr_num_copies[k]=l/2; sgbr_num_copies[j]=l-sgbr_num_copies[k]; 
                              //printf("(1)%d %d %d  ",l,copies_sw[j],copies_sw[k]); pause(); 
                              statistic(0,j,interestpacket[i],curgap);
                              //printf("case(4)2.1:Я����%d����û�п����ڵ�-%d ",j,k); //pause();
                          }   
                          //����sgbr�ڵ������� 
                          //if( t % step == 0 )  
                          {   if( (t-recent_contact_time[j][k]) >1 )                 //(1)������һ����ϵ���ʱ�� 
                              {    last_contact_time[j][k] = last_contact_time[k][j] = recent_contact_time[j][k];    }
                              recent_contact_time[j][k]=recent_contact_time[k][j]= t; //(2)�������һ����ϵʱ��Ϊ��ǰ
                              if(last_contact_time[j][k]>0)                                      //(3)�ӵڶ�����ϵ��ʼ������ͨϵ�� 
                              {    double tempt_f = pow(sgbr_aging_constant, (t-last_contact_time[j][k])*times);
                                   double ddd=tempt_f;
                                   tempt_f = connected_d[j][k]*tempt_f + (1.0-connected_d[j][k]*tempt_f)*sgbr_updating_factor; 
                                   connected_d[j][k] = connected_d[k][j] = tempt_f;
                                   if(connected_d[j][k]>1) 
                                   { printf("ERROR! tempt_f=%.2f, %.2f %.2f ",tempt_f, (t-last_contact_time[j][k])*times, ddd); pause(); } 
                              }
                          }    
                          //***add end for sgbr
                                     
            
            
            
            
                         }//if( k >= (j+1) )//����ִ��Epidemic��PS�����㷨  
                                         
                     }//if( Iscontact(val1, val2) > 0 ) 
                            
                }//for(k=(j+1); k<=Num_node;
           //(1/2)�Ͽ��ڼ��ͨ�Ž���************************************************************************************************* 
                
                
           //(2/2)�μ�ͨ�ſ�ʼ***************************************************************************************************** 
            if( (curgap+50*60) < TTL ) //���������ڻ�û�н���
            { for(j=1; j<=Num_node; j++)  {  outroom[j] = getprob();  place[j] = rand()%Num_place+1;  }
            
              for(j=1; j<=Num_node; j++)
                for(k=1; k<=Num_node; k++)//for(k=1; k<=Num_node; k++) if(k!=j)     
                if(k!=j) 
                {    //printf("�ڵ�%d<--->�ڵ�%d\n",j,k); 
                     if( Iscontact_outroom(j, k )==1 ) //�ڵ�j��k�μ���ͬһ¥�㡢���߳����ҡ�ȥͬһ���ȵ����򣬽��а���ɢ
                     {   
                         
                         //�����㷨��ʼ************************************************************************************************************************ 
                        /* if( ( received[1][j]==1) && ( received[1][k]==0) )
                         {    received[1][k]=1;     statistic(1, k, interestpacket[i], curgap+50*60 );//statistic(1, k, interestpacket[i], classtime[rand3+t-1]-classtime[rand3]+50*60 ); 
                              if( choose_seed_for_class[j][ next_roomid[k] ]==0 ) { nextseed[k]=1;  choose_seed_for_class[j][ next_roomid[k] ]==k; }
                         } */
                         if( received[1][j]==1 ) 
                         {    if( received[1][k]==1) //�ڵ�k�п��� 
                              {    if( choose_seed_for_class[j][ next_roomid[k] ]==0 ) { nextseed[k]=1;  choose_seed_for_class[j][ next_roomid[k] ]==k; }
                              } 
                              else//���� 
                              {    if( oneofinterest(interestpacket[i],k)==1) //�ڵ�k�Դ˰�����Ȥ 
                                   {    received[1][k]=1;     statistic(1, k, interestpacket[i], curgap+50*60 ); 
                                        if( choose_seed_for_class[j][ next_roomid[k] ]==0 ) { nextseed[k]=1;  choose_seed_for_class[j][ next_roomid[k] ]==k; }
                                   }
                                   else//����
                                   {    if( choose_seed_for_class[j][ next_roomid[k] ]==0 ) 
                                        {     received[1][k]=1;     statistic(1, k, interestpacket[i], curgap+50*60 );
                                              nextseed[k]=1;        choose_seed_for_class[j][ next_roomid[k] ]==k; 
                                        }
                                   } 
                              } 
                         }
                        //�����㷨����***********************************************************************************************************************
                          
                          
                          
                        if( k >= (j+1) )//����ִ��Epidemic��PS�����㷨 
                        { 
                          //Epidemic�㷨��ʼ*************************************************************************************************************** 
                          if( (received[2][j]==1) && (received[2][k]==0)  )  //Я������������ڵ� 
                          {   received[2][k]=1;  statistic(2,k,interestpacket[i],curgap+50*60);//statistic(2,k,interestpacket[i],classtime[rand3+t-1]-classtime[rand3]+50*60);  
                              //printf("case(2):Я����%d�����ڵ�-%d ",j,k); //pause();
                          }
                          else if( (received[2][k]==1) && (received[2][j]==0)  ) //Я������������ڵ�
                          {   received[2][j]=1;  statistic(2,j,interestpacket[i],curgap+50*60);  //statistic(2,j,interestpacket[i],classtime[rand3+t-1]-classtime[rand3]+50*60);  
                              //printf("case(2):Я����%d�����ڵ�-%d ",k,j); //pause();
                          }
                      
                          //Publish/Subscribe(PS)�㷨��ʼ*************************************************************************************************** 
                          if(ismet[j][k]==0){ popular[j][1]++; if(isbroker[k]==1) metbrokers[j]++; }       ismet[j][k] = 1; //1:���½ڵ�ĵ�ǰʱ�䴰������ֵ����ʶ������ 
                          if(ismet[k][j]==0){ popular[k][1]++; if(isbroker[j]==1) metbrokers[k]++; }       ismet[k][j] = 1;
                          
                          otherspopular[j][k] = popular[k][0];   otherspopular[k][j] = popular[j][0];//2:���½ڵ㱣��������ڵ���һ��ʱ�䴰������ֵ   
                     
                          if( (popular[j][1]<lower_bound) && (isbroker[k]==0) )   isbroker[k] = 1;//3_1������ͨ�ڵ�ѡΪbroker 
                          if( (popular[k][1]<lower_bound) && (isbroker[j]==0) )   isbroker[j] = 1;//3_1������ͨ�ڵ�ѡΪbroker
                          
                          if( (metbrokers[j]>=upper_bound) && (isbroker[k]==1) )  isbroker[k] = 0;//3_2����brokerѡΪ��ͨ�ڵ�
                          if( (metbrokers[k]>=upper_bound) && (isbroker[j]==1) )  isbroker[j] = 0;//3_2����brokerѡΪ��ͨ�ڵ�
                          
                          //4��������ɢ
                          if((j==source)&&(isbroker[k]==1)&&(received[3][k]==0))//Դ����broker
                          {   received[3][k]=1;  statistic(3,k,interestpacket[i],curgap+50*60);//statistic(3,k,interestpacket[i],classtime[rand3+t-1]-classtime[rand3]+50*60);  
                              //printf("case4.1:Դ����broker-%d ",k); //pause();
                          } 
                          if((k==source)&&(isbroker[j]==1)&&(received[3][j]==0))//ͬ��
                          {   received[3][j]=1;  statistic(3,j,interestpacket[i],curgap+50*60);//statistic(3,j,interestpacket[i],classtime[rand3+t-1]-classtime[rand3]+50*60);  
                              //printf("case4.1:Դ����broker-%d ",j); //pause();
                          }
                          if((isbroker[j]==1)&&(received[3][j]==1)&&(isbroker[k]==1)&&(received[3][k]==0))//broker����broker
                          {   received[3][k]=1;  statistic(3,k,interestpacket[i],curgap+50*60);//statistic(3,k,interestpacket[i],classtime[rand3+t-1]-classtime[rand3]+50*60);  
                              //printf("case4.2:broker%d����broker-%d ",j,k); //pause();
                          }
                          if((isbroker[k]==1)&&(received[3][k]==1)&&(isbroker[j]==1)&&(received[3][j]==0))//ͬ��
                          {   received[3][j]=1;  statistic(3,j,interestpacket[i],curgap+50*60);//statistic(3,j,interestpacket[i],classtime[rand3+t-1]-classtime[rand3]+50*60);  
                              //printf("case4.2:broker%d����broker-%d ",k,j); //pause();
                          }
                          if((isbroker[j]==1)&&(received[3][j]==1)&&(isbroker[k]==0)&&(k!=source)
                          &&(received[3][k]==0)&&(oneofinterest(interestpacket[i],k)==1))//broker��������Ȥ�ڵ� 
                          {   received[3][k]=1;  statistic(3,k,interestpacket[i],curgap+50*60);//statistic(3,k,interestpacket[i],classtime[rand3+t-1]-classtime[rand3]+50*60);  
                              //printf("case4.3:broker%d������Ȥ�ڵ�-%d ",j,k); //pause();
                          }
                          if((isbroker[k]==1)&&(received[3][k]==1)&&(isbroker[j]==0)&&(j!=source)
                          &&(received[3][j]==0)&&(oneofinterest(interestpacket[i],j)==1))//broker��������Ȥ�ڵ� 
                          {   received[3][j]=1;  statistic(3,j,interestpacket[i],curgap+50*60);//statistic(3,j,interestpacket[i],classtime[rand3+t-1]-classtime[rand3]+50*60);  
                              //printf("case4.3:broker%d��������Ȥ�ڵ�-%d ",k,j); //pause();
                          }        
                          //PS�㷨����********************************************************************************************* 
                     
                          //***add for sgbr                           
                          if( (received[0][j]==1) && (sgbr_num_copies[j]>=1) && (received[0][k]==0) && (oneofinterest(interestpacket[i],j)==0)
                              && (oneofinterest(interestpacket[i],k)==1) )//Я����j(������Ȥ��1 ������)��������Ȥ�ڵ㣬ת�� 
                          {   //printf("(0)������=%d       ",sgbr_num_copies[j]); 
                              received[0][k]=1;  sgbr_num_copies[k]=sgbr_num_copies[j]; sgbr_num_copies[j]=0; statistic(0,k,interestpacket[i],curgap+50*60);  
                              //printf("case(0)1.1:Я����%d������Ȥ�ڵ�-%d ",j,k); pause();
                          }
                          else if( (received[0][k]==1) && (sgbr_num_copies[k]>=1) && (received[0][j]==0) && (oneofinterest(interestpacket[i],k)==0)
                              && (oneofinterest(interestpacket[i],j)==1) )//������Գƣ�Я����k(������Ȥ��1 ������)��������Ȥ�ڵ㣬ת�� 
                          {   //printf("(0)������=%d     ",sgbr_num_copies[k]); 
                              received[0][j]=1;  sgbr_num_copies[j]=sgbr_num_copies[k]; sgbr_num_copies[k]=0;  statistic(0,j,interestpacket[i],curgap+50*60);  
                              //printf("case(0)1.2:Я����%d������Ȥ�ڵ�-%d ",k,j); pause();
                          }
     
                          else if( (received[0][j]==1) && (sgbr_num_copies[j]>1) && (received[0][k]==0) && (Isnextrelay_sgbr(j,k,interestpacket[i])==1)  )
                          //Я����j(����1������)����û���յ������ҷ��������ڵ㣬ת��    
                          {   //printf("(1)������=%d      ",sgbr_num_copies[j]); 
                              received[0][k]=1;   l = sgbr_num_copies[j]; sgbr_num_copies[j]=l/2; sgbr_num_copies[k]=l-sgbr_num_copies[j]; 
                              //printf("(1)%d %d %d  ",l,copies_sw[j],copies_sw[k]); pause(); 
                              statistic(0,k,interestpacket[i],curgap+50*60); 
                              //printf("case(0)2.1:Я����%d����û�п����ڵ�-%d ",j,k); pause();
                          }
                          else if( (received[0][k]==1) && (sgbr_num_copies[k]>1) && (received[0][j]==0) && (Isnextrelay_sgbr(k,j,interestpacket[i])==1)  )
                          //������Գƣ�Я����k(����1������)����û���յ������ҷ��������ڵ㣬ת��    
                          {   //printf("(1)������=%d     ",sgbr_num_copies[k]); 
                              received[0][j]=1;   l = sgbr_num_copies[k]; sgbr_num_copies[k]=l/2; sgbr_num_copies[j]=l-sgbr_num_copies[k]; 
                              //printf("(1)%d %d %d  ",l,copies_sw[j],copies_sw[k]); pause(); 
                              statistic(0,j,interestpacket[i],curgap+50*60);
                              //printf("case(0)2.2:Я����%d����û�п����ڵ�-%d ",j,k); pause();
                          }   
                          //����sgbr�ڵ������� 
                          //if( t % step == 0 )  
                          {   if( (t-recent_contact_time[j][k]) >1 )                 //(1)������һ����ϵ���ʱ�� 
                              {    last_contact_time[j][k] = last_contact_time[k][j] = recent_contact_time[j][k];    }
                              recent_contact_time[j][k]=recent_contact_time[k][j]= t; //(2)�������һ����ϵʱ��Ϊ��ǰ
                              if(last_contact_time[j][k]>0)                                      //(3)�ӵڶ�����ϵ��ʼ������ͨϵ�� 
                              {    double tempt_f = pow(sgbr_aging_constant, (t-last_contact_time[j][k])*times);
                                   tempt_f = connected_d[j][k]*tempt_f + (1.0-connected_d[j][k]*tempt_f)*sgbr_updating_factor; 
                                   connected_d[j][k] = connected_d[k][j] = tempt_f;
                              }
                          }    
                          //***add end for sgbr                          
                     
                     
                        }//if( k >= (j+1) )//����ִ��Epidemic��PS�����㷨  
                                         
                     }//if( Iscontact_outroom(j, k )==1 ) 
                            
                }//for(k=(j+1); k<=Num_node;                
           }//end if( (curgap+50*60) < TTL ) //���������ڻ�û�н���  
          //(2/2)�μ�ͨ�Ž���*************************************************************************************************                 
          
          
          currand1 = nextrand1;    currand2 = nextrand2;   currand3 = nextrand3;
          curgap += delta;
          
          t++;    //t��Ϊ��һ�ڿ�
          l=0;
          for(j=1; j<=Num_node; j++) 
            if(sgbr_num_copies[j]>0){  l +=sgbr_num_copies[j]; /*printf( "%d--",sgbr_num_copies[j]);*/ } 
         // printf("�ܿ�����=%d ",l); //getchar(); 
           
      }//end while( ((rand3+t-1)<=11) && ((classtime[rand3+t-1]-classtime[rand3])<TTL) )
      
      
      printf("\nʱ��=%ld ���η�������\n",t); 
   }//end for(i=1;
                                
                                
   //(7)��ʾ���н��
   //���ӳ�Ԥ����
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
   printf("�����㷨:  ������=%f, ƽ���ӳ�=%f ������Ŀ=%f\n", num_delivered[1]*1.0/total_num_interest, delay[1]*1.0, num_copy[1]*1.0/num_delivered[1]);
   printf("3 ratios(���Epidemic)��%f, %f, %f\n", ratio1, ratio2, ratio3);
   
   ratio1=(num_delivered[1]-num_delivered[3])*1.0/total_num_interest;  
   ratio2=(delay[1]-delay[3])/(delay[3]*1.0);
   ratio3=(num_copy[1]*1.0/num_delivered[1]-num_copy[3]*1.0/num_delivered[3])/(num_copy[3]*1.0/num_delivered[3]);
   printf("3 ratios(���PS)      ��%f, %f, %f\n\n", ratio1, ratio2, ratio3);
   
   //***add for sgbr
   ratio1=(num_delivered[1]-num_delivered[0])*1.0/total_num_interest;  
   ratio2=(delay[1]-delay[0])/(delay[0]*1.0);
   ratio3=(num_copy[1]*1.0/num_delivered[1]-num_copy[0]*1.0/num_delivered[0])/(num_copy[0]*1.0/num_delivered[0]);
   printf("3 ratios(���SGBR)    ��%f, %f, %f\n\n", ratio1, ratio2, ratio3);   
   //***add end

   printf("Epidemic:  ������=%f, ƽ���ӳ�=%f ������Ŀ=%f\n\n", num_delivered[2]*1.0/total_num_interest, delay[2]*1.0, num_copy[2]*1.0/num_delivered[2]);

   printf("PS      :  ������=%f, ƽ���ӳ�=%f ������Ŀ=%f\n\n", num_delivered[3]*1.0/total_num_interest, delay[3]*1.0, num_copy[3]*1.0/num_delivered[3]);
 
   //***add for sgbr
   printf("SGBR    :  ������=%f, ƽ���ӳ�=%f ������Ŀ=%f\n\n", num_delivered[0]*1.0/total_num_interest, delay[0]*1.0, num_copy[0]*1.0/num_delivered[0]);
   //***add end
 
   pause();
   printf("\nPress any key to end.");
   return 1;
}//���������� 

                              
                       
