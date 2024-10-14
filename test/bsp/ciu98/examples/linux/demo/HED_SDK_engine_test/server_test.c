#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <openssl/crypto.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/engine.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

//#define ECC_CERT_TEST
 #define RSA_CERT_TEST

#ifdef ECC_CERT_TEST
//server certificate
#define SEVER_CRT_FILE  "server.crt"
//CA certificate 
#define CA_CRT_FILE "ca.crt"
//server private key
#define PRI_KEY "serverpri.key"
#endif

#ifdef RSA_CERT_TEST
//server certificate
#define SEVER_CRT_FILE  "server_rsa.crt"
//CA certificate
#define CA_CRT_FILE "ca_rsa.crt"
//server private key
#define PRI_KEY "server_rsa.key"
#endif

//set IP port
#define DEFAULT_IP              "127.0.0.1"
#define DEFAULT_PORT            5000
// set security mode
#define SECURE_COMM		TLS_server_method()
//#define SECURE_COMM		DTLS_server_method()

typedef enum {
	SOCKET_IS_NONBLOCKING,
	SOCKET_IS_BLOCKING,
	SOCKET_HAS_TIMED_OUT,
	SOCKET_HAS_BEEN_CLOSED,
	SOCKET_OPERATION_OK
} timeout_state;

extern  int waitpid();

void serverListen(void);
void SSLConnected(int,int);

int main (int argc, char *argv[])
{

	
  serverListen();

	return 0;
}

void serverListen(void)
{
	int                     err;
	int                     error=0;
	int                     pid;
	int                     listen_sock;
	int                     sock;
	struct sockaddr_in      sa_serv;
	struct sockaddr_in      sa_cli;
	size_t                  client_len;
	short int               s_port = DEFAULT_PORT;
	int                     connect=0;


   printf("\n+++++++++++++++++++++++++++++++\n");
	// set Socket
	listen_sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP); 
	if (listen_sock == -1)
	{
		error=1;
	}
	memset(&sa_serv, '\0', sizeof(sa_serv));
	sa_serv.sin_family = AF_INET;
	sa_serv.sin_addr.s_addr = INADDR_ANY;
	sa_serv.sin_port = htons(s_port);
	err = bind(listen_sock, (struct sockaddr*)&sa_serv,sizeof(sa_serv));
	if (err == -1)
	{
		error=1;
	}

	/*wait for TCP connect */
	err = listen(listen_sock, 5);
	if (err == -1)
	{
		error=1;
	}
	client_len = sizeof(sa_cli);

	while (error == 0)
	{
    printf("\nListening to incoming connection\n");
		sock = accept(listen_sock, (struct sockaddr*)&sa_cli,(socklen_t *) &client_len);
		if (sock == -1)
		{
			error=1;
			break;
		}

			printf("Connection from %d.%d.%d.%d, port :0x%.4x",
				sa_cli.sin_addr.s_addr & 0x000000ff,
				(sa_cli.sin_addr.s_addr & 0x0000ff00) >> 8,
				(sa_cli.sin_addr.s_addr & 0x00ff0000) >> (8*2),
				(sa_cli.sin_addr.s_addr & 0xff000000) >> (8*3),
				sa_cli.sin_port);
		
		pid = fork();
		if (pid == -1)
		{
		    error=1;
		}

		if (pid == 0)
		{
			connect = getpid();
			close(listen_sock);
			SSLConnected(sock,connect);
      printf("\n[%d] Return from Child", connect);
			error=1;
		}
	}
	// close socket
	err = close(sock);
	if (err == -1)
	{
		error=1;
	}
}

void SSLConnected(int sock, int connect)
{
	int             err;
	int             len;
	int             error=0;
	clock_t		start, end;
	SSL_CTX         *ctx;
	SSL             *ssl;
	SSL_METHOD      *meth;
	char         buf[4096];
	ENGINE          *e;
	EVP_PKEY        *pkey;
	UI_METHOD       *ui_method;
	EC_KEY *ecdh;

	do {
	    // initialize openssl
	    SSL_library_init();
	    SSL_load_error_strings();
	   	meth = (SSL_METHOD*) SECURE_COMM;
      
      //generate SSL_CTX
	    ctx = SSL_CTX_new(meth);    
	    if (!ctx)
	    {
    		ERR_print_errors_fp(stderr);
    		exit(1);
	    }

#ifdef ECC_CERT_TEST
	    ecdh = EC_KEY_new_by_curve_name(NID_X9_62_prime256v1);
	    SSL_CTX_set_tmp_ecdh(ctx,ecdh);
#endif 
  
	   SSL_CTX_use_PrivateKey_file(ctx, PRI_KEY, SSL_FILETYPE_PEM);
    
	   
     //load cert 
	    if(SSL_CTX_use_certificate_file(ctx,SEVER_CRT_FILE, SSL_FILETYPE_PEM) <= 0)
	    {
		   printf("Load Certificate Fail\n");
		   break;
	    }
	    printf("Load Certificate ok\n");

	    // check whether the private key match the certificate
	   if(!SSL_CTX_check_private_key(ctx))
	   {
    		printf("Private Key do not Match the Server Certificate!!!!\n");
    		break;
	   }
	    printf("Private Key Match the Server Certificate.\n");

	   //load the CA certificate
	   if(!SSL_CTX_load_verify_locations(ctx, CA_CRT_FILE, NULL))
	   {
    		printf("Load CA cert Fail\n");
    		break;
	   }
	      printf("Load CA cert ok\n");
 
	  
	   SSL_CTX_set_verify(ctx, SSL_VERIFY_PEER, NULL);

	   SSL_CTX_set_verify_depth(ctx,1);
	}while(0);

    if(error==0)
    {
	// generate a new SSL
	ssl = SSL_new(ctx);
	if (ssl == NULL)
	{
	    error = 1;
	}

	// SSLput socket into SSL
	SSL_set_fd(ssl, sock);
	// open SSL connecting
  printf("waiting for the client connection!");
	err = SSL_accept(ssl);
  
	switch(err)
	{
		case 1:
			printf("The cipher suite is : %s\n", SSL_get_cipher(ssl));
			printf("The SSL/TLS version is : %s\n", SSL_get_version(ssl));
			start = clock();
			while(1)
			{
				len = SSL_read(ssl, buf, sizeof(buf) - 1);
				err = SSL_get_error(ssl, len);

				if ((err != 0) && (len == 0))
				{
				    end = clock();
				    if (((end-start)/1000000) > 5)
				    {
					printf("[%d] Timeout !!\n",connect);
					break;
				    }
				}
				else
				{
					start = clock();
					printf("[%d] the data from client is : %d\n", connect, buf[0]);
					if (buf[0] > 100)
						break;

					sprintf(buf,"From Server [%d] : %.3d",connect, buf[0]);
					err = SSL_write(ssl, buf,strlen(buf));
					if (err==-1)
					{
						ERR_print_errors_fp(stderr);
					}
				}
			}
			break;
		case 0:
			printf("Connection Refuse\n ");
			break;
		default:
			printf("SSL Error!!! %d\n",err);
	}

  //close SSL connecting
	err = SSL_shutdown(ssl);
	printf("Connection Closed!!!\n");
    }

    //free SSL 
    ENGINE_free(e);
    SSL_free(ssl);
    SSL_CTX_free(ctx);
    printf("Leaving Routine!!!\n");
}
