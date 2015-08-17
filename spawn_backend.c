/*************************************************************************
 *  --------->    File Name: spawn.c
 *  --------->    Author: chengfeiZH
 *  --------->    Mail: chengfeizh@gmail.com
 *  --------->    Time: 2015年07月28日 星期二 10时52分13秒
 ************************************************************************/
 
// Example taken from wiki.xensource.com/xenwiki/XenStoreReference and some code added   
//   
// Compiled using   
//        gcc -g xenstore.c -DREAD=1 -lxenstore -o xenstore-read   
//        gcc -g xenstore.c -DWRITE=1 -lxenstore -o xenstore-write   
//   
// To change permissions on /local/domain/0, use   
// xenstore-chod -r /local/domain/0 b  - for r and w   
// xenstore-chod -r /local/domain/0 w  - for w   

#include <unistd.h>
#include <assert.h>
#include <sys/types.h>   
#include <xenstore.h>   
#include <stdio.h>   
#include <string.h>   
#include <malloc.h>   
#include <sys/stat.h>
#include <stdlib.h>

typedef struct config
{
    char path[64];
    char name[64];
    char mem[16];
} Config;

/*
 * prog : mini-os启动文件路径
 * name : domain名字
 * mem : domain内存大小
 */
void backend_spawn(const Config conf)
{
    pid_t pid = fork();
    if (pid == 0) {
        printf("New mini-os creating... ...\n");
        char confPath[1028];
        strcpy(confPath, conf.path);
        strcat(confPath, "temp.conf");
        printf("path: %s\n", confPath);
        FILE *stream = fopen(confPath, "w");
        assert(stream);
        char content[1024] = {0};
        strcat(content, "kernel = \"mini-os.gz\"\n");
        strcat(content, "name = \"");
        strcat(content, conf.name);
        
        strcat(content, "\"\nvif = [\"bridge=xenbr0,ip=192.168.182.13\"]");
        strcat(content, "\nmemory = ");
        strcat(content, conf.mem);
        strcat(content, "\non_crash = \'destroy\'\n");
        printf("%s\n", content); 
        assert(fwrite(content, strlen(content), 1, stream));
        fclose(stream);
        system("./xl-test create -c \"temp.conf\"");
    } else if (pid < 0) {
        perror("fork");
    } else {
        printf("parent process\n");
    }
}

int main() {  
    struct xs_handle *xs;     // handle to xenstore   
    xs_transaction_t trans;  
    char *path;  
    int fd;  
    fd_set set;  
    bool err;  
    struct timeval tv = {.tv_sec = 0, .tv_usec = 0};  
    char **vec;  
    unsigned int num;  
    char *buf;  
    char **buf2;  
    unsigned int len, i, domid;  
    char domID[16];  
    char *name = "Mini-OS_Server";
    char namePath[1024];
    char *tmpName;
    
    xs = xs_daemon_open(); 
    
    if (xs == NULL) {
        error();  
    }
    
    trans = xs_transaction_start(xs);  
    if (trans == 0) {  
        printf("---> Could not start xaction with XS\n");  
        return ;  
    }  
    
    // Get contents of a directory. Need to call free after use,   
    // since the API mallocs memory   
    buf2 = xs_directory(xs, trans, "/local/domain" , &len);  
    if (!buf2) {  
        printf("---> Could not read XS dir /local/domain\n" );  return ;  
    }  
    
    xs_transaction_end(xs, trans, true );  
    if (trans == 0) {  
        printf("---> Could not end xaction with XS\n" );  return ;  
    } 
     
    printf("---> Len of Dir /local/domain is %d\n" , len);  
    for (i=0; i<len;i++) {
        memset(namePath, 0, 1024 * sizeof(char));
		strcpy(namePath, "/local/domain/");
		strcat(namePath, buf2[i]);
		strcat(namePath, "/name");
		printf( "---> namePath = %s\n" , namePath); 
		tmpName = xs_read(xs, 0, namePath, NULL);
		printf("---> tmpName = %s\n", tmpName);
		if (strcmp(tmpName, name) == 0) {
	        printf("---> OKOK! find id by name!\n");
			strcpy(domID, buf2[i]);
			domid = atoi(buf2[i]);
			break;
		}
    }  

    printf("---> Setting Dom ID = %s\n" , domID);  
    
    // Get the local Domain path in xenstore   
    path = xs_get_domain_path(xs, domid);  
    if (path == NULL) {  
        printf("---> Dom Path in Xenstore not found\n" );  
        error(); 
        return ;  
    }  
    
    // xs_directory has an implicit root at /local/domain/<domid> in DomU   
    // and thus need to just pass "memory/target" if we are running this   
    // from a DomU to read the directory's contents   
    char tmp[1024];
	strcpy(tmp, path);
	strcat(tmp, "/console");
	path = tmp;
         
    printf("---> Path = %s\n" , path);  
     
    // If we are doing a write to the xenstore, branch off here and   
    // then exit from the program. Else for read, cary on, and do a   
    // select() to wait for a change in watched values   
    // Create a watch on the path   
    err = xs_watch(xs, path, "mypeer" );  
    if (err == 0) {  
        printf("---> Error in setting watch on mytoken in %s\n" , path);  
        error(); 
        return ;  
    }  
    // Watches are notified via a File Descriptor. We can POLL on this.   
    fd = xs_fileno(xs);  
    while (1) {  
        FD_ZERO(&set);  // clear the set 
        FD_SET(fd, &set);  // add a fd into set
        fflush(stdout);  
        struct  timeval tv = {.tv_sec = 5, .tv_usec = 0};  
        if  (select(fd+1, &set, NULL, NULL, &tv) > 0  
            && FD_ISSET(fd, &set)) {  
            // This blocks is nothing is pending. Returns an array   
            // containing path and token. Use Xs_WATCH_* to   
            // access these elements. Call free after use.   
            vec = xs_read_watch(xs, &num);  
            if  (!vec) {  
                printf("---> Error on watch firing\n" );  
                error(); 
                return ;  
            }  
            // In our example code, the following will print out   
            // /local/domain/0/memory/target|mytoken   
            printf("---> vec contents: %s|%s\n" , vec[XS_WATCH_PATH],  
                    vec[XS_WATCH_TOKEN]);  
            // Prepare a transacation to do a read   
            trans = xs_transaction_start(xs);  
            if  (trans == 0) {  
				printf("---> Could'nt start xaction xenstore\n" );  
                return ;  
            }  
            buf = xs_read(xs, trans, vec[XS_WATCH_PATH], &len);  
            if  (!buf) {  
				printf("---> Could'nt read watch var in vec\n" );  
                return ;  
            }  
            xs_transaction_end(xs, trans, true );  
            if  (trans == 0) {  
				printf("---> Could not end xaction xenstore\n" );  
                return ;  
            }  
            if  (buf) {  
                printf("---> buflen: %d, buf: %s\n" , len, buf);  
				if (strcmp(buf, "192.168.182.13") == 0) {
					printf("---> start a new mini-os\n");
				    Config conf;
				    strcpy(conf.path, "/root/xen-4.2.1/stubdom/mini-os-x86_32-c/");
				    strcpy(conf.name, "mini-os_server_new");
					strcpy(conf.mem, "32");
					backend_spawn(conf);
					break;
				}
            }  
        } // end of select   
    } // end while(1)   
   
    // cleanup   
    close(fd);  
    xs_daemon_close(xs);  
    //free(path);  
    return ;  
}  

