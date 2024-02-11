#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <netinet/in.h>
#include <unistd.h>
#include <sys/socket.h>
#include <unordered_map>
#include <sstream>
#include <iostream>
#include <vector>
using namespace std;

unordered_map<string, string> dict;

class serve
{
private:
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
			string res = "NULL";
			strcat(output, res.c_str());
			strcat(output, "\n");
		}
		else
		{
			string res = "FIN";
			strcat(output, res.c_str());
			strcat(output, "\n");
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
				auto &key = tokens[i + 1];
				auto &val = tokens[i + 2];
				write_into(&i, key, val);
				strcat(output, "FIN\n");
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
				auto &key = tokens[i + 1];
				remove(key, output);
				i++;
			}
			else if (str == "END")
			{
				strcat(output, "\n");
				return -1;
			}
		}
		return 0;
	}

public:
	int confd;
	serve(int confile)
	{
		printf("in callback\n");
		confd = confile;
		while (1)
		{
			char buffer[1500];
			read(confd, buffer, 1500);

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
				break;
			}
			char buff[100] = {"this is a recieve buffer"};
			strcpy(buff, response);
			write(confd, buff, strlen(buff));
		}
	}
};

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
	server.sin_addr.s_addr = htonl(INADDR_ANY);
	server.sin_port = htons(portno);
	// binding
	bind(sock, (struct sockaddr *)&server, sizeof(server));
	printf("starting TCP server\n");
	listen(sock, 5);
	while (1)
	{
		new_socket = accept(sock, (struct sockaddr *)&client, (socklen_t *)&client_len);
		if (new_socket >= 0)
		{
			serve obj(new_socket);
		}
	}
	close(sock);
	shutdown(sock, SHUT_RDWR);
	return 0;
}
