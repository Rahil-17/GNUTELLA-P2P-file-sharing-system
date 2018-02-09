#include <stdio.h>  // for perror
#include <iostream>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <bits/stdc++.h>
#include <chrono>       //system clock
#include <ctime>        //system clock
#include <fstream>      //ifstream and ofstream
#include <regex>
#include <sys/wait.h>   //wait
#include <unistd.h> //read and write


using namespace std;
#define MAX 1024
/*
struct sockaddr {
    unsigned short    sa_family;    // address family, AF_xxx
    char              sa_data[14];  // 14 bytes of protocol address
};
*/

void rpc(string command);
void insert_ds(char path1[]);
void search_ds();
void delete_ds(char path1[]);
void insert_client_log(string c_alias,string c_ip,string c_port,string c_apath);
void insert_logfile(string c);


/************repo file and data structure  ****/

string user,s1,s2,s3;
string file_name,alias,path,cmd,output_file,comm,c_info;
int i1,i2,len,mirror_no;
unordered_map<string,unordered_map <string, string> > repo_data;    

/************** client_log  ************/

unordered_map<string, string> client_log;
 char client_alias[MAX]= {0};
 char client_ip[MAX] = {0};
 char client_port[MAX] = {0};
 char client_dport[MAX] = {0};
 char ctos_buff[MAX];
 char stoc_buff[MAX];
 
 string stoc(stoc_buff);

/**************socket connection ************/

 int server_fd, new_socket, valread;
 struct sockaddr_in address;
 int opt = 1;
 int addrlen = sizeof(address);
 char buffer[MAX] = {0};

 /**********************server ***************/

void connection(const char a[])     //socket connection
{   
    int PORT=atoi(a);
    server_fd = socket(AF_INET, SOCK_STREAM, 0);        //AF_INEt for ipv4,sock_stream for TCP,0 fpr internet protocol
    
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT,
                                                  &opt, sizeof(opt)))   //helps in reuse of address and port. optional
    {
        printf("setsockopt");
        exit(0);
    }
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;   //INADDR_ANY for localhos
    address.sin_port = htons( PORT );   //The htons() function converts the unsigned short integer hostshort from host byte order to network byte order.
      
    // Forcefully attaching socket to the port 8080
    if (bind(server_fd, (struct sockaddr *)&address, 
                                 sizeof(address))<0)
    {
        printf("bind failed");
        exit(0);
    }
    listen(server_fd, 3);
    
    if ((new_socket = accept(server_fd, (struct sockaddr *)&address, 
                       (socklen_t*)&addrlen))<0)
    {
        printf("accept");
        exit(0);
    }
}

void recieve_initial_data()     //initialization
{
    cout<<"new client connected\n";
    read( new_socket , client_alias, MAX);
    read( new_socket , client_ip, MAX);
    read( new_socket , client_port, MAX);
    read( new_socket , client_dport, MAX);

    string c_alias(client_alias);
    string c_ip(client_ip);
    string c_port(client_port);
    string c_dport(client_dport);
    insert_client_log(c_alias,c_ip,c_port,c_dport);
}


int filerecv()
{
    FILE *fp;
    int bytesReceived=0;
    fp = fopen("sample_file.txt", "ab"); 
    char recvBuff[MAX]={0};
    if(NULL == fp)
    {
        printf("Error opening file");
        return 1;
    }

    if((bytesReceived = read(new_socket, recvBuff, MAX)) > 0)
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

int filesend()
 {
        FILE *fp = fopen("new.txt","rb");
        if(fp==NULL)
        {
            printf("File open error");
            return 1;   
        }   

        while(1)
        {
            unsigned char buff[256]={0};
            int nread = fread(buff,1,256,fp);
            
            if(nread > 0)
            {
                write(new_socket, buff, nread);
            }

            if (nread < 256)
            {
                if (feof(fp))
                    printf("File Sent\n");
                if (ferror(fp))
                    printf("Error reading\n");
                break;
            }


        }
        return 0;
 }   

void receive_from_client()      //receive data from client
{    
   // bzero(ctos_buff,MAX);
    int i=read( new_socket , ctos_buff, MAX);
    ctos_buff[i]='\0';
    //ctos(ctos_buff);
}

void send_to_client()           //send data to client
{
    int i=stoc.copy(stoc_buff,stoc.length(),0);
    stoc_buff[i]='\0';
    send(new_socket,stoc_buff,strlen(stoc_buff),0);
} 


/*********************repo file data structure************/

void getcommand(string input,char path1[])               //parsing the commands
{   
    string temp;
    int x=input.length();
    if(input.compare(0,6,"search")==0)
    {
        cmd=input.substr(0,6);
        int i1;
        i1=input.find(":");
        file_name=input.substr(8,i1-9);
        alias=input.substr(i1+1,x-i1);
        search_ds();
    }   
    
    else if(input.compare(0,5,"share")==0)
    {
        cmd=input.substr(0,5);
        temp=input.substr(7,x-7);
        int i1,i2;
        i2=temp.rfind("/");
        path=temp.substr(0,i2);
        i1=temp.find(":");
        file_name=temp.substr(i2+1,i1-i2-2);
        alias=temp.substr(i1+1,x-i1);
        insert_ds(path1);        
    }       

    else if(input.compare(0,3,"del")==0)
    {
        cmd=input.substr(0,3);
        temp=input.substr(5,x-5);
        int i1,i2;
        i2=temp.rfind("/");
        path=temp.substr(0,i2);
        i1=temp.find(":");
        file_name=temp.substr(i2+1,i1-i2-2);    
        alias=temp.substr(i1+1,x-i1);
        delete_ds(path1);
    }       
    else if(input.compare(0,3,"get")==0)
    {
        cmd=input.substr(0,3);
        if(input[4]=='[')
        {
            cout<<"serial no. of mirror: "<<input[5]<<endl;
            output_file=input.substr(9,x-10);
            mirror_no=(int)input[5];
        }
        else if(input[4]=='"')
        {
            int i1,i2;
            temp=input.substr(5,x-5);
            i1=temp.find('"');
            i2=temp.rfind('"');
            output_file=temp.substr(i2+2,x-i2);
            alias=temp.substr(0,i1);
            path=temp.substr(i1+3,i2-i1-3);
            i1=alias.find('\\');
            alias.erase(i1,1);  
            i2=path.find('\\');
            path.erase(i2,1);
            
            i1=path.rfind('/');
            file_name=path.substr(i1+1,path.length()-i1-1);
            path=path.substr(0,i1-1);
            
        }   
    }
    else if(input.compare(0,4,"exec")==0)
    {

        cmd=input.substr(0,4);
        temp=input.substr(6,x-7);
        int i1,i2;
        i1=temp.find('\"');
        i2=temp.find('\"');
        alias=temp.substr(0,i1);
        comm=temp.substr(i2+3,temp.length()-i2-i1-1);
        rpc(comm);
    }   


    else{cout<<"invalid input\n";}
    
    time_t now = time(0);
    tm* dt = localtime(&now);
    string hr=to_string(dt->tm_hour);
    string min=to_string(dt->tm_min);
    string sec=to_string(dt->tm_sec);
    string c=hr + ":" + min + ":" + sec + " "+ cmd + " from " + alias;
    insert_logfile(c);
}

void printmap()     // display repo_file data structure
{
    unordered_map<string, unordered_map <string, string> >::iterator it1;
    unordered_map<string, string>::iterator it2;
    cout<<"\nALIAS\t\tFILENAME\t\tRELATIVE PATH"<<endl;
    for(it1=repo_data.begin();it1!=repo_data.end();it1++)
    {
        
        for(it2=it1->second.begin();it2!=it1->second.end();it2++)
        {
            cout<<it2->first<<" \t\t: ";
            cout<<it1->first<<" \t\t: ";
            cout<<it2->second<<" \t\t "<<endl;
        }
        cout<<"\n";
    }
}
void loadfile(char path1[])         //loads repo_file with data stored in repo_file data structure
{
    ofstream myfile;
    unordered_map<string, unordered_map <string, string> >::iterator it1;
    unordered_map<string, string>::iterator it2;
    myfile.open (path1,std::ofstream::out);

    for(it1=repo_data.begin();it1!=repo_data.end();it1++)
    {
        
        for(it2=it1->second.begin();it2!=it1->second.end();it2++)
        {
            myfile << it2->first + ":" + it1->first + ":" + it2->second;
            myfile <<"\n";
        }   
    }
    myfile.close();
}   
void loadmap(char path1[])          //populate repo_file data structure
{
    ifstream file(path1);
    while (getline(file, user))
    {
        unordered_map<string, string> data;
        unordered_map<string, string> data1;
        len=user.length();
        i1=user.find(":");
        s1=user.substr(0,i1);
        i2=user.rfind(":");
        s2=user.substr(i1+1,i2-i1-1);
        s3=user.substr(i2+1,len-i2-1);
        
        unordered_map<string,unordered_map <string, string> >::iterator got=repo_data.find(s2);
        if(repo_data.end()== got)
        {   
            data.insert(make_pair(s1,s3));
            repo_data.insert(make_pair(s2,data));
        }
        else
        {
            
            unordered_map<string, string>::iterator it22;   
            for(it22=got->second.begin();it22!=got->second.end();it22++)
            {
                data1.insert(make_pair(it22->first,it22->second));
                cout<<it22->first;
                cout<<it22->second<<endl;
            }
            data1.insert(make_pair(s1,s3));
            repo_data.erase(s2);
            
        }
        repo_data.insert(make_pair(s2,data1));  
        
    }
}


void search_ds()        //search command
{
    int no_of_hit=0;
    int flag=0;
    stoc="";
    regex reg ("\\b("+file_name+")([^]*)",regex::ECMAScript | regex::icase );
    unordered_map<string,unordered_map <string,string> >::iterator it;
    for(it=repo_data.begin();it!=repo_data.end();it++)
    {
        string s=it->first;
        if(regex_match ( s , reg ) )
        {
            cout<<file_name<<"  search found\n";
            for(auto it1=it->second.begin();it1!=it->second.end();it1++)
            
            {   
                no_of_hit++;
                string str="ip:po";
                cout<<"["<<no_of_hit<<"] ";
                string alias=it1->first;
                unordered_map<string,string>::iterator cit;
                cit=client_log.find(alias);
                if(cit==client_log.end())
                {
                    cout<<it->first <<":"<<it1->second<<"/"<<it->first<<":"<<it1->first<<":"<<str<<endl;
                    stoc+="[" + to_string(no_of_hit) + "] " + it->first + ":" + it1->second + "/" + it->first + ":" + it1->first + ":" +str + "\n";
                }
                else
                {   
                string ctemp=cit->second;
                int r=ctemp.rfind(":");
                str=ctemp.substr(0,r-1);
                cout<<it->first <<":"<<it1->second<<"/"<<it->first<<":"<<it1->first<<":"<<ctemp<<endl;
                stoc+="[" + to_string(no_of_hit) + "] " + it->first + ":" + it1->second + "/" + it->first + ":" + it1->first + ":" + str + "\n";
                }
            }
        flag=1; 
        }   
    }   
    if(flag==0)
       { 
        stoc="File not found";
        send_to_client();
        cout<<"File not found\n";   
       }
     else
     {
        send_to_client();    
     }  
    cout<<"\n";
}

void insert_ds(char path1[])        //share command
{
    unordered_map<string, string> data;
    unordered_map<string, string> data1;
    unordered_map<string,unordered_map <string, string> >::iterator got=repo_data.find(file_name);
        if(repo_data.end()== got)
        {   
            data.insert(make_pair(alias,path));
            repo_data.insert(make_pair(file_name,data));
        }
        else
        {
            
            unordered_map<string, string>::iterator it22;   
            for(it22=got->second.begin();it22!=got->second.end();it22++)
            {
                data1.insert(make_pair(it22->first,it22->second));
                cout<<it22->first;
                cout<<it22->second<<endl;
            }
            data1.insert(make_pair(alias,path));
            repo_data.erase(file_name);
            
        }
        repo_data.insert(make_pair(file_name,data1));
        stoc="SUCCESS";
        send_to_client();
        cout<<"SUCCESS";
        loadfile(path1); 
}

void delete_ds(char path1[])        //del command
{
    unordered_map<string,unordered_map <string, string> >::iterator got=repo_data.find(file_name);
        if(repo_data.end()== got)
        {   
            cout<<"unsuccessful";
            stoc="UNSUCCESSFUL";
            send_to_client();
        }
        else
        {   
            
            int size=got->second.size();
            if(size==1)
            {
                unordered_map<string, string>::iterator it22=got->second.find(alias);
                if(got->second.end()==it22)
                {   
                    cout<<"unsuccessful";
                    stoc="UNSUCCESSFUL";
                    send_to_client();
                }
                else
                {   
                    repo_data.erase(file_name);
                    cout<<"successful";
                    stoc="SUCCESSFUL";
                    send_to_client();
                }   
            }   
            else
            {
                unordered_map<string, string>::iterator it22=got->second.find(alias);
                 if(got->second.end()==it22)
                 {
                    cout<<"successful";
                    stoc="SUCCESSFUL";
                    send_to_client();
                 }  
                 else
                 {
                    got->second.erase(alias);
                    cout<<"successful";
                    stoc="SUCCESSFUL";
                    send_to_client();
                 }  
            }   

            
        }
        loadfile(path1); 

}  



/************** client log file data structure  ******************/

void dis_client_log()       //display client info
{
    unordered_map<string, string>:: iterator it;
    for(it=client_log.begin();it!=client_log.end();it++)
    {

        cout<<it->first<<" "<<it->second << endl;
    }
    cout<<"\n";
}

void insert_client_log(string c_alias,string c_ip,string c_port,string c_dport)     //insert the data of recently connected client
{
    string input= c_ip + ":" + c_port + ":" + c_dport;
    client_log.insert(make_pair(c_alias,input)); 
    ofstream log;
    log.open (c_info, std::ofstream::out | std::ofstream::app );
    log << c_alias + input;
    log <<"\n";
    log.close();        
}

void load_client_log(string c_info)         //populate client info data structure
{
   ifstream file(c_info);
    while (getline(file, user))
    { 
        int i1=user.find(":");
        string alias=user.substr(0,i1);
        string other=user.substr(i1+1,user.length()-i1-1);
        client_log.insert(make_pair(alias,other));
    }    
}

void insert_logfile(string c)       //insert into log file
{
    ofstream log;
    log.open ("log.txt", std::ofstream::out | std::ofstream::app );
    log << c;
    log <<"\n";
    log.close(); 
}

void dis_logfile()      // show log of all operations
{
 ifstream file("log.txt");
 while (getline(file, user))
 {
    cout<<user<<endl;
 }    
}

/**************     RPC     ************************/
void rpc(string cmd)            // exec command
{  
  int pipe_exec[2]; 
  pid_t pid;    //use to fetch output of execl command into memset
  int flag=2;
  int x=cmd.length();
  
  char comm_output[4096 + 1];
  char command_buff[1024];
  char option_buff[1024];
  char bin_buff[1024];
  memset(comm_output, 0, 4096);     //to store output of execl
  
  string command;
  string option;
  string bin;
  int i=cmd.find('-');          //parsing the string to separate command and options
  if(i==-1)
  {
    command=cmd;
    flag=1;
  }  
  else
  {
     option=cmd.substr(i,x-i);
     command=cmd.substr(0,i-1);
     i=option.copy(option_buff,option.length(),0);
     option_buff[i]='\0'; 
     flag=0;
  }  
  bin="/bin/" + command;
  i=command.copy(command_buff,command.length(),0);
  command_buff[i]='\0';

  i=option.copy(option_buff,option.length(),0);
  option_buff[i]='\0';

  i=bin.copy(bin_buff,bin.length(),0);
  bin_buff[i]='\0';

  if (pipe(pipe_exec)==-1)
    cout<<"unsuccessful attempt to create pipe\n";

   if ((pid = fork()) == -1)
    cout<<"unsuccessful attempt to create pipe\n";

  if(pid == 0)
  {
    dup2 (pipe_exec[1], STDOUT_FILENO); //dup() system call creates a copy of a file descriptor. 
    close(pipe_exec[0]);
    close(pipe_exec[1]);
    
    if(flag==0)
        execl(bin_buff,command_buff,option_buff, (char *)0);    //execute shell commands
    else
        execl(bin_buff,command_buff, (char *)0);    
    cout<<"command not found\n";
    stoc="command not found";
    send_to_client();
    return;
  } 
  else 
  {
    close(pipe_exec[1]);
    int no_bytes = 0;
    string final_string;
    while(0 != (no_bytes = read(pipe_exec[0], comm_output, sizeof(comm_output)))) 
    {
        final_string = final_string + comm_output;      // append the data of memset into string
        memset(comm_output, 0, 4096);
    }
    cout<<final_string;
    stoc=final_string;
    send_to_client();
    wait(NULL);     //waiting for child process to complete
    return;
  }
}


int main(int argc, char const *argv[])
{
    if(argc!=6)
     {
        cout<<"insufficient arguements\n";
        exit(0);
     }   
    connection(argv[2]);
    

    //************ communication *****//

    recieve_initial_data();     //recieves client's info and store it in the data structure
    string repfname(argv[3]);
    string rootpath(argv[5]);
    string clientfname(argv[4]);
    if(*(rootpath.end()-1)!='/')
    {
        rootpath+="/";
    }
    string cpath1=rootpath+repfname; // absolute path of the repo file
    string cpath2=rootpath+clientfname; // absolute path of the client file
    char *path1=new char[cpath1.length()+1];
    //char *path2=new char[cpath2.length()+1];
    strcpy(path1,cpath1.c_str());
    //strcpy(path2,cpath2.c_str());
    loadmap(path1);                  // load data into data structure from repo_file
    string ctemp(argv[4]);
    c_info=cpath2;
    load_client_log(c_info);    // load data into data structure from list.txt    
    //dis_client_log();           // display connected clients' information.
    while(1)
    {     
     cout<<"waiting for command...\n";   
     receive_from_client();     // recieves command from client
     string input(ctos_buff);       

     if(input.compare("exit")==0)   // terminate server if "exit" is recieved
        break;
     else
        getcommand(input,path1);      // send the comand for parsing and then do corresponding task
    }
    //printmap();               // display contain of data structure for repo_file
    loadfile(path1);                 // all the changes made in repo_file data structure store it in the repo_file.txt
    /*
    filerecv();
    filesend();
    */
    //dis_logfile();            //to display logs
    
    cout<<"Good bye\n";
    return 0;
}
