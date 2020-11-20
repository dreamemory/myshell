#include<bits/stdc++.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<string.h>
#include<dirent.h>
#include<sys/types.h>
#include <sys/wait.h>
#include <sys/signal.h>
#include <sys/types.h>
#include <errno.h>
#include <pwd.h>

#define BUF_SZ 256
#define TRUE 1
#define FALSE 0
using namespace std;
const char* COMMAND_RM="rm";
const char* COMMAND_EXIT = "exit";
const char* COMMAND_HELP = "help";
const char* COMMAND_CD = "cd";
const char* COMMAND_IN = "<";
const char* COMMAND_OUT = ">";
const char* COMMAND_PIPE = "|";
const char* COMMAND_CP="cp";
const char* COMMAND_MV="mv";
const char* COMMAND_LS="ls";
const char* COMMAND_CAT="cat";
const char* COMMAND_WC="wc";

// 内置的状态码
enum {
	RESULT_NORMAL,
	ERROR_FORK,
	ERROR_COMMAND,
	ERROR_WRONG_PARAMETER,
	ERROR_MISS_PARAMETER,
	ERROR_TOO_MANY_PARAMETER,
	ERROR_CD,
	ERROR_SYSTEM,
	ERROR_EXIT,

	/* 重定向的错误信息 */
	ERROR_MANY_IN,
	ERROR_MANY_OUT,
	ERROR_FILE_NOT_EXIST,

	/* 管道的错误信息 */
	ERROR_PIPE,
	ERROR_PIPE_MISS_PARAMETER
};

char username[BUF_SZ];
char hostname[BUF_SZ];
char curPath[BUF_SZ];
char commands[BUF_SZ][BUF_SZ];

int isCommandExist(const char* command) { // 判断指令是否存在
	if (command == NULL || strlen(command) == 0) return FALSE;

	int result = TRUE;

	int fds[2];
	if (pipe(fds) == -1) {
		result = FALSE;
	} else {
		/* 暂存输入输出重定向标志 */
		int inFd = dup(STDIN_FILENO);
		int outFd = dup(STDOUT_FILENO);

		pid_t pid = vfork();
		if (pid == -1) {
			result = FALSE;
		} else if (pid == 0) {
			/* 将结果输出重定向到文件标识符 */
			close(fds[0]);
			dup2(fds[1], STDOUT_FILENO);
			close(fds[1]);

			char tmp[BUF_SZ];
			sprintf(tmp, "command -v %s", command);
			system(tmp);
			exit(1);
		} else {
			waitpid(pid, NULL, 0);
			/* 输入重定向 */
			close(fds[1]);
			dup2(fds[0], STDIN_FILENO);
			close(fds[0]);

			if (getchar() == EOF) { // 没有数据，意味着命令不存在
				result = FALSE;
			}

			/* 恢复输入、输出重定向 */
			dup2(inFd, STDIN_FILENO);
			dup2(outFd, STDOUT_FILENO);
		}
	}

	return result;
}

void getUsername() { // 获取当前登录的用户名
	struct passwd* pwd = getpwuid(getuid());
	strcpy(username, pwd->pw_name);
}

void getHostname() { // 获取主机名
	gethostname(hostname, BUF_SZ);
}

void getCurWorkDir() { // 获取当前的工作目录
	 getcwd(curPath, BUF_SZ);
}

int splitCommands(char command[BUF_SZ]) { // 以空格分割命令， 返回分割得到的字符串个数
	int num = 0;
	int i, j;
	int len = strlen(command);

	for (i=0, j=0; i<len; ++i) {
		if (command[i] != ' ') {
			commands[num][j++] = command[i];
		} else {
			if (j != 0) {
				commands[num][j] = '\0';
				++num;
				j = 0;
			}
		}
	}
	if (j != 0) {
		commands[num][j] = '\0';
		++num;
	}

	return num;
}

int callExit() { // 发送terminal信号退出进程
	pid_t pid = getpid();
	if (kill(pid, SIGTERM) == -1)
		return ERROR_EXIT;
	else return RESULT_NORMAL;
}



//               rm          //
//               **************
void del_stat(const char name[])
{
	DIR* fl = opendir(name);
	chdir(name);

	for(struct dirent* dir = readdir(fl); NULL!=dir;)
	{
		if(dir->d_type == DT_DIR)
		{
			if(strcmp(dir->d_name,"..")&&strcmp(dir->d_name,"."))
				del_stat(dir->d_name);
		}
		remove(dir->d_name);
		dir = readdir(fl);
	}
	chdir("..");
	remove(name);
}

  int callrm(int argcc,char* argvv[]){

     if(3 != argcc)
	{
		printf("please like this:rm -rf xxx\n");
		return -1;
	}

	struct stat sta;
	if(0 > stat(argvv[2],&sta))
	{
		perror("open");
		return -1;
	}

	if(!S_ISDIR(sta.st_mode))
	{
		char temp;
		printf("该文件是一个文件，是否删除(y)、(n):");
		scanf("%c",&temp);
		if(temp == 'n')return 0;
		remove(argvv[2]);
		return 0;
	}

	del_stat(argvv[2]);


      return 0;
}
//                end                 *************** //
//
//
//           cp          *********************//
//
//
//
void callcp(int argc, char * argv[] )
{
	FILE * source = fopen(argv[1],"r");
	if(source==NULL)
	{
		perror ("file no exsit!\n");
		return ;
	}
	else
	{
		struct stat * statbuf=(struct stat *)malloc(sizeof(statbuf));
		stat(argv[2], statbuf); //判断目标路径是一个普通文件还是一个目录
		if(S_ISDIR (statbuf->st_mode)) //目标是个目录
		{
				int i,j,k=0;
				//因为目标路径是一个文件夹，所以默认拷贝一个和源文件名字一样的名字
				//要找到源文件的名字，就要找源路径最后一个'/'后面的字符串就是源文件名！！
				for(i=strlen(argv[1])-1;i>=0;i--)
				{
					if(argv[1][i]=='/') //找到了最后一个'/'的下标
					{
						break; //找到就退出 ，保留'/'下标为i
					}
				}
				char source_file_name[128]={};//用来存储源文件的名字
				for(j=i+1;j<strlen(argv[1]);j++)
				{	//把源路径最后一个'/'后面的字符串给source_file_name,它就是源文件名字
					source_file_name[k++]=argv[1][j];
				}
				//如果目标路径最后面没有‘/’，则需要加一个'/'
				if(argv[2][strlen(argv[2])-1]!='/')
				{
					strcat (argv[2],"/");
				}
				//把目标路径和源文件名拼接起来，成为一个目标文件名，并创建打开它
				FILE * target = fopen(strcat (argv[2],source_file_name),"w+");
				while(!feof(source))
				{	//把源文件内容全部传给目标文件
					char  buf[10]={};
					fread (buf,1,10,source);
					fwrite (buf,1,10, target);
				}
					fclose (target);
		}
		else//目标路径是个文件
		{
			FILE * target = fopen(argv[2],"w+");
			while(!feof(source))
			{//把源文件内容全部传给目标文件
				char  buf[10]={};
				fread (buf,1,10,source);
				fwrite (buf,1,10, target);
			}
				fclose (target);
		}
		fclose (source);
	}
}


//       end  ....//
//
//        mv     ***************//
void callmv(int argc, char * argv[] )
{
	FILE * source = fopen(argv[1],"r");
	if(source==NULL)
	{
		perror ("file no exsit!\n");
		return ;
	}
	else
	{
		struct stat * statbuf=(struct stat *)malloc(sizeof(statbuf));
		stat(argv[2], statbuf); //判断目标路径是一个普通文件还是一个目录
		if(S_ISDIR (statbuf->st_mode)) //目标是个目录
		{
				int i,j,k=0;
				//因为目标路径是一个目录，所以默认拷贝一个和源文件名字一样的名字
				//要找到源文件的名字，就要找源路径最后一个'/'后面的字符串就是源文件名！！
				for(i=strlen(argv[1])-1;i>=0;i--)
				{
					if(argv[1][i]=='/') //找到了最后一个'/'的下标
					{
						break; //找到就退出 ，保留'/'下标为i
					}
				}
				char source_file_name[128]={};//用来存储源文件的名字
				for(j=i+1;j<strlen(argv[1]);j++)
				{	//把源路径最后一个'/'后面的字符串给source_file_name,它就是源文件名字
					source_file_name[k++]=argv[1][j];
				}
				//如果目标路径最后面没有‘/’，则需要加一个'/'
				if(argv[2][strlen(argv[2])-1]!='/')
				{
					strcat (argv[2],"/");
				}
				//把目标路径和源文件名拼接起来，成为一个目标文件名，并创建打开它
				FILE * target = fopen(strcat (argv[2],source_file_name),"w+");
				while(!feof(source))
				{	//把源文件内容全部传给目标文件
					char  buf[10]={};
					fread (buf,1,10,source);
					fwrite (buf,1,10, target);
				}
					fclose(target);
		}
		else//目标路径是个文件
		{
			FILE * target = fopen(argv[2],"w+");
			while(!feof(source))
			{//把源文件内容全部传给目标文件
				char  buf[10]={};
				fread (buf,1,10,source);
				fwrite (buf,1,10, target);
			}
				fclose(target);
		}

	}
	remove(argv[1]);  //删除源文件
	fclose(source);

}
//  rm end*****************************//
//
//  ls **************8//
//
//
//



int callls(int argc,char *argv[]){
    DIR *dir;
    char str[100];
    string res[100];
    struct dirent *rent;//struct
    dir = opendir(argv[1]);
    int cnt = 0;
    while((rent=readdir(dir))){
        strcpy(str, rent->d_name);
        if(str[0] == '.')
            continue;
        if(!str)
            continue;
        //printf("%s  ", str);
        res[cnt++] = str;
    }
    //cout<<"cnt = "<<cnt<<endl;

    for(int i=0; i<cnt; i++)
        cout<<res[i]<<"  ";
    puts("");
    closedir(dir);
    return 0;

}
//
//
//
//  end  ***********//
//
//  wc  ****************8//
//
//
struct message{
	int lines;
	int words;
	int max_line_length;
	int size;
	int chars;
}info;
void error_print(char str[]){
	printf("Error:%s",str);
}
void init(char filename[]){
	struct stat get_message = {};
	FILE *fp;
	int ret_stat = stat(filename,&get_message);
	if(ret_stat == -1){
			error_print(filename);
			return ;
	}
	mode_t mode = get_message.st_mode;
	int length = 0;
	if(S_ISDIR(mode))
			printf("Error %s is dir\n0\t0\t0\t%s",filename,filename);
	else{
			info.size = get_message.st_size;
			fp = fopen(filename,"r");
			char ch;
			int flag = 0;
			while((ch = fgetc(fp))!=EOF){
					info.chars++;
					if(ch != '\n'){

							length++;
					}
					if(ch == '\n'){
							info.lines ++;
							if(length>info.max_line_length)
								info.max_line_length = length;
							length = 0;
					}
					if(ch == '\t' || ch == ' ' || ch == '\n'){
							flag = 0;
							continue;
					}
					else{

							if(flag == 0){
								info.words++;
									flag = 1;
							}
					}
			}
			fclose(fp);
	}

}

void EmptyFile(){
	char ch;
	int flag = 0;
	int length = 0;

	while((ch = getchar())!=EOF){
		info.chars++;
		info.size += sizeof(ch);
		if(ch != '\n'){
				length++;
		}
		if(ch == '\n'){
				info.lines ++;
				if(length>info.max_line_length)
					info.max_line_length = length;
				length = 0;
		}
		if(ch == '\t' || ch == ' ' || ch == '\n'){
				flag = 0;
				continue;
		}
		else{
 				if(flag == 0){
						info.words++;
						flag = 1;
				}
		}

	}
}
int callwc(int argc,char *argv[]){

	if(argc == 2){
			if(argv[1][0] != '-'){
					init(argv[1]);
					printf("%d %d %d %s\n",info.lines,info.words,info.size,argv[1]);
					return 0;
			}
			else{
				EmptyFile();

			}
	}
	else if(argc == 1){
			EmptyFile();
			printf("%d\t%d\t%d\n",info.lines,info.words,info.size);
			return 0;
	}
	else if(argc == 3){
			init(argv[2]);
	}
	int num;
	while((num = getopt(argc,argv,"lwmcL"))!=-1){
			switch(num){
					case 'l':
						printf("%d\t",info.lines);
						break;
					case 'w':
						printf("%d\t",info.words);
						break;
					case 'm':
						printf("%d\t",info.chars);
						break;
					case 'c':
						printf("%d\t",info.size);
						break;
					case 'L':
						printf("%d\t",info.max_line_length);
						break;
				}
	}
	if(argc != 2 && argv[1][0] != '-')
			printf("%s\n",argv[2]);


		return 0;
}




//
//  end           ///
//
//  cat **********************
//
//
//
//
int callcat(int argc, char *argv[])
{
    FILE *fp = fopen(argv[1], "r");
    int read_ret;
    if(argc < 2)
    {
        printf("please input source file!\n");
    }
    if(fp == NULL){
        printf("open source %s failed!\n", argv[1]);
        return -1;
    }
    while(1)
    {
        read_ret = fgetc(fp);
        if(feof(fp))
        {
            printf("read file %s endl\n",argv[1]);break;
        }
        fputc(read_ret,stdout);
    }
}



//
//
//  end********8


//
//
//
//
//
//
//
//
//
//
//
int callCommandWithRedi(int left, int right) { // 所要执行的指令区间[left, right)，不含管道，可能含有重定向
	if (!isCommandExist(commands[left])) { // 指令不存在
		return ERROR_COMMAND;
	}

	/* 判断是否有重定向 */
	int inNum = 0, outNum = 0;
	char *inFile = NULL, *outFile = NULL;
	int endIdx = right; // 指令在重定向前的终止下标

	for (int i=left; i<right; ++i) {
		if (strcmp(commands[i], COMMAND_IN) == 0) { // 输入重定向
			++inNum;
			if (i+1 < right)
				inFile = commands[i+1];
			else return ERROR_MISS_PARAMETER; // 重定向符号后缺少文件名

			if (endIdx == right) endIdx = i;
		} else if (strcmp(commands[i], COMMAND_OUT) == 0) { // 输出重定向
			++outNum;
			if (i+1 < right)
				outFile = commands[i+1];
			else return ERROR_MISS_PARAMETER; // 重定向符号后缺少文件名

			if (endIdx == right) endIdx = i;
		}
	}
	/* 处理重定向 */
	if (inNum == 1) {
		FILE* fp = fopen(inFile, "r");
		if (fp == NULL) // 输入重定向文件不存在
			return ERROR_FILE_NOT_EXIST;

		fclose(fp);
	}

	if (inNum > 1) { // 输入重定向符超过一个
		return ERROR_MANY_IN;
	} else if (outNum > 1) { // 输出重定向符超过一个
		return ERROR_MANY_OUT;
	}

	int result = RESULT_NORMAL;
	pid_t pid = vfork();
	if (pid == -1) {
		result = ERROR_FORK;
	} else if (pid == 0) {
		/* 输入输出重定向 */
		if (inNum == 1)
			freopen(inFile, "r", stdin);
		if (outNum == 1)
			freopen(outFile, "w", stdout);

		/* 执行命令 */
		char* comm[BUF_SZ];
		for (int i=left; i<endIdx; ++i)
			comm[i] = commands[i];
		comm[endIdx] = NULL;
		execvp(comm[left], comm+left);
		exit(errno); // 执行出错，返回errno
	} else {
		int status;
		waitpid(pid, &status, 0);
		int err = WEXITSTATUS(status); // 读取子进程的返回码

		if (err) { // 返回码不为0，意味着子进程执行出错，用红色字体打印出错信息
			printf("\e[31;1mError: %s\n\e[0m", strerror(err));
		}
	}


	return result;
}
int callCommandWithPipe(int left, int right) { // 所要执行的指令区间[left, right)，可能含有管道
	if (left >= right) return RESULT_NORMAL;
	/* 判断是否有管道命令 */
	int pipeIdx = -1;
	for (int i=left; i<right; ++i) {
		if (strcmp(commands[i], COMMAND_PIPE) == 0) {
			pipeIdx = i;
			break;
		}
	}
	if (pipeIdx == -1) { // 不含有管道命令
		return callCommandWithRedi(left, right);
	} else if (pipeIdx+1 == right) { // 管道命令'|'后续没有指令，参数缺失
		return ERROR_PIPE_MISS_PARAMETER;
	}

	/* 执行命令 */
	int fds[2];
	if (pipe(fds) == -1) {
		return ERROR_PIPE;
	}
	int result = RESULT_NORMAL;
	pid_t pid = vfork();
	if (pid == -1) {
		result = ERROR_FORK;
	} else if (pid == 0) { // 子进程执行单个命令
		close(fds[0]);
		dup2(fds[1], STDOUT_FILENO); // 将标准输出重定向到fds[1]
		close(fds[1]);

		result = callCommandWithRedi(left, pipeIdx);
		exit(result);
	} else { // 父进程递归执行后续命令
		int status;
		waitpid(pid, &status, 0);
		int exitCode = WEXITSTATUS(status);

		if (exitCode != RESULT_NORMAL) { // 子进程的指令没有正常退出，打印错误信息
			char info[4096] = {0};
			char line[BUF_SZ];
			close(fds[1]);
			dup2(fds[0], STDIN_FILENO); // 将标准输入重定向到fds[0]
			close(fds[0]);
			while(fgets(line, BUF_SZ, stdin) != NULL) { // 读取子进程的错误信息
				strcat(info, line);
			}
			printf("%s", info); // 打印错误信息

			result = exitCode;
		} else if (pipeIdx+1 < right){
			close(fds[1]);
			dup2(fds[0], STDIN_FILENO); // 将标准输入重定向到fds[0]
			close(fds[0]);
			result = callCommandWithPipe(pipeIdx+1, right); // 递归执行后续指令
		}
	}

	return result;
}
int callCommand(int commandNum) { // 给用户使用的函数，用以执行用户输入的命令
	pid_t pid = fork();
	if (pid == -1) {
		return ERROR_FORK;
	} else if (pid == 0) {
		/* 获取标准输入、输出的文件标识符 */
		int inFds = dup(STDIN_FILENO);
		int outFds = dup(STDOUT_FILENO);

		int result = callCommandWithPipe(0, commandNum);

		/* 还原标准输入、输出重定向 */
		dup2(inFds, STDIN_FILENO);
		dup2(outFds, STDOUT_FILENO);
		exit(result);
	} else {
		int status;
		waitpid(pid, &status, 0);
		return WEXITSTATUS(status);
	}
}


int callCd(int commandNum) { // 执行cd命令
	int result = RESULT_NORMAL;

	if (commandNum < 2) {
		result = ERROR_MISS_PARAMETER;
	} else if (commandNum > 2) {
		result = ERROR_TOO_MANY_PARAMETER;
	} else {
		int ret = chdir(commands[1]);
		if (ret) result = ERROR_WRONG_PARAMETER;
	}

	return result;
}

int main( ) {
	/* 获取当前工作目录、用户名、主机名 */
    getCurWorkDir();
	getUsername();
	getHostname();

	/* 启动myshell */
	char argv[BUF_SZ];
	while (1) {
		printf("\e[32;1m%s@%s:%s\e[0m$ ", username, hostname,curPath); // 显示为绿色
		/* 获取用户输入的命令 */
		fgets(argv, BUF_SZ, stdin);
		int len = strlen(argv);
		if (len != BUF_SZ) {
			argv[len-1] = '\0';
		}

		int commandNum = splitCommands(argv);
        //        callrm(commandNum,commands);
        char *argvv[100];
        for(int i=0;i<commandNum;i++)
        {
                argvv[i]=commands[i];
        }
		if (commandNum != 0) { // 用户有输入指令
			if (strcmp(commands[0], COMMAND_EXIT) == 0) { // exit命令
				int result = callExit();
				if (ERROR_EXIT == result) {
					exit(-1);
				}
			 }else if(strcmp(commands[0],COMMAND_MV)==0){

			        callmv(commandNum,argvv);
			 }
			else if(strcmp(commands[0],COMMAND_CP)==0){

			       	callcp(commandNum,argvv);


			}
			else if(strcmp(commands[0],COMMAND_LS)==0){

				callls(commandNum,argvv);
			}
			else if(strcmp(commands[0],COMMAND_CAT)==0){

				callcat(commandNum,argvv);
			}else if(strcmp(commands[0],COMMAND_WC)==0){

				callwc(commandNum,argvv);
			}
		        else if(strcmp(commands[0],COMMAND_RM)==0){

			     callrm(commandNum,argvv);
			}
			else if (strcmp(commands[0], COMMAND_CD) == 0) { // cd命令
				 callCd(commandNum);

				}
			 
			else { // 其它命令
				 callCommand(commandNum);

            }
        }
	}
    return 0;
}