#include <stdio.h>
#include <iostream>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>	//for inet_pton in c++

using namespace std;
#define PORT 8080
#define MAX 1024


 struct sockaddr_in address;
 int sock = 0;
 struct sockaddr_in serv_addr;
 char buffer[MAX] = {0};
 char ctos_buff[MAX];
 char stoc_buff[MAX];
 string ctos;


int connection(const char a[],const char b[])
{ 
    int s_port=atoi(b); 
    sock = socket(AF_INET, SOCK_STREAM, 0);  
    memset(&serv_addr, '0', sizeof(serv_addr)); //The memset function is also used to set structure variables....set 0 in ser_addr for this much length
  
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(s_port);
       
    inet_pton(AF_INET,a, &serv_addr.sin_addr); //converts the character string src into a network address structure in the af address family, then copies the network address structure to dst
    
  
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        cout<<"\nConnection not established \n";
        return -1;
    }
    cout<<"successfully connected to server\n";
} 

void send_initial_data(const char a[],const char b[],const char c[],const char d[])
{
    send(sock,a,strlen(a),0);
    sleep(1);
    send(sock,b,strlen(b),0);
    sleep(1);
    send(sock,c,strlen(c),0);
    sleep(1);
    send(sock,d,strlen(d),0);
    sleep(1);
}

int filerecv()
{
	FILE *fp;
	int bytesReceived=0;
    fp = fopen("sample_file.txt", "ab"); 
    char recvBuff[256]={0};
    if(NULL == fp)
    {
        printf("Error opening file");
        return 1;
    }

    while((bytesReceived = read(sock, recvBuff, 256)) > 0)
    {
        fwrite(recvBuff, 1,bytesReceived,fp);
        printf("%s \n", recvBuff);
    }

    if(bytesReceived < 0)
    {
        printf("\n Read Error \n");
    }
    else cout<<"File recieved\n";
    return 0;
} 
void send_to_server()
{
    int i=ctos.copy(ctos_buff,ctos.length(),0);
    ctos_buff[i]='\0';
    send(sock,ctos_buff,strlen(ctos_buff),0);

} 

void receive_from_server()
{    
    int i=read(sock,stoc_buff, MAX);
    stoc_buff[i]='\0';
}	
  
void getcommand(string input,const char a[])
{   
    string alias(a);
    int x=input.length();
    if(input.compare(0,6,"search")==0)
    {
        ctos=input + ":" + alias;
        send_to_server();
        int bytesReceived=0;
        char recvBuff[MAX]={0};
        bytesReceived = read(sock, recvBuff, MAX);
        printf("%s \n", recvBuff);        
    }  
    
    else if(input.compare(0,5,"share")==0)
    {
        ctos= input + ":" + alias;
        send_to_server();
        receive_from_server();
        printf("%s\n",stoc_buff);
    }       

    else if(input.compare(0,3,"del")==0)
    {
        ctos=input + ":" + alias;
        send_to_server();
        receive_from_server();
        printf("%s\n",stoc_buff);
    }       
    else if(input.compare(0,3,"get")==0)
    {
        if(input[4]=='[')
        {
            ctos=input;
            send_to_server();
        }
        else if(input[4]=='"')
        {
            ctos=input;
            send_to_server();       
        }   
    } 
    else if((input.compare(0,4,"exec"))==0)
    {
        ctos=input;
        send_to_server();
        receive_from_server();
        printf("%s\n",stoc_buff);
    }      
    else{cout<<"invalid input\n";}
    //cout<<"\nname of file:        "<<file_name<<"\n";
    //cout<<"path:              "<<path<<"\n";
    //cout<<"alias of client:       "<<alias<<"\n";
    //cout<<"output_file:           " <<output_file<<"\n\n";

}  
int main(int argc, char const *argv[])
{
    if(argc!=8)
    {
        cout<<"insufficent arguements"; exit(0);
    }    
    connection(argv[4],argv[5]);
    

    //************ communication *****//

    send_initial_data(argv[1],argv[2],argv[6],argv[5]);
  /* chat();
    filerecv();
  */
    while(1)
    {    
    string input;
    cout<<"write command:";
    getline(cin,input);
    if(input.compare("exit")==0)
    { 
        ctos=input;
        send_to_server(); 
        break;
    }
    else
        getcommand(input,argv[1]);
    }
    return 0;
}
