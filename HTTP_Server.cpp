// Server side C program to demonstrate Socket programming
#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/sendfile.h>
#include <sys/stat.h>
#include <errno.h>

#include <vector>
#include <random>
#include <chrono>
#include <algorithm>

#include <FuncA.h>

#define PORT 8081


void sendGETresponse(int fd, char strFilePath[], char strResponse[]);
void sendPUTresponse(int fd, char strFilePath[], char strBody[], char strResponse[]);
void sendHEADresponse(int fd, char strFilePath[], char strResponse[]);

char HTTP_200HEADER[] = "HTTP/1.1 200 Ok\r\n";
char HTTP_201HEADER[] = "HTTP/1.1 201 CREATED\r\n";
char HTTP_404HEADER[] = "HTTP/1.1 404 Not Found\r\n";
char HTTP_400HEADER[] = "HTTP/1.1 400 Bad request\r\n";



int CreateHTTPserver()
{
    int connectionSocket, clientSocket, pid; 
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    
    if ((connectionSocket = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("socket open failed\n");
        exit(EXIT_FAILURE);
    }
    
    address.sin_family      = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port        = htons( PORT );

    memset(address.sin_zero, '\0', sizeof address.sin_zero);
    
    if (bind(connectionSocket, (struct sockaddr *)&address, sizeof(address)) < 0)
    {
        perror("socket bind failed\n");
        close(connectionSocket);
        exit(EXIT_FAILURE);
    }
    
    if (listen(connectionSocket, 10) < 0)	// queue up up to 10 connections ready for accept
    {
        perror("socket listen failed\n");
        exit(EXIT_FAILURE);
    }
    
    while(1)
    {
        printf("\n+++++++ Waiting for a new connection ++++++++\n\n");
        
        if ((clientSocket = accept(connectionSocket, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0)
        {
            perror("Error accept()");
            exit(EXIT_FAILURE);
        }
        
      
	// Create child process to handle request from different client
	pid = fork();
	
	if (pid < 0)	
	{
		perror("Error on fork");
		exit(EXIT_FAILURE);
	}   

	if (pid == 0) // child process
	{
	    char  buffer[30000] = {0};
            char* ptrBuffer = &buffer[0];
            
            int iBytesRead = read(clientSocket, ptrBuffer, 30000);
            printf("\nClient message of %d bytes:\n%s\n", iBytesRead, buffer);
            
            if (iBytesRead == 0)
            {
		printf("Client closed connection prematurely\n");
		close(clientSocket);
		continue;
  	    }

            printf("\nParsing request...\n");

            // Parse Request method
            char strHTTP_requestMethod[10] = {0};
            char* pch = strchr(ptrBuffer, ' ');
            strncpy(strHTTP_requestMethod, ptrBuffer, pch-ptrBuffer);
            printf("Client method: %s\n", strHTTP_requestMethod);
            
            ptrBuffer = pch+1;

            // Parse Request path
            char strHTTP_requestPath[200] = {0};
            pch = strchr(ptrBuffer, ' ');
            strncpy(strHTTP_requestPath, ptrBuffer, pch-ptrBuffer);
            printf("Client asked for path: %s\n", strHTTP_requestPath);
            
            // Parse Request extension
            char strHTTPreqExt[200] = {0};
            pch = strrchr(strHTTP_requestPath, '.');
            if (pch != NULL) strcpy(strHTTPreqExt, pch+1);
            

            char strFilePath[500] = {0};
            char strResponse[500] = {0};


            if (!strcmp(strHTTP_requestMethod, "GET"))
            {
                if(!strcmp(strHTTP_requestPath, "/"))
                {
                    // strHTTP_requestPath == "/"  --> Send index.html file
                    sprintf(strFilePath, "./index.html");
                    sprintf(strResponse, "%s%s", HTTP_200HEADER, "Content-Type: text/html\r\n");

                    sendGETresponse(clientSocket, strFilePath, strResponse);
                }
                else if(!strcmp(strHTTP_requestPath, "/compute"))
		{
			
			auto t1 = std::chrono::high_resolution_clock::now();	

			std::vector<double> aValues;
            aValues.reserve(1000000);
            FuncA obj;
			// Mersenne Twister random engine
			std::mt19937 mtre {123};
			std::uniform_int_distribution<int> distr {5, 25};

			for (int i=0; i<1000000; i++) {
				aValues.push_back(obj.calculate(distr(mtre)));
			}

            for(int i = 0; i < 1200; i++)
            {
                sort(begin(aValues), end(aValues));
            }
			
			auto t2 = std::chrono::high_resolution_clock::now();
			auto int_ms = std::chrono::duration_cast<std::chrono::milliseconds>(t2-t1);

			int iMS = int_ms.count();

			char strTimeEllapsed[20];
			sprintf(strTimeEllapsed, "%i", iMS);

			sprintf(strResponse, "%sContent-type: text/html\r\nContent-Length: %ld\r\n\r\n", HTTP_200HEADER, strlen(strTimeEllapsed));
			
			write(clientSocket, strResponse, strlen(strResponse));
			printf("\nResponse: \n%s\n", strResponse);

			write(clientSocket, strTimeEllapsed, strlen(strTimeEllapsed));
			printf("%s\n", strTimeEllapsed);
		}
		else if ((!strcmp(strHTTPreqExt, "JPG")) || (!strcmp(strHTTPreqExt, "jpg")))
                {
                    //send image to client

                    sprintf(strFilePath, ".%s", strHTTP_requestPath);
                    sprintf(strResponse, "%s%s", HTTP_200HEADER, "Content-Type: image/jpeg\r\n");

                    sendGETresponse(clientSocket, strFilePath, strResponse);
                }
                else if (!strcmp(strHTTPreqExt, "ico"))
                {
                    //https://www.cisco.com/c/en/us/support/docs/security/web-security-appliance/117995-qna-wsa-00.html

                    sprintf(strFilePath, "./img/favicon.png");
                    sprintf(strResponse, "%s%s", HTTP_200HEADER, "Content-Type: image/vnd.microsoft.icon\r\n");

                    sendGETresponse(clientSocket, strFilePath, strResponse);
                }
                else if (!strcmp(strHTTPreqExt, "js"))
                {
                    sprintf(strFilePath, ".%s", strHTTP_requestPath);
                    sprintf(strResponse, "%s%s", HTTP_200HEADER, "Content-Type: text/javascript\r\n");

                    sendGETresponse(clientSocket, strFilePath, strResponse);
                }
                else if (!strcmp(strHTTPreqExt, "png"))
                {
                    sprintf(strFilePath, ".%s", strHTTP_requestPath);
                    sprintf(strResponse, "%s%s", HTTP_200HEADER, "Content-Type: image/png\r\nCache-Control: max-age=3600\r\n");

                    sendGETresponse(clientSocket, strFilePath, strResponse);
                }
                else  // unknown mime type
                {
                    sprintf(strFilePath, ".%s", strHTTP_requestPath);
                    sprintf(strResponse, "%s%s", HTTP_200HEADER, "Content-Type: text/plain\r\n");

                    sendGETresponse(clientSocket, strFilePath, strResponse);
                }
            }
            else if (!strcmp(strHTTP_requestMethod, "PUT"))
            {
		// search for body
		ptrBuffer = strstr(buffer, "\r\n\r\n");
		ptrBuffer +=4;
				
		if (ptrBuffer)
		{
                    sprintf(strFilePath, ".%s", strHTTP_requestPath);
  		    sprintf(strResponse, "%s", HTTP_201HEADER);
					
                    sendPUTresponse(clientSocket, strFilePath, ptrBuffer, strResponse);
		}
            }
            close(clientSocket);
            
	    return 0;
        }
	else // parent process branch
	{
		printf(">>>>>>>> Forked a child with pid: %d <<<<<<<<<<\n", pid);

		close(clientSocket);
	}
    }
    
    close(connectionSocket);
    
    return 0;
}



void sendGETresponse(int fdSocket, char strFilePath[], char strResponse[])
{
    int fdFile = open(strFilePath, O_RDONLY);
    if (fdFile < 0)
    {
	sprintf(strResponse, "%s", HTTP_404HEADER);
	write(fdSocket, strResponse, strlen(strResponse));
		
        printf("\nCannot open file path : %s with error %d\n", strFilePath, fdFile);
        printf("Response:\n%s\n", strResponse); 
        
        return;
    }
     
    struct stat stat_buf;  // hold information about input file
    fstat(fdFile, &stat_buf);
    
    int file_total_size = stat_buf.st_size;
    int block_size      = stat_buf.st_blksize;
    
    // append another header
    char* strOffset = strResponse + strlen(strResponse);
    sprintf(strOffset, "Content-Length: %d\r\n\r\n", file_total_size);

    printf("\nResponse:\n%s\n", strResponse); 
    int iRes = write(fdSocket, strResponse, strlen(strResponse));
    if (iRes < 0)
    {
        printf("\nCannot write to client socket with error %d\n", iRes);
        return;
    }
		
	while(file_total_size > 0)
	{
		int iToSend = ((file_total_size < block_size) ? file_total_size : block_size);
		int done_bytes = sendfile(fdSocket, fdFile, NULL, iToSend);
		if (done_bytes < 0)
		{
			printf("\nCannot write to client socket with error %d\n", done_bytes);
			return;
		}
		  
		file_total_size -= done_bytes;
	}
	close(fdFile);
}


void sendPUTresponse(int fdSocket, char strFilePath[], char strBody[], char strResponse[])
{
    int fdFile = open(strFilePath, O_WRONLY|O_CREAT|O_TRUNC, 0666);
    if (fdFile < 0)
    {
	sprintf(strResponse, "%s", HTTP_400HEADER);
	write(fdSocket, strResponse, strlen(strResponse));
		
        printf("\nCannot save file path : %s with error %d\n", strFilePath, fdFile);
        printf("Response:\n%s\n", strResponse); 
        
        return;
    }
    
    printf("\nResponse:\n%s\n", strResponse); 
    int iRes = write(fdSocket, strResponse, strlen(strResponse));
    if (iRes < 0)
    {
        printf("\nCannot write to client socket with error %d\n", iRes);
        return;
    }
    
    iRes = write(fdFile, strBody, strlen(strBody));
    if (iRes < 0)
    {
        printf("\nCannot write to file %s with error %d\n", strFilePath, fdFile);
        
        return;
    }
    
    close(fdFile);
}

