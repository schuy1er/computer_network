#include<iostream>
#include<winsock2.h>
#include<stdio.h>
#include<fstream>
#include<winsock.h>
#include<time.h>
#include<string.h>
#include<bits/stdc++.h>

using namespace std;

const int N = 10;

clock_t start,endd;

long long int len_recv = 0;

bool close = false;

unsigned char sendd[4];
unsigned char recvv[256];

int rande = 0;
int expectedseqnum = 0;
   
unsigned char message[100000000];
unsigned char real_content[100000000];

SOCKET server;
SOCKADDR_IN serverAddr, clientAddr;
int lenc = sizeof(clientAddr);
const char ACK = 1;
const char NAK = 2;
const char SYN = 4;
const char FIN = 8;
const char RETRAN = 16;
const char SIMPLE_DATA_NOT_LAST = 3;
const char LAST = 32;
const char SYN_ACK = 5;

unsigned char generate_sum(unsigned char *s, int len)
{
    unsigned char tmp = s[0];
    s[3] = 0;
    for(int i = 1; i < len; i++)
    {
        unsigned int k = tmp + (unsigned char)s[i];
        k = k / (1<<8) + k % (1<<8);
        tmp = k;
    }
    return ~tmp;
}

bool check_sum(unsigned char *s, int len)
{
  unsigned char tmp = s[0];
  for(int i = 1; i <= len; i++)
  {
      unsigned int k = tmp + (unsigned char)s[i];
      k = k / (1<<8) + k % (1<<8);
      tmp = k;
  }
   int k = (unsigned char)~tmp;
  return (k==0);
}
DWORD WINAPI  sendtoClient(LPVOID lpParameter)
{
	while(1)
	{
		if(recvfrom(server, (char*)recvv, 256, 0, (sockaddr*)&clientAddr, &lenc) != SOCKET_ERROR)
       {
     	cout<<"i got a packet"<<(int)recvv[1]<<endl;
           if(check_sum(recvv, (int)recvv[2]+3) && (int)recvv[1] == (expectedseqnum % 256))
               {
                   expectedseqnum++;
                   sendd[0] = ACK;
                   sendd[1] = recvv[1];
                   sendd[2] = generate_sum(sendd, 2);
                //  cout<<" and i accept "<<(int)recvv[1]<<endl;
            //   if(rand()%2==0)
         {sendto(server, (char*)sendd, 3, 0, (sockaddr *) &clientAddr, sizeof(clientAddr));}
              
               //   else cout<<"i miss"<<endl;
                //  cout<<"my expectedseqnum is"<<expectedseqnum<<endl;
				   if (LAST == recvv[0] || SIMPLE_DATA_NOT_LAST == recvv[0] )
				    {
                for (int i = 4; i < recvv[2] + 4; i++)
                {
				message[len_recv++] = recvv[i];
				// if(message[len_recv-1] != real_content[len_recv-1])
				// {
				// 	cout<<"this is different! "<<len_recv - 1;
				// 	_sleep(5000);
				// }
            }
				  } 
				  if(recvv[0] == FIN)
				  {
				  	close = true;
				    cout<<"close: true"<<endl;
				  	return 0;
				  }
               }
            else sendto(server, (char*)sendd, 3, 0, (sockaddr *) &clientAddr, sizeof(clientAddr));
       }
	}
}
//  bool recv_message()
//  {
//      unsigned char recv[256];
//      int lenc = sizeof(clientAddr);
//      int expectedseqnum = 0;
//      while(true)
//      {
//          while(true)
//          {
//              while(recvfrom(server, (char*)recv, 256, 0, (sockaddr*)&clientAddr, &lenc) == SOCKET_ERROR);
//              unsigned char send[4];
//              cout<<"i got a packet which is "<<(int)recv[0]<<" "<<(int)recv[1]<<" "<<(int)recv[2]<<" "<<(int)recv[3]<<endl;
//                  if(check_sum(recv, (int)recv[2]+3) && (int)recv[2] == expectedseqnum)
//                  {
//                      expectedseqnum++;
//                      expectedseqnum %= 256;
//                      send[0] = ACK;
//                      send[1] = recv[1];
//                      send[2] = 0;
//                      send[3] = generate_sum(send, 3);
//                      cout<<" and i accept it"<<endl;
//                      sendto(server, (char*)send, 4, 0, (sockaddr *) &clientAddr, sizeof(clientAddr));
//                      cout<<"i send ack"<<endl;
//                      break;   
//                  }
//                  else
//                  {
//  				  sendto(server, (char*)send, 4, 0, (sockaddr *) &clientAddr, sizeof(clientAddr));
//  				  cout<<"i send old ack"<<(int)send[0]<<endl;
//  				}
//          }
//          for (int i = 4; i < recv[2] + 4; i++)
//              message[len_recv++] = recv[i];
//          if(FIN == recv[0])
//          {
//              message[len_recv] = 0;
//              return true;
//          }
//      }
//      return false;
//  }

bool recv_message()
{
   sendd[0] = NAK;
   sendd[1] = 255;
   sendd[2] = generate_sum(sendd, 2);  
   HANDLE t = CreateThread(NULL, 0, &sendtoClient, &server, 0, NULL);
   CloseHandle(t);
   while(1)  
   {
//    cout<<close<<endl;
   	if(close == true)return true;
   }
   return false;
}

int main()
{
   WSADATA wsadata;
   int version = WSAStartup(MAKEWORD(2, 2), &wsadata);
   if(version)
   {
       cout<<"init error!"<<endl;
       return 0;
   }
    server = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (server == -1) {
		cout << "Socket failed." << endl;
		return -1;
	}
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(6556);
    serverAddr.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
    
//    int RcvNetTimeout = 1;
//    setsockopt(server, SOL_SOCKET, SO_RCVTIMEO, (char *)&RcvNetTimeout, sizeof(int));
    
    if (server == INVALID_SOCKET) {
        cout<<"Invalid Socket!"<<endl;
        closesocket(server);
        return 0;
    }
    int binds = bind(server, (sockaddr *)(&serverAddr), sizeof(serverAddr));
    if (binds == SOCKET_ERROR) {
        printf("bind fail");	
        closesocket(server);
        WSACleanup();
        return 0;
    }
    int i = 0;
    string file = "helloworld.txt";
    ifstream in(file.c_str(), ifstream::binary);
     unsigned char t = in.get();
        while(in)
        {
            real_content[i] = t;
            i++;
            t = in.get();
        }
        in.close();
    cout<<"waiting for connection!"<<endl;
    string fi = "5.jpg";
    if(recv_message() == true)cout<<"recv me ok!"<<endl;
	cout<<len_recv<<endl;
    ofstream fout1(fi.c_str(), ofstream::binary);
    for (int i = 0; i < len_recv; i++)
        fout1 << message[i];
    fout1.close();
    
    system("pause");
    return 0;
}
