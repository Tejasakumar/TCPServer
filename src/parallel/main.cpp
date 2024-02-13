#include <stdio.h>

#include <stdlib.h>

#include <pthread.h>

#include <string.h>

#include <string>

#include <netinet/in.h>

#include <unistd.h>

#include <sys/socket.h>

#include <unordered_map>

#include <sstream>

#include <iostream>

#include <vector>

#include <sys/socket.h>

#include <arpa/inet.h>

#include <queue>

using namespace std;

queue <int> mcq;

int active_threads = 0;

unordered_map<string, string> dict;

int lock = 1;

void aquire(){

	lock--;

}

void release(){

	lock++;

}

/* data */

void write_into(int *pointer, string &key, string &value)

{



	value.erase(value.begin(), value.begin() + 1);

	dict[key] = value;

	*pointer++;

	*pointer++;

}

void read_from(string &key, char *output)

{

	if (dict.find(key) == dict.end())

	{

		strcat(output, "NULL\n");

	}

	else

	{

		strcat(output, dict.find(key)->second.c_str());

		strcat(output, "\n");

	}

}

void gimme_count(char *output)

{

	int count = 0;

	for (auto &i : dict)

	{

		count++;

	}

	string str_count = to_string(count);

	strcat(output, str_count.c_str());

	strcat(output, "\n");

}

void remove(string &key, char *output)

{

	int ret = dict.erase(key);

	if (ret == 0)

	{

		string res = "NULL\n";

		strcat(output, res.c_str());

	}

	else

	{

		string res = "FIN\n";

		strcat(output, res.c_str());

		

	}

}

int func(char *input, char *output)

{

	output[0] = '\0';

	char delim = '\n';

	istringstream ss(input);

	string token;

	vector<string> tokens;

	while (getline(ss, token, delim))

	{

		tokens.push_back(token);

	}

	printf("printing lines.......\n");

	for (int i = 0; i < tokens.size(); i++)

	{

		auto &str = tokens[i];

		if (str == "WRITE")

		{

			while (lock == 0){

			//do nothing

			}

			aquire();

			auto &key = tokens[i + 1];

			auto &val = tokens[i + 2];

			write_into(&i, key, val);

			strcat(output, "FIN\n");

			release();

		}

		else if (str == "READ")

		{

			auto &key = tokens[i + 1];

			read_from(key, output);

			i++;

		}

		else if (str == "COUNT")

		{

			gimme_count(output);

		}

		else if (str == "DELETE")

		{

			while(lock == 0){

			

			}

			aquire();

			auto &key = tokens[i + 1];

			remove(key, output);

			i++;

			release();

		}

		else if (str == "END")

		{

			strcat(output, "\n");

			return -1;

		}

	}

	return 0;

}



void *serve(void *randomm)

{

	active_threads++;

	printf("active thread count%d\n", active_threads);

	int confd = *((int *)randomm);

	printf("in callback\n");

	while (1)

	{

		char buffer[1500];

		int val = read(confd, buffer, 1500);

		// printf("valiue of val %d", val);

		if (val > 0)

		{

			char response[2048];

			int returnval = func(buffer, response);



			printf("return value at line 103%d\n", returnval);

			printf("response at 104%s\n", response);

			if (returnval == -1)

			{

				char buff[100] = {"recieved message\n"};

				strcpy(buff, response);

				write(confd, buff, strlen(buff));

				printf("Terminating\n");

				close(confd);

				shutdown(confd, SHUT_RDWR);

				active_threads--;

				break;

			}

			char buff[2048] = {"this is a recieve buffer"};

			strcpy(buff, response);

			write(confd, buff, strlen(buff));

		}

	}

}



int main(int argc, char **argv)

{

	int portno;

	if (argc != 2)

	{

		fprintf(stderr, "usage: %s <port>\n", argv[0]);

		exit(1);

	}

	// DONE: Server port number taken as command line argument

	portno = atoi(argv[1]);

	printf("%d\t", portno);



	struct sockaddr_in server, client;

	int client_len, new_socket;

	client_len = sizeof(server);



	char buffer[1500];



	int sock = socket(AF_INET, SOCK_STREAM, 0);

	memset((char *)&server, 0, sizeof(server));

	server.sin_family = AF_INET;

	// defining incoming address and portnumber

	//inet_pton(AF_INET, "192.168.0.115", &server.sin_addr);

	server.sin_addr.s_addr = htonl(INADDR_ANY);

	server.sin_port = htons(portno);

	// binding



	bind(sock, (struct sockaddr *)&server, sizeof(server));

	printf("starting TCP server\n");

	listen(sock, 5);

	pthread_t thread_pool[1001]; // Array to hold thread IDs

	int thread_count = 0;

	int returnvalue = 0;

	while (1)

	{

		new_socket = accept(sock, (struct sockaddr *)&client, (socklen_t *)&client_len);

		if (new_socket >= 0 && thread_count<101)

		{

			pthread_create(&thread_pool[thread_count], NULL, &serve, &new_socket);

			thread_count++;

		}

		else{

			printf("pushing");

			mcq.push(new_socket);

			for (int i = 0; i < 101; i++){

				int threadTerminationStatus = pthread_tryjoin_np(thread_pool[i], NULL);

				if (threadTerminationStatus == 0){

					pthread_join(thread_pool[i], NULL);

					int poped = mcq.front();

					mcq.pop();

					printf("printing now\n%d",i);

					pthread_create(&thread_pool[i], NULL, &serve, &poped);

				}

				printf("return join error %d\n", threadTerminationStatus);

			}	

		}

	}

	for (int i = 0; i < thread_count; i++)

	{

		pthread_join(thread_pool[i], NULL);

	}

	close(sock);

	shutdown(sock, SHUT_RDWR);

	return 0;

}
