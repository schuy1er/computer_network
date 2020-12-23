#include<iostream>
#include<winsock2.h>
#include<stdio.h>
#include<fstream>
#include<winsock.h>
#include<time.h>
#include<vector>
#include<string.h>
#include<bits/stdc++.h>
#include<thread>
#include<mutex>

using namespace std;

int N = 1;
int rande = 0;
int conge_timer = 0;
int state = 1;
int ssthresh = 128;
int dup = 0;

double timeout = 2;

mutex m;

clock_t start,endd;

int num_in_window = 0;
int has_got = 0;
int re_tran = 0;
long long int c;
long long int nextseqnum = 0;
int base = 0;
int syn = 0;
int fin = 0;
int seq;
int last = 0;
long long int all = c;
long long int len = all / 252;
long long int pack_len = 0;

bool close = false;
bool var = false;
bool is_cal_time = false;
bool in_window_but_not_admit[256] ={false};

queue<int> window;

unsigned char content[100000000];

SOCKET client;
SOCKADDR_IN serverAddr, clientAddr;

HANDLE hMutex1;

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

bool send_package(unsigned char * message, int pack_len, int seq, int last, int syn, int fin)
{
    unsigned char packet[pack_len + 4];
    if(syn)
    {
        packet[0] = SYN;
    }
    else if(fin)
    {
        packet[0] = FIN;
    }
    else if(last)
    {
        packet[0] = LAST;
    }
    else packet[0] = SIMPLE_DATA_NOT_LAST;
    
//    cout<<(int)packet[0]<<endl;
    
    packet[1] = seq;
    packet[2] = pack_len;
    for(int i = 4; i < pack_len + 4; i++)
        {
            packet[i] = message[i-4];
        }
    packet[3] = generate_sum(packet, 4 + pack_len);
    
    sendto(client, (char*)packet, pack_len + 4, 0, (sockaddr *)&serverAddr, sizeof(serverAddr));
//    cout<<"i have sent!"<<" sign is: "<<(int)packet[0]<<endl; 
    return true;
}

void  recvFromServer()
{
    int lens = sizeof(serverAddr);
	unsigned char recv[3];
	while (1) {
		memset(recv, '\0', sizeof(recv));
       if(recvfrom(client, (char*)recv, 3, 0, (sockaddr*)&serverAddr, &lens) != SOCKET_ERROR)//&& check_sum(recv, 2) == 0 && recv[0] == ACK && recv[2] == (base % 256)
       {
        m.lock();
        // cout<<"in recv"<<(int)recv[1]<<endl;
       	if(check_sum(recv, 2) && (recv[0] == ACK) && in_window_but_not_admit[recv[1]])
       	{
       	//  cout<<"line 129"<<endl;
        //    cout<<(int)recv[1]<<" window: "<<window.front()<<endl;
        // cout<<"ok ack: "<<(int)recv[1]<<" window "<<window.front()<<" "<<window.size()<<endl;
           while(!window.empty() && window.front() != recv[1])
           {
               base++;
               if(state == 1)
               {
                   N++;
               }
               if(state == 2)
               {
                   conge_timer++;
                   if(conge_timer == N)
                   {
                       N++;
                       conge_timer = 0;
                   }
               }
               if((N >= ssthresh) && state == 1){state = 2;conge_timer = 0;}
            //    cout<<"now base is "<<base<<endl;
               in_window_but_not_admit[window.front()] = 0;
               window.pop();
           }
           start = clock(); 
        //    cout<<"again clock"<<endl;
           if(!window.empty())
           {
                base++;
                if(state == 1)
               {
                   N++;
               }
               if(state == 2)
               {
                   conge_timer++;
                   if(conge_timer == N)
                   {
                       N++;
                       conge_timer = 0;
                   }
               }
               if((N >= ssthresh) && state == 1){state = 2;conge_timer = 0;}
           re_tran = 0; 
           in_window_but_not_admit[recv[1]] = 0;
           window.pop();
           }
		//    cout<<"base: "<<base<<" len: "<<len<<endl;
           if(base == len)
           {
           	close = true;
           	cout<<"close: "<<close<<endl;
           	cout<<"im leaving recv thread!"<<endl;
           	break;
		   }
           has_got += 1;
        //    if(base == nextseqnum)
        //    {
        //        is_cal_time = false;
        //    }
        //    else
        //    {
        //        start = clock();
        //        re_tran = 0;
        //        is_cal_time = true;
        //    }  
       }
       else if(check_sum(recv, 2) && (recv[0] == ACK))
       {
           dup++;
        //    cout<<"dup ack: "<<(int)recv[1]<<" windowfront "<<window.front()<<" "<<in_window_but_not_admit[recv[1]]<<'\n';
           if(dup == 3)
           {
               var = 1;
               dup = 0;
           }
       }
    //    cout<<"line 157"<<endl;
       
    //    _sleep(1);
	}
    if(re_tran > 7)
           {
           	close = true;
           	// cout<<"close: "<<close<<endl;
           	cout<<"im leaving recv thread!"<<endl;
           	break;
		   }
    m.unlock();
}
m.unlock();
}

void sendtoServer()
{
    while(1)
    {
    	m.lock();
        // cout<<"line 168"<<endl;
        if(var == 1)
        {
            while(!window.empty())window.pop();
            nextseqnum = base;
            var = 0;
            dup = 0;
            state = 1;
            ssthresh = N / 2;
            N = 1;
        }
    	if(nextseqnum < base + N)
        {      
        if(nextseqnum == 0)
        {
            syn = 1;
            fin = 0;
            last = 0;
            pack_len = 0;
            start = clock();
        }
        else if(nextseqnum == len - 2)
        {
        	syn = 0;
        	fin = 0;
            last = 1;
            pack_len = all - (nextseqnum - 1) * 252;
        }
        else if(nextseqnum == len - 1)
        {
            last = 0;
            fin = 1;
            syn = 0;
            pack_len = 0;
        }
        else
        {
            syn = 0;
            fin = 0;
            last = 0;
            pack_len = 252;
        }
        // cout<<"line 200"<<endl;
        
    //    if(rand()% 100 != 0)
        {
        if(nextseqnum >= 1)send_package(content + (nextseqnum - 1) * 252, pack_len, (nextseqnum % 256), last, syn, fin);
        else send_package(content, pack_len, (nextseqnum%256), last, syn, fin);
        }
        // rande ++;
            window.push(nextseqnum % 256);
            // cout<<"line 204"<<endl;
             cout<<"send "<<nextseqnum%256<<"base is"<<base<<"window size is "<<N<<"state is: "<<state<<"ssthresh is : "<<ssthresh<<'\n';
            in_window_but_not_admit[nextseqnum % 256] = 1;
            // cout<<"line 205"<<endl;
        //    cout<<"nextseqnum "<<nextseqnum<<endl;
            // if(base == nextseqnum){
            //     start = clock();
            //     re_tran = 0;
            //     is_cal_time = true;
            //     }
            nextseqnum++;
        }
        endd = clock();
        // cout<<"line 214"<<endl;
        if( (double)(endd-start)/CLOCKS_PER_SEC > timeout)
        {
            // cout<<"line 217"<<endl;
            nextseqnum = base;
            if(state == 1)
            {
                ssthresh = N / 2;
                N = 1;
            }
            if(state == 2)
            {
            ssthresh = N / 2;
            N = 1;
            state = 1;
            }
            // cout<<"line 218"<<endl;
            start = clock();
            // cout<<"line 219"<<endl;
            re_tran++;
            while(!window.empty())window.pop();
            // cout<<"line 220"<<endl;
           cout<<"time up time: "<<re_tran<<endl;
            if(re_tran > 7)break;
            if(close == true)
            {
            	cout<<"im leaving send thread!"<<endl;
				break;
			}
        }
        // cout<<"line 227"<<endl;
        m.unlock();
        // _sleep(1);
	}
	cout<<"im leaving send thread!"<<endl;
    m.unlock();
}

bool send_message(unsigned char * message)
{ 
    syn = 0;
    fin = 0;
    last = 0;
    all = c;
    len = all / 252;
    pack_len = 0;
    if(all % 252 != 0)len++;
    unsigned char recv[3];
    int lens = sizeof(serverAddr);
    len = len + 2;
    cout<<"this message all long is "<<len<<endl;
    thread t[2];
    t[0] = thread(recvFromServer);
     t[1] = thread(sendtoServer);
    t[0].join();
    t[1].join();
//    	cout<<close<<endl;
        if(close == true)
        {
        	cout<<"im leaving send func!"<<endl;
        	return true; 
		}
        if(re_tran > 7)
        {
            cout<<"retran so much"<<endl;
            return false;
        }
    cout<<"ok i will go"<<endl;
    return true;
}

int main()
{
    WSADATA wsadata;
    int version = WSAStartup(MAKEWORD(2, 2), &wsadata);
    if(version)
    {
        cout<<"version init error!"<<endl;
        return 0;
    }
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(5577);
    serverAddr.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
    client = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
//   int RcvNetTimeout = 1;
//   setsockopt(client, SOL_SOCKET, SO_RCVTIMEO, (char *)&RcvNetTimeout, sizeof(int));
    string file;
    while(true)
    {
        cout<<"enter the file name: "<<endl;
        cin>>file;
        // fstream in(file);
        ifstream in(file.c_str(), ifstream::binary);
        if(!in)
        {
            cout<<"file is invalid, try another!"<<endl;
            continue;
        }
        int i = 0;
        unsigned char t = in.get();
        while(in)
        {
            content[i] = t;
            i++;
            t = in.get();
        }
        c = i;
        in.close();
        break;
    }
    cout<<"trying to set up!"<<endl;
    if(send_message(content) == true)cout<<"send ok!"<<endl;
    cout<<c<<endl;
	system("pause");
    return 0;
}
